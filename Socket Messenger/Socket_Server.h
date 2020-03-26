#ifndef SOCKET_SERVER
#define SOCKET_SERVER

#include <WinSock2.h>
#include <wx/thread.h>
#include <wx/cpp.h>
#include <string>
#include <array>
#include <atomic>
#include <string>
#include <mutex>
#include <vector>

class Server_Form;

enum
{
	ID_SOCKET_SERVER_POST_MESSAGE = 100,
	ID_SOCKET_SERVER_SET_INITIALIZED = 101,
	ID_SOCKET_SERVER_CLEAR_MESSAGE_BOX = 102,
	ID_SOCKET_SERVER_ADD_CLIENT = 103,
	ID_SOCKET_SERVER_REMOVE_CLIENT = 104
};

class Socket_Server : public wxThread
{
public:
	Socket_Server(Server_Form*);
	virtual void* Entry() wxOVERRIDE;
	std::string Get_IP();
	void Connect_Clients(SOCKET);
	void Send_Message_To_Clients(const std::string&);
	bool Recieved_Name(std::string);
	std::string Get_User_Name(std::string);
	std::string Get_Local_IP();

private:
	Server_Form* server_form;
	std::mutex Lock_Client_Sockets;
	std::array<SOCKET*, 25> client_sockets;
	std::mutex Lock_Client_Socket_Names;
	std::array<std::string, 25> client_socket_names;
	std::atomic<bool> connected_client;
	std::vector<std::string> all_client_names;
	std::mutex Lock_All_Client_Names;
};


#endif