#include "Server_Form.h"
#include "Socket_Server.h"
#include <wx/wx.h>
#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/button.h>
#include <wx/font.h>
#include <wx/textctrl.h>
#include <wx/msgdlg.h>
#include <wx/app.h>
#include <wx/stattext.h>
#include <wx/event.h>
#include <wx/listbox.h>
#include "Get_Name.h"
#include "Get_Message.h"
#include <vector>

extern wxSize Screen_Size;
std::mutex Lock_Admin_Message;



BEGIN_EVENT_TABLE(Server_Form, wxFrame)
	EVT_BUTTON(ID_INITIALIZE, Server_Form::Init_Button_Pressed)
	EVT_BUTTON(ID_DISCONNECT, Server_Form::Disconnect_Button_Pressed)
	EVT_BUTTON(ID_KICK_BUTTON, Server_Form::On_Kick_Button)
	EVT_COMMAND(ID_SOCKET_SERVER_POST_MESSAGE, wxEVT_COMMAND_TEXT_UPDATED, Server_Form::Socket_Server_Event_Post_Message)
	EVT_COMMAND(ID_SOCKET_SERVER_SET_INITIALIZED, wxEVT_COMMAND_TEXT_UPDATED, Server_Form::Socket_Server_Event_Set_Server_Initialized)
	EVT_COMMAND(ID_SOCKET_SERVER_ADD_CLIENT, wxEVT_COMMAND_TEXT_UPDATED, Server_Form::Socket_Server_Event_Add_Client)
	EVT_COMMAND(ID_SOCKET_SERVER_REMOVE_CLIENT, wxEVT_COMMAND_TEXT_UPDATED, Server_Form::Socket_Server_Event_Remove_Client)
END_EVENT_TABLE()

Server_Form::Server_Form(const std::string title, const int x, const int y, const int form_width, const int form_height) 
	:
	wxFrame(NULL, wxID_ANY, title, wxPoint(x, y), wxSize(form_width, form_height), wxDEFAULT_FRAME_STYLE & ~wxRESIZE_BORDER & ~wxMAXIMIZE_BOX),
	panel(NULL),
	button_initialize_server(NULL),
	button_disconnect_server(NULL),
	Message_Box(NULL),
	port_text_box(NULL),
	port_label(NULL),
	port_numbers_info_label(NULL),
	Input_Box(NULL),
	send_request(false),
	server_initialized(false),
	client_list_box(NULL),
	list_box_label(NULL),
	button_kick(NULL),
	kick_requested(false)
{
	/*width and height to size inside the frame border*/
	const int width = GetClientSize().GetWidth();
	const int height = GetClientSize().GetHeight();

	panel = new wxPanel(this, wxID_ANY, wxPoint(0,0), wxSize(Screen_Size.GetWidth(), Screen_Size.GetHeight()));
	panel->SetBackgroundColour(wxColor(200, 200, 200, 255));


	/*Create Messages TextBox to display text*/
	int text_box_width = width * 0.60;
	int text_box_height = height * 0.70;
	int text_box_posx = width * 0.05;
	int text_box_posy = 0;
	Message_Box = new wxRichTextCtrl(panel, wxID_ANY, "", wxPoint(text_box_posx, text_box_posy), wxSize(text_box_width, text_box_height), wxTE_READONLY | wxTE_MULTILINE);
	Message_Box->SetBackgroundColour(*wxWHITE);


	/*Create Inupt TextBox for user input*/
	int input_box_width = width * 0.60;
	int input_box_height = height * 0.20;
	int input_box_posx = Message_Box->GetPosition().x;
	int input_box_posy = height * 0.75;
	Input_Box = new wxRichTextCtrl(panel, wxID_ANY, "", wxPoint(input_box_posx, input_box_posy), wxSize(input_box_width, input_box_height), wxTE_MULTILINE);
	Message_Box->SetBackgroundColour(*wxWHITE);
	int font_size = (input_box_height / 2) / 4;
	wxFont font(font_size, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	Input_Box->SetFont(font);
	Input_Box->Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(Server_Form::OnKeyDown), NULL, this);
	Input_Box->SetMaxLength(1000);


	/*Create list box for displaying connected clients*/
	int list_box_width = width * 0.25;
	int list_box_height = height * 0.30;
	int list_box_posx = width * 0.70;
	int list_box_posy = height * 0.10;
	client_list_box = new wxListBox(panel, wxID_ANY, wxPoint(list_box_posx, list_box_posy), wxSize(list_box_width, list_box_height));
	font_size = (list_box_height / 2) / 6;
	font = wxFont(font_size, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
	client_list_box->SetFont(font);

	/*Create list box label*/
	int list_box_label_width = list_box_width * 0.90;
	int list_box_label_height = list_box_height * 0.15;
	int list_box_label_x = list_box_posx + list_box_width / 2 - list_box_label_width / 2;
	int list_box_label_y = list_box_posy - list_box_label_height * 1.05;
	list_box_label = new wxStaticText(panel, wxID_ANY, "Connected Clients", wxPoint(list_box_label_x, list_box_label_y), wxSize(list_box_label_width, list_box_label_height));
	font_size = list_box_label_height / 2;
	font = wxFont(font_size, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
	list_box_label->SetFont(font);


	/*Create Kick Button for list box*/
	int kick_button_width = list_box_width * 0.75;
	int kick_button_height = list_box_height * 0.20;
	int kick_button_posx = list_box_posx + list_box_width / 2 - kick_button_width / 2;
	int kick_button_posy = list_box_posy + list_box_height + kick_button_height / 3;
	button_kick = new wxButton(panel, ID_KICK_BUTTON, "Kick Client", wxPoint(kick_button_posx, kick_button_posy), wxSize(kick_button_width, kick_button_height));
	font_size = kick_button_height / 3.5;
	font = wxFont(font_size, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
	button_kick->SetFont(font);


	/*Create Initialize Button*/
	int initialize_button_width = width * 0.20;
	int initialize_button_height = initialize_button_width / 3;
	int initialize_button_posx = (width - initialize_button_width) * 0.90;
	int initialize_button_posy = (Input_Box->GetPosition().y + Input_Box->GetSize().GetHeight() - initialize_button_height);
	button_initialize_server = new wxButton(panel, ID_INITIALIZE, "Initialize Server", wxPoint(initialize_button_posx, initialize_button_posy), wxSize(initialize_button_width, initialize_button_height));
	font_size = initialize_button_height / 3.5;
	font = wxFont(font_size, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
	button_initialize_server->SetFont(font);

	/*Create Disconnect Button*/
	int disconnect_button_width = width * 0.20;
	int disconnect_button_height = disconnect_button_width / 3;
	int disconnect_button_posx = button_initialize_server->GetPosition().x;
	int disconnect_button_posy = button_initialize_server->GetPosition().y;
	button_disconnect_server = new wxButton(panel, ID_DISCONNECT, "Disconnect", wxPoint(disconnect_button_posx, disconnect_button_posy), wxSize(disconnect_button_width, disconnect_button_height));
	font_size = initialize_button_height / 3.5;
	font = wxFont(font_size, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
	button_disconnect_server->SetFont(font);
	button_disconnect_server->Disable();
	button_disconnect_server->Hide();


	/*Create Port Text Box*/
	int port_text_box_width = initialize_button_width*0.70;
	int port_text_box_height = initialize_button_height * 0.90;
	int port_text_box_posx = initialize_button_posx;
	int port_text_box_posy = (initialize_button_posy - port_text_box_height) * 0.975;
	port_text_box = new wxTextCtrl(panel, wxID_ANY, "", wxPoint(port_text_box_posx, port_text_box_posy), wxSize(port_text_box_width, port_text_box_height));
	port_text_box->SetMaxLength(5);
	font_size = port_text_box_height/2;
	font.SetFractionalPointSize(font_size);
	font.SetWeight(wxFONTWEIGHT_NORMAL);
	port_text_box->SetFont(font);

	
	/*Create Port Label*/
	int port_label_width = port_text_box->GetSize().GetWidth();
	int port_label_height = port_text_box->GetSize().GetHeight()/2;
	int port_label_posx = port_text_box->GetPosition().x;
	int port_label_posy = (port_text_box->GetPosition().y - port_label_height) * 0.975;
	port_label = new wxStaticText(panel, wxID_ANY, "Port Number:", wxPoint(port_label_posx, port_label_posy), wxSize(port_label_width, port_label_height));
	font_size = port_label_height;
	font.SetFractionalPointSize(font_size);
	port_label->SetFont(font);

	/*Create Port Numbers info Label*/
	int port_numbers_label_width = port_label->GetSize().GetWidth() * 0.50;
	int port_numbers_label_height = port_label->GetSize().GetHeight() / 2;
	int port_numbers_label_posx = port_label->GetPosition().x;
	int port_numbers_label_posy = port_label->GetPosition().y - port_numbers_label_height;
	port_numbers_info_label = new wxStaticText(panel, wxID_ANY, "(1024-65535)", wxPoint(port_numbers_label_posx, port_numbers_label_posy), wxSize(port_numbers_label_width, port_numbers_label_height));
	font_size = port_numbers_label_height * 0.80;
	font.SetFractionalPointSize(font_size);
	port_numbers_info_label->SetFont(font);

}

void Server_Form::Init_Button_Pressed(wxCommandEvent& event)
{
	Disable_Initialize_Button();
	Socket_Server* server = new Socket_Server(this);
	server->Create();
	server->Run();
}


void Server_Form::Disconnect_Button_Pressed(wxCommandEvent& event)
{
	Disable_Disconnect_Button();

	Set_Server_Initialized(false);
}


void Server_Form::Create_Pop_Up(const std::string& message, const std::string& title, long style)
{
	wxMessageBox(message, title, style);
}


int Server_Form::Get_Port()
{
	std::string number = port_text_box->GetValue().ToStdString();
	for (std::string::iterator it = number.begin(); it != number.end();)
	{
		if (std::isdigit(*it))
		{
			it++;
		}
		else if (std::isspace(*it))
		{
			it = number.erase(it);
		}
		else
		{
			return -1;
		}
	}

	int inumber = 0;
	if (!number.empty())
	{
		inumber = std::stoi(number);
	}

	if (inumber < 1024 || inumber > 65535)
	{
		return -1;
	}
	else
	{
		return inumber;
	}
}


void Server_Form::Set_Server_Initialized(bool flag)
{

	server_initialized = flag;

	if (server_initialized)
	{
		Show_Disconnect_Button();
		Enable_Disconnect_Button();
		message_box_lines.clear();
		port_text_box->Disable();
	}
	else
	{
		Show_Initialize_Button();
		Enable_Initialize_Button();
		port_text_box->Enable();
	}
}


void Server_Form::Enable_Initialize_Button()
{
	button_initialize_server->Enable();
}

void Server_Form::Enable_Disconnect_Button()
{
	button_disconnect_server->Enable();
}

void Server_Form::Disable_Initialize_Button()
{
	button_initialize_server->Disable();
}

void Server_Form::Disable_Disconnect_Button()
{
	button_disconnect_server->Disable();
}

void Server_Form::Show_Initialize_Button()
{
	button_initialize_server->Show();
	button_disconnect_server->Hide();
}

void Server_Form::Show_Disconnect_Button()
{
	button_disconnect_server->Show();
	button_initialize_server->Hide();
}


void Server_Form::Post_Message(const std::string& data)
{

	std::string recieved_data = data;
	std::string Name = Get_Name(recieved_data);
	std::string message = Get_Message(recieved_data);

	int font_size = (Message_Box->GetSize().GetHeight() / 2) / SERVER_MSG_BOX_LINES_VISIBLE;
	wxFont font(font_size, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
	Message_Box->BeginFont(font);
	Message_Box->DoWriteText(Name + ": ");
	Message_Box->EndFont();

	wxFont font2(font_size, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	Message_Box->BeginFont(font2);
	Message_Box->DoWriteText(message + "\n");
	Message_Box->EndFont();

	Message_Box->ScrollIntoView(Message_Box->GetLastPosition(), WXK_PAGEDOWN);

}

void Server_Form::Socket_Server_Event_Post_Message(wxCommandEvent& event)
{
	Post_Message(event.GetString().ToStdString());
}

void Server_Form::Socket_Server_Event_Set_Server_Initialized(wxCommandEvent& event)
{
	Set_Server_Initialized(event.GetInt());
}


void Server_Form::Clear_Message_Box()
{
	Message_Box->SetValue("");
}

void Server_Form::OnKeyDown(wxKeyEvent& event)
{
	if (event.GetKeyCode() == WXK_RETURN && server_initialized && Input_Box->GetValue() != "")
	{
		Lock_Admin_Message.lock();
		admin_message = Input_Box->GetValue();
		Lock_Admin_Message.unlock();
		Input_Box->SetValue("");
		send_request = true;

	}
	else if (event.GetKeyCode() == WXK_RETURN)
	{
		//Do Nothing
	}
	else
	{
		event.Skip();
	}

}


std::string Server_Form::Get_Input_Box_Value()
{

	if (send_request)
	{
		Lock_Admin_Message.lock();
		std::string message = admin_message;
		Lock_Admin_Message.unlock();
		send_request = false;
		return message;

	}
	else
	{
		return "";
	}
}

bool Server_Form::Server_Is_Initialized()
{
	return server_initialized;
}


void Server_Form::Socket_Server_Event_Add_Client(wxCommandEvent& event)
{
	client_list_box->AppendString(event.GetString());
}



void Server_Form::Socket_Server_Event_Remove_Client(wxCommandEvent& event)
{
	for (int i = 0; i < client_list_box->GetStrings().size(); i++)
	{
		if (client_list_box->GetString(i) == event.GetString())
		{
			client_list_box->Delete(i);
		}
	}
}



void Server_Form::On_Kick_Button(wxCommandEvent& event)
{
	std::lock_guard<std::mutex> Lock_Guard_Kick_Client_Name(Lock_Kick_Client_Name);
	if (server_initialized == true && kick_requested == false)
	{
		kick_client_name = client_list_box->GetStringSelection().ToStdString();
		kick_requested = true;
	}

}


bool Server_Form::Kick_Requested()
{
	return kick_requested;
}


std::string Server_Form::Get_Kick_Client_Name()
{
	std::lock_guard<std::mutex> Lock_Guard_Kick_Client_Name(Lock_Kick_Client_Name);
	kick_requested = false;
	return kick_client_name;
}


bool Server_Form::Admin_Send_Requested()
{
	return send_request;
}

