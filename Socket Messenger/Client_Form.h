#ifndef CLIENT_FORM
#define CLIENT_FORM

#include <wx/frame.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/stattext.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/notebook.h>
#include <atomic>
#include <string>
#include <mutex>
#include <deque>

enum
{
	ID_CLIENT_CONNECT,
	ID_CLIENT_DISCONNECT,
	ID_CLIENT_SEND,
	CLIENT_MSG_BOX_LINES_VISIBLE = 15
};

class Client_Form : public wxFrame
{
public:

	Client_Form(const std::string&, const int, const int, const int, const int);
	void Notebook_Selection(int);
	void Button_Pressed_Connect(wxCommandEvent&);
	void Button_Pressed_Disconnect(wxCommandEvent&);
	void Create_Pop_Up(const std::string&, const std::string&, long);
	void Set_Connection_Initialized(bool flag = true);
	bool Send_Request();
	void Post_Message(const std::string&);
	void Set_Send_Request(bool);
	std::string Get_Input_Box_Value();
	void Clear_Input_Box();
	std::string Get_Port();
	std::string Get_User_Name();
	std::string Get_IP();
	bool Name_Is_Valid();
	bool Port_Is_Valid();
	bool IP_Is_Valid();
	std::string Remove_Spaces(std::string);
	void Show_Connect_Button();
	void Show_Disconnect_Button();
	void Client_Connection_Event_Post_Message(wxCommandEvent&);
	void Client_Connection_Event_Set_Connection_Initialized(wxCommandEvent&);
	void Enable_Connect_Button();
	void Disable_Connect_Button();
	void Enable_Disconnect_Button();
	void Disable_Disconnect_Button();
	void Clear_Message_Box();
	void OnKeyDown(wxKeyEvent&);
	bool Connection_Is_Initialized();
	bool User_Disconnected();
	void Set_Disconnected(bool);

private:
	wxNotebook* notebook;
	wxButton* button_connect;
	wxButton* button_disconnect;
	wxTextCtrl* name_text_box;
	wxTextCtrl* IP_text_box;
	wxTextCtrl* port_text_box;
	wxStaticText* IP_Label;
	wxStaticText* Port_Label;
	wxStaticText* Name_Label;
	wxRichTextCtrl* Message_Box;
	wxRichTextCtrl* input_box;
	std::string user_name;
	std::string server_port;
	std::string server_ip_address;
	std::mutex Lock_Client_Message;
	std::string client_message;
	std::atomic<bool> send_request;
	std::deque<std::string> message_box_lines;
	std::atomic<bool> connection_initialized;
	std::atomic<bool> user_disconnected;
	std::deque<std::pair<std::string, std::string>> messages;

	DECLARE_EVENT_TABLE()
};



#endif