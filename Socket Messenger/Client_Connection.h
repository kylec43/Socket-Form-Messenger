#ifndef CLIENT_CONNECTION
#define CLIENT_CONNECTION

#include <WinSock2.h>
#include <wx/thread.h>
#include <atomic>
#include <string>
class Client_Form;


enum
{
	ID_CLIENT_CONNECTION_POST_MESSAGE = 200,
	ID_CLIENT_CONNECTION_SET_INITIALIZED = 201,
	ID_CLIENT_CONNECTION_CLEAR_MESSAGE_BOX = 202
};

class Client_Connection: public wxThread
{
public:
	Client_Connection(Client_Form*, std::string, std::string, std::string);
	void Recieve_Data(SOCKET);
	virtual void* Entry() wxOVERRIDE;

private:
	Client_Form* client_form;
	std::string client_name;
	std::string server_ip;
	std::string server_port;
};

#endif