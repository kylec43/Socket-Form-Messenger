#include "Socket_Server.h"
#include "Server_Form.h"
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <curl/curl.h>
#include <string>
#include <fstream>
#include <thread>
#include <regex>
#include <WinInet.h>
#include <array>
#include <wx\msgdlg.h>
#include <vector>
#include <algorithm>
#include <iphlpapi.h>
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
#define BUFFER_LENGTH 1001


Socket_Server::Socket_Server(Server_Form* form): 
	server_form(form),
	client_sockets({}),
	connected_client(false)
{

}

class test
{
public:
	std::string m;
	std::string t;
	int f;
};

void* Socket_Server::Entry()
{
	wxCommandEvent post_message_event(wxEVT_COMMAND_TEXT_UPDATED, ID_SOCKET_SERVER_POST_MESSAGE);
	wxCommandEvent set_initialized_event(wxEVT_COMMAND_TEXT_UPDATED, ID_SOCKET_SERVER_SET_INITIALIZED);

	/*Evaluate Port Number*/
	std::string port_no = std::to_string(server_form->Get_Port());
	if (port_no == "-1")
	{
		server_form->Create_Pop_Up("Invalid Port Number", "Error", wxOK | wxICON_ERROR);

		set_initialized_event.SetInt(false);
		server_form->GetEventHandler()->AddPendingEvent(set_initialized_event);

		return NULL;
	}


	SOCKET Listen_Socket = INVALID_SOCKET;
	WSADATA wsadata;
	int result = WSAStartup(MAKEWORD(2, 2), &wsadata);

	if (result != 0)
	{
		server_form->Create_Pop_Up("WSA Startup Failed", "Error", wxOK | wxICON_ERROR);

		set_initialized_event.SetInt(false);
		server_form->GetEventHandler()->AddPendingEvent(set_initialized_event);

		return NULL;
	}

	struct addrinfo* addr_result = NULL;
	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;


	result = getaddrinfo(NULL, port_no.c_str(), &hints, &addr_result);
	if (result != 0)
	{
		server_form->Create_Pop_Up("Error Getting Address Info:" + std::to_string(WSAGetLastError()), "Error", wxOK | wxICON_ERROR);
		WSACleanup();

		set_initialized_event.SetInt(false);
		server_form->GetEventHandler()->AddPendingEvent(set_initialized_event);

		return NULL;
	}


	Listen_Socket = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
	if (Listen_Socket == INVALID_SOCKET)
	{
		server_form->Create_Pop_Up("Error Creating Socket:" + std::to_string(WSAGetLastError()), "Error", wxOK | wxICON_ERROR);
		freeaddrinfo(addr_result);
		WSACleanup();

		set_initialized_event.SetInt(false);
		server_form->GetEventHandler()->AddPendingEvent(set_initialized_event);

		return NULL;
	}


	result = bind(Listen_Socket, addr_result->ai_addr, addr_result->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		server_form->Create_Pop_Up("Socket Binding Failed: " + std::to_string(WSAGetLastError()), "Error", wxOK | wxICON_ERROR);
		freeaddrinfo(addr_result);
		closesocket(Listen_Socket);
		WSACleanup();

		set_initialized_event.SetInt(false);
		server_form->GetEventHandler()->AddPendingEvent(set_initialized_event);

		return NULL;
	}
	/*address info no longer needed*/
	freeaddrinfo(addr_result);


	result = listen(Listen_Socket, SOMAXCONN);
	if (result == SOCKET_ERROR)
	{
		server_form->Create_Pop_Up("Socket Listening Error: " + std::to_string(WSAGetLastError()), "Error", wxOK | wxICON_ERROR);
		closesocket(Listen_Socket);
		WSACleanup();

		set_initialized_event.SetInt(false);
		server_form->GetEventHandler()->AddPendingEvent(set_initialized_event);

		return NULL;
	}
	set_initialized_event.SetInt(true);
	server_form->GetEventHandler()->AddPendingEvent(set_initialized_event);

	/*Get IP Addresses*/
	std::string server_ip = Get_IP();
	std::string local_ip = Get_Local_IP();

	post_message_event.SetString("COMMAND===NAME_IDENTIFIER===(SERVER)===MESSAGE===Server has been created!");
	server_form->GetEventHandler()->AddPendingEvent(post_message_event);

	post_message_event.SetString("COMMAND===NAME_IDENTIFIER===(SERVER)===MESSAGE===Public IPV4: " + ((server_ip != "NOT_FOUND") ? server_ip : std::string("Unavailable")));
	server_form->GetEventHandler()->AddPendingEvent(post_message_event);

	post_message_event.SetString("COMMAND===NAME_IDENTIFIER===(SERVER)===MESSAGE===LAN IPV4: " + ((local_ip != "NOT_FOUND") ? local_ip : std::string("Unavailable")));
	server_form->GetEventHandler()->AddPendingEvent(post_message_event);

	post_message_event.SetString("COMMAND===NAME_IDENTIFIER===(SERVER)===MESSAGE===Local Host: localhost");
	server_form->GetEventHandler()->AddPendingEvent(post_message_event);

	/*Accept Connections and Recieve messages from clients*/
	std::vector<std::thread> connect_clients;
	connect_clients.push_back(std::thread(&Socket_Server::Connect_Clients, this, Listen_Socket));

	std::string admin_message;
	std::string message;
	std::string user_name;
	while (server_form->Server_Is_Initialized())
	{

		/*Create new thread to accept another client*/
		if (connected_client == true)
		{
			connect_clients.push_back(std::thread(&Socket_Server::Connect_Clients, this, Listen_Socket));
			connected_client = false;
		}
		
		/*If there is a send request Send the Clients the message from the admin*/
		if (server_form->Admin_Send_Requested())
		{
			admin_message = server_form->Get_Input_Box_Value();
			if (admin_message != "")
			{
				message = "COMMAND===NAME_IDENTIFIER===(Admin)===MESSAGE===" + admin_message;
				post_message_event.SetString(message);
				server_form->GetEventHandler()->AddPendingEvent(post_message_event);
				Send_Message_To_Clients(message);
			}
		}

		/*If a kick is requested, kick the specified user*/
		if (server_form->Kick_Requested())
		{
			user_name = server_form->Get_Kick_Client_Name();

			Lock_Client_Sockets.lock();
			Lock_Client_Socket_Names.lock();
			for (int i = 0; i < client_socket_names.size(); i++)
			{
				if (user_name == client_socket_names.at(i))
				{
					shutdown(*client_sockets.at(i), SD_SEND);
					break;
				}
			}
			Lock_Client_Sockets.unlock();
			Lock_Client_Socket_Names.unlock();
		}

	}

	Lock_Client_Sockets.lock();
	for (auto client_socket : client_sockets)
	{
		if (client_socket != NULL)
		{
			shutdown(*client_socket, SD_SEND);
		}
	}
	Lock_Client_Sockets.unlock();

	closesocket(Listen_Socket);
	WSACleanup();


	/*Wait for all threads to finish*/
	std::for_each(connect_clients.begin(), connect_clients.end(), std::mem_fn(&std::thread::join));

	post_message_event.SetString("COMMAND===NAME_IDENTIFIER===(SERVER)===MESSAGE===Server has been disconnected!");
	server_form->GetEventHandler()->AddPendingEvent(post_message_event);

	return NULL;
}


std::string Socket_Server::Get_IP()
{
	/*Checks Internet Connection by checking if a connection can be made with google or facebook*/
	bool connected = InternetCheckConnection(L"http://www.google.com", FLAG_ICC_FORCE_CONNECTION, 0);
	if (!connected)
	{

		connected = InternetCheckConnection(L"http://www.facebook.com", FLAG_ICC_FORCE_CONNECTION, 0);
		if (!connected)
		{
			return "NOT_FOUND";
		}
	}
	

	CURL* curl;
	FILE* fp;
	CURLcode res;
	const char* url = "http://checkip.dyndns.org/";
	char outfilename[FILENAME_MAX] = "ip_addr";
	curl = curl_easy_init();
	if (curl)
	{
		fp = fopen(outfilename, "wb");
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		fclose(fp);
	}
	

	std::ifstream r_file("ip_addr");


	bool found_ip = false;
	std::string server_ip;
	if (r_file.is_open())
	{
		std::string ip_pattern("[[:digit:]]+\.[[:digit:]]+\.[[:digit:]]+\.[[:digit:]]+");
		std::string line;
		while (std::getline(r_file, line))
		{
			std::regex r(ip_pattern);
			std::smatch ip_addr;
			if (std::regex_search(line, ip_addr, r))
			{
				server_ip = ip_addr.str();
				found_ip = true;
				break;
			}
		}

		r_file.close();
		remove("ip_addr");
	}
	
	return found_ip ? server_ip : "NOT_FOUND";
}




void Socket_Server::Connect_Clients(SOCKET Listen_Socket)
{
	wxCommandEvent post_message_event(wxEVT_COMMAND_TEXT_UPDATED, ID_SOCKET_SERVER_POST_MESSAGE);
	wxCommandEvent add_client_event(wxEVT_COMMAND_TEXT_UPDATED, ID_SOCKET_SERVER_ADD_CLIENT);
	wxCommandEvent remove_client_event(wxEVT_COMMAND_TEXT_UPDATED, ID_SOCKET_SERVER_REMOVE_CLIENT);


	/*Accept Client Connection*/
	SOCKET Client_Socket;
	std::string user_name;
	std::string message;
	bool name_set = false;

	Client_Socket = accept(Listen_Socket, NULL, NULL);
	if (Client_Socket == SOCKET_ERROR)
	{
		return;
	}
	else
	{
		connected_client = true;

		/*Find Open Spot for client. If full, kick them out*/
		int client_pos = -1;

		Lock_Client_Sockets.lock();
		for (int i = 0; i < client_sockets.size(); i++)
		{
			if (client_sockets.at(i) == NULL)
			{
				client_pos = i;
				break;
			}
		}
		Lock_Client_Sockets.unlock();

		if (client_pos == -1)
		{
			shutdown(Client_Socket, SD_SEND);
			closesocket(Client_Socket);

			Lock_Client_Sockets.lock();
			client_sockets.at(client_pos) = NULL;
			Lock_Client_Sockets.unlock();
			return;
		}

		Lock_Client_Sockets.lock();
		client_sockets.at(client_pos) = &Client_Socket;
		Lock_Client_Sockets.unlock();

		int bytes_recieved = 0;
		char recv_buffer[BUFFER_LENGTH];
		std::string command_before_name = "COMMAND===NAME_IDENTIFIER===(";
		std::string command_after_name_before_message = ")===MESSAGE===";
		do
		{
			bytes_recieved = recv(Client_Socket, recv_buffer, BUFFER_LENGTH, 0);
			if (bytes_recieved > 0)
			{

				recv_buffer[(bytes_recieved == BUFFER_LENGTH) ? BUFFER_LENGTH-1 : bytes_recieved] = '\0';
				std::string data_string = recv_buffer;
				if (name_set == true)
				{
					message = command_before_name + user_name + command_after_name_before_message + data_string;
					post_message_event.SetString(message);
					server_form->GetEventHandler()->AddPendingEvent(post_message_event);

					/*If a valid message is recieved from the client, send the message to all clients*/
					Send_Message_To_Clients(message);

				}
				else if (name_set == false)
				{
					/*Get user name and set user name*/
					user_name = Get_User_Name(data_string);
					name_set = true;

					Lock_Client_Socket_Names.lock();
					client_socket_names.at(client_pos) = user_name;
					Lock_Client_Socket_Names.unlock();

					add_client_event.SetString(user_name);
					server_form->GetEventHandler()->AddPendingEvent(add_client_event);

					message = command_before_name + "SERVER" + command_after_name_before_message + user_name + " has Connected!";
					post_message_event.SetString(message);
					server_form->GetEventHandler()->AddPendingEvent(post_message_event);
					Send_Message_To_Clients(message);
				}
			}

		} while (bytes_recieved > 0);

		if (server_form->Server_Is_Initialized())
		{
			message = command_before_name + "SERVER" + command_after_name_before_message + user_name + " has Disconnected!";
			post_message_event.SetString(message);
			server_form->GetEventHandler()->AddPendingEvent(post_message_event);
			Send_Message_To_Clients(message);
		}

		shutdown(Client_Socket, SD_SEND);
		closesocket(Client_Socket);

		Lock_Client_Sockets.lock();
		client_sockets.at(client_pos) = NULL;
		Lock_Client_Sockets.unlock();
	}

	/*remove name from all_client_names*/
	Lock_All_Client_Names.lock();
	for (int i = 0; i < all_client_names.size(); i++)
	{
		if (all_client_names.at(i) == user_name)
		{
			all_client_names.erase(all_client_names.begin() + i);
		}
	}
	Lock_All_Client_Names.unlock();

	/*remove name from list_box on server form*/
	remove_client_event.SetString(user_name);
	server_form->GetEventHandler()->AddPendingEvent(remove_client_event);
}




void Socket_Server::Send_Message_To_Clients(const std::string& message)
{
	/*If a valid message is recieved from the client, send the message to all clients*/
	Lock_Client_Sockets.lock();
	for (auto client_socket : client_sockets)
	{
		if (client_socket != NULL)
		{
			int send_result = send(*client_socket, message.c_str(), strlen(message.c_str()), 0);
			if (send_result == INVALID_SOCKET)
			{
				closesocket(*client_socket);
			}
		}
	}
	Lock_Client_Sockets.unlock();
}


bool Socket_Server::Recieved_Name(std::string data_string)
{
	std::string pattern = "COMMAND===NAME_IDENTIFIER===\\(([[:print:]]+)\\)";
	std::regex r(pattern);
	std::smatch identifier;
	if (std::regex_search(data_string, identifier, r))
	{
		if (identifier.str().size() == data_string.size())
		{
			return true;
		}
	}
	else
	{
		return false;
	}
}


std::string Socket_Server::Get_User_Name(std::string data_string)
{
	std::lock_guard<std::mutex> Lock_Guard_Client_Names(Lock_All_Client_Names);

	std::string pattern = "COMMAND===NAME_IDENTIFIER===\\(([[:print:]]+)\\)";
	std::regex r(pattern);
	std::string fmt("$1");
	std::smatch name;
	if (std::regex_search(data_string, name, r))
	{

		/*Search for duplicate names and update name if duplicate found*/
		std::string founded_name = std::regex_replace(name.str(), r, fmt);
		int name_duplicates_found = 0;
		for (int i = 0; i < all_client_names.size();)
		{
			if (founded_name == all_client_names.at(i))
			{
				founded_name = std::regex_replace(name.str(), r, fmt) + "(" + std::to_string(++name_duplicates_found) + ")";
				i = 0;
			}
			else
			{
				i++;
			}
		}
		all_client_names.push_back(founded_name);

		return founded_name;
	}
	else
	{

		/*Search for duplicates and update name as necessary*/
		std::string error_name = "ERROR_NAME";
		std::string founded_error_name = error_name;
		int name_duplicates_found = 0;
		for (int i = 0; i < all_client_names.size();)
		{
			if ( founded_error_name == all_client_names.at(i))
			{
				founded_error_name = error_name + "(" + std::to_string(++name_duplicates_found) + ")";
				i = 0;
			}
			else
			{
				i++;
			}
		}
		all_client_names.push_back(founded_error_name);


		return founded_error_name;
	}
}


std::string Socket_Server::Get_Local_IP()
{
	int i = 0;

	PMIB_IPADDRTABLE pIPAddrTable;
	DWORD dwSize = 0;
	DWORD dwRetVal = 0;
	IN_ADDR IPAddr;

	LPVOID lpMsgBuf;

	pIPAddrTable = (MIB_IPADDRTABLE*)MALLOC(sizeof(MIB_IPADDRTABLE));

	if (pIPAddrTable) 
	{

		if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) ==
			ERROR_INSUFFICIENT_BUFFER) 
		{
			FREE(pIPAddrTable);
			pIPAddrTable = (MIB_IPADDRTABLE*)MALLOC(dwSize);
		}
		if (pIPAddrTable == NULL) 
		{
			return "NOT_FOUND";
		}

	}

	if ((dwRetVal = GetIpAddrTable(pIPAddrTable, &dwSize, 0)) != NO_ERROR) 
	{
		if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),       // Default language
			(LPTSTR)&lpMsgBuf, 0, NULL)) 
		{
			LocalFree(lpMsgBuf);
		}
		return "NOT_FOUND";
	}

	for (i = 0; i < pIPAddrTable->dwNumEntries; i++) 
	{
		IPAddr.S_un.S_addr = (u_long)pIPAddrTable->table[i].dwAddr;
		std::string ip = inet_ntoa(IPAddr);
		if (ip.find("127.0.0.1") == std::string::npos)
		{
			return inet_ntoa(IPAddr);
		}
	}

	if (pIPAddrTable) 
	{
		FREE(pIPAddrTable);
		pIPAddrTable = NULL;
	}

	return "NOT_FOUND";
}
