#ifndef SERVER_FORM
#define SERVER_FORM

#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/richtext/richtextctrl.h>
#include <atomic>
#include <mutex>
#include <deque>
#include <wx/listbox.h>
#include <vector>
class wxButton;
class wxTextCtrl;
class wxStaticText;

enum
{
	ID_INITIALIZE,
	ID_DISCONNECT,
	ID_KICK_BUTTON
};


class Server_Form : public wxFrame
{
public:
	Server_Form(const std::string, const int, const int, const int, const int);
	void Init_Button_Pressed(wxCommandEvent&);
	void Disconnect_Button_Pressed(wxCommandEvent&);
	void Create_Pop_Up(const std::string&, const std::string&, long style = 5L);
	int Get_Port();
	void Set_Server_Initialized(bool);
	void Show_Initialize_Button();
	void Show_Disconnect_Button();
	void Post_Message(const std::string&);
	void Disable_Disconnect_Button();
	void Enable_Disconnect_Button();
	void Disable_Initialize_Button();
	void Enable_Initialize_Button();
	void Socket_Server_Event_Post_Message(wxCommandEvent&);
	void Socket_Server_Event_Set_Server_Initialized(wxCommandEvent&);
	void Clear_Message_Box();
	bool Admin_Send_Requested();
	void OnKeyDown(wxKeyEvent&);
	std::string Get_Input_Box_Value();
	bool Server_Is_Initialized();
	void Socket_Server_Event_Add_Client(wxCommandEvent&);
	void Socket_Server_Event_Remove_Client(wxCommandEvent&);
	void On_Kick_Button(wxCommandEvent&);
	bool Kick_Requested();
	std::string Get_Kick_Client_Name();


private:
	wxPanel* panel;
	wxButton* button_initialize_server;
	wxButton* button_disconnect_server;
	wxRichTextCtrl* Message_Box;
	wxTextCtrl* port_text_box;
	wxStaticText* port_label;
	wxStaticText* port_numbers_info_label;
	wxRichTextCtrl* Input_Box;
	std::atomic<bool> send_request;
	std::string admin_message;
	std::deque<std::string> message_box_lines;
	std::atomic<bool> server_initialized;
	wxListBox* client_list_box;
	wxStaticText* list_box_label;
	wxButton* button_kick;
	std::atomic<bool> kick_requested;
	std::string kick_client_name;
	std::mutex Lock_Kick_Client_Name;


	DECLARE_EVENT_TABLE()
};


#endif