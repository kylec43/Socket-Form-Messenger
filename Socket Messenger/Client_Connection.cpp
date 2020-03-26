#include "Client_Connection.h"
#include "Client_Form.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <thread>
#include <atomic>
#define BUFFER_LENGTH 1001

Client_Connection::Client_Connection(Client_Form* form, std::string name, std::string ip, std::string port)
	: 
	client_form(form), 
	client_name(name), 
	server_ip(ip), 
	server_port(port)
{

}

void* Client_Connection::Entry()
{

	wxCommandEvent set_initialized_event(wxEVT_COMMAND_TEXT_UPDATED, ID_CLIENT_CONNECTION_SET_INITIALIZED);


	WSADATA wsadata;
	int result = WSAStartup(MAKEWORD(2, 2), &wsadata);

	if (result != 0)
	{
		client_form->Create_Pop_Up("Error Initializing WSA: " + std::to_string(WSAGetLastError()), "Error", wxOK | wxICON_ERROR);

		set_initialized_event.SetInt(false);
		client_form->GetEventHandler()->AddPendingEvent(set_initialized_event);

		return NULL;
	}

	struct addrinfo* addr_result = NULL;
	struct addrinfo hints;
	struct addrinfo* ptr = NULL;


	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;


	result = getaddrinfo(server_ip.c_str(), server_port.c_str(), &hints, &addr_result);
	if (result != 0)
	{
		client_form->Create_Pop_Up("Getting Address Info Failed: " + std::to_string(WSAGetLastError()), "Error", wxOK | wxICON_ERROR);
		WSACleanup();

		set_initialized_event.SetInt(false);
		client_form->GetEventHandler()->AddPendingEvent(set_initialized_event);

		return NULL;
	}


	ptr = addr_result;


	SOCKET Connect_Socket = INVALID_SOCKET;
	Connect_Socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
	if (Connect_Socket == INVALID_SOCKET)
	{
		client_form->Create_Pop_Up("Socket Creation Error: " + std::to_string(WSAGetLastError()), "Error", wxOK | wxICON_ERROR);
		freeaddrinfo(addr_result);
		WSACleanup();

		set_initialized_event.SetInt(false);
		client_form->GetEventHandler()->AddPendingEvent(set_initialized_event);

		return NULL;
	}

	result = connect(Connect_Socket, ptr->ai_addr, ptr->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		closesocket(Connect_Socket);
		Connect_Socket = INVALID_SOCKET;
	}

	freeaddrinfo(addr_result);

	if (Connect_Socket == INVALID_SOCKET)
	{
		client_form->Create_Pop_Up("Unable to Connect to Server: ", "Error", wxOK | wxICON_ERROR);
		WSACleanup();

		set_initialized_event.SetInt(false);
		client_form->GetEventHandler()->AddPendingEvent(set_initialized_event);

		return NULL;
	}
	else
	{
		set_initialized_event.SetInt(true);
		client_form->GetEventHandler()->AddPendingEvent(set_initialized_event);

		client_form->Notebook_Selection(0);

		std::string name_identifier = "COMMAND===NAME_IDENTIFIER===(" + client_name + ")";
		result = send(Connect_Socket, name_identifier.c_str(), strlen(name_identifier.c_str()), 0);
	}


	/*Recieve Data From Server*/
	std::thread recieve_data(&Client_Connection::Recieve_Data, this, Connect_Socket);

	/*Send Data to Server*/
	std::string message;
	while (client_form->Connection_Is_Initialized())
	{
		message = client_form->Get_Input_Box_Value();
		if (message != "")
		{
			send(Connect_Socket, message.c_str(), strlen(message.c_str()), 0);
		}


	}


	result = shutdown(Connect_Socket, SD_SEND);
	if (result == SOCKET_ERROR) 
	{
		client_form->Create_Pop_Up("Shutdown Error: " + std::to_string(WSAGetLastError()), "Error", wxOK | wxICON_ERROR);
		closesocket(Connect_Socket);
		WSACleanup();

		set_initialized_event.SetInt(false);
		client_form->GetEventHandler()->AddPendingEvent(set_initialized_event);

		return NULL;
	}

	closesocket(Connect_Socket);
	WSACleanup();
	recieve_data.join();

	set_initialized_event.SetInt(false);
	client_form->GetEventHandler()->AddPendingEvent(set_initialized_event);
	return NULL;
}


void Client_Connection::Recieve_Data(SOCKET Connect_Socket)
{
	wxCommandEvent post_message_event(wxEVT_COMMAND_TEXT_UPDATED, ID_CLIENT_CONNECTION_POST_MESSAGE);
	wxCommandEvent set_initialized_event(wxEVT_COMMAND_TEXT_UPDATED, ID_CLIENT_CONNECTION_SET_INITIALIZED);

	int recv_buffer_length = BUFFER_LENGTH;
	char recv_buffer[BUFFER_LENGTH];
	int bytes_recieved = 0;
	while (client_form->Connection_Is_Initialized())
	{

		bytes_recieved = recv(Connect_Socket, recv_buffer, recv_buffer_length, 0);
		if (bytes_recieved > 0)
		{
			recv_buffer[(bytes_recieved == BUFFER_LENGTH) ? BUFFER_LENGTH - 1 : bytes_recieved] = '\0';
			post_message_event.SetString(recv_buffer);
			client_form->GetEventHandler()->AddPendingEvent(post_message_event);
		}
		else
		{

			set_initialized_event.SetInt(false);
			client_form->GetEventHandler()->AddPendingEvent(set_initialized_event);

			if (client_form->User_Disconnected())
			{
				client_form->Create_Pop_Up("You have sucessfully disconnected from the server!", "info", wxOK | wxICON_INFORMATION);
				client_form->Set_Disconnected(false);
			}
			else
			{
				client_form->Create_Pop_Up("Connection Lost.", "info", wxOK | wxICON_INFORMATION);
			}
			break;
		}
	}

}