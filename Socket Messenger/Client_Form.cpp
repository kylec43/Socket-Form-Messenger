#include "Client_Form.h"
#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/notebook.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/msgdlg.h>
#include <atomic>
#include <regex>
#include <algorithm>
#include "Client_Connection.h"
#include "Get_Name.h"
#include "Get_Message.h"
#include <mutex>
#include <limits.h>




BEGIN_EVENT_TABLE(Client_Form, wxFrame)
	EVT_BUTTON(ID_CLIENT_CONNECT, Client_Form::Button_Pressed_Connect)
	EVT_BUTTON(ID_CLIENT_DISCONNECT, Client_Form::Button_Pressed_Disconnect)
	EVT_COMMAND(ID_CLIENT_CONNECTION_POST_MESSAGE, wxEVT_COMMAND_TEXT_UPDATED, Client_Form::Client_Connection_Event_Post_Message)
	EVT_COMMAND(ID_CLIENT_CONNECTION_SET_INITIALIZED, wxEVT_COMMAND_TEXT_UPDATED, Client_Form::Client_Connection_Event_Set_Connection_Initialized)
END_EVENT_TABLE()


Client_Form::Client_Form(const std::string& title, const int posx, const int posy, const int frame_width, const int frame_height)
	:
	wxFrame(NULL, wxID_ANY, title, wxPoint(posx, posy), wxSize(frame_width, frame_height), wxDEFAULT_FRAME_STYLE & ~wxRESIZE_BORDER & ~wxMAXIMIZE_BOX),
	notebook(NULL),
	button_connect(NULL),
	button_disconnect(NULL),
	name_text_box(NULL),
	IP_text_box(NULL),
	port_text_box(NULL),
	IP_Label(NULL),
	Port_Label(NULL),
	Name_Label(NULL),
	Message_Box(NULL),
	input_box(NULL),
	send_request(false),
	connection_initialized(false)

{
	/*Get Client area width and height*/
	const int width = GetClientSize().GetWidth();
	const int height = GetClientSize().GetHeight();

	notebook = new wxNotebook(this, wxID_ANY);

	/*Create Panels for notebook*/
	wxPanel* panel_messenger = new wxPanel(notebook, wxID_ANY);
	panel_messenger->SetBackgroundColour(wxColor(200, 200, 200, 255));

	wxPanel* panel_connect = new wxPanel(notebook, wxID_ANY);
	panel_connect->SetBackgroundColour(wxColor(200, 200, 200, 255));



	/*Components for panel_messenger*/

	/*Create text box for messages*/
	int text_box_width = width * 0.70;
	int text_box_height = height * 0.70;
	int text_box_posx = width * 0.50 - text_box_width / 2;
	int text_box_posy = 0;
	Message_Box = new wxRichTextCtrl(panel_messenger, wxID_ANY, "", wxPoint(text_box_posx, text_box_posy), wxSize(text_box_width, text_box_height), wxBORDER_SIMPLE | wxRE_READONLY | wxRE_MULTILINE);
	Message_Box->SetBackgroundColour(*wxWHITE);


	/*Create Input box for sending messages*/
	int input_box_width = Message_Box->GetSize().GetWidth();
	int input_box_height = height * 0.20;
	int input_box_posx = text_box_posx;
	int input_box_posy = height * 0.725;
	input_box = new wxRichTextCtrl(panel_messenger, wxID_ANY, "", wxPoint(input_box_posx, input_box_posy), wxSize(input_box_width, input_box_height), wxTE_MULTILINE);
	input_box->SetBackgroundColour(*wxWHITE);
	input_box->SetMaxLength(1000);
	int font_size = (input_box_height / 2) / 4;
	wxFont font(font_size, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	input_box->SetFont(font);
	input_box->Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(Client_Form::OnKeyDown), NULL, this);


	/*Components for panel_connect*/

	/*Create connect Button*/
	int connect_button_width = width * 0.20;
	int connect_button_height = connect_button_width / 3;
	int connect_button_posx = width / 2 - connect_button_width / 2;
	int connect_button_posy = (height - connect_button_height) * 0.60;
	button_connect = new wxButton(panel_connect, ID_CLIENT_CONNECT, "Connect", wxPoint(connect_button_posx, connect_button_posy), wxSize(connect_button_width, connect_button_height));
	font_size = connect_button_height / 3.5;
	font = wxFont(font_size, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
	font.SetFractionalPointSize(font_size);
	button_connect->SetFont(font);

	/*Create Disconnect Button*/
	int disconnect_button_width = width * 0.20;
	int disconnect_button_height = disconnect_button_width / 3;
	int disconnect_button_posx = button_connect->GetPosition().x;
	int disconnect_button_posy = button_connect->GetPosition().y;
	button_disconnect = new wxButton(panel_connect, ID_CLIENT_DISCONNECT, "Disconnect", wxPoint(disconnect_button_posx, disconnect_button_posy), wxSize(disconnect_button_width, disconnect_button_height));
	font_size = connect_button_height / 3.5;
	font = wxFont(font_size, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
	button_disconnect->SetFont(font);
	button_disconnect->Disable();
	button_disconnect->Hide();


	/*Create IPv4_Text_Box*/
	int IP_textbox_width = button_connect->GetSize().GetWidth() * 1.30;
	int IP_textbox_height = IP_textbox_width / 5;
	int IP_textbox_posx = button_connect->GetPosition().x;
	int IP_textbox_posy = (button_connect->GetPosition().y - IP_textbox_height) * 0.975;
	IP_text_box = new wxTextCtrl(panel_connect, wxID_ANY, "", wxPoint(IP_textbox_posx, IP_textbox_posy), wxSize(IP_textbox_width, IP_textbox_height));
	IP_text_box->SetMaxLength(15);
	font_size = IP_textbox_height / 2;
	font.SetFractionalPointSize(font_size);
	font.SetWeight(wxFONTWEIGHT_NORMAL);
	IP_text_box->SetFont(font);


	/*Create Port_Text_Box*/
	int port_textbox_width = button_connect->GetSize().GetWidth() * 1.30;
	int port_textbox_height = port_textbox_width / 5;
	int port_textbox_posx = button_connect->GetPosition().x;
	int port_textbox_posy = (IP_text_box->GetPosition().y - port_textbox_height) * 0.975;
	port_text_box = new wxTextCtrl(panel_connect, wxID_ANY, "", wxPoint(port_textbox_posx, port_textbox_posy), wxSize(port_textbox_width, port_textbox_height));
	port_text_box->SetMaxLength(5);
	font_size = port_textbox_height / 2;
	font.SetFractionalPointSize(font_size);
	font.SetWeight(wxFONTWEIGHT_NORMAL);
	port_text_box->SetFont(font);


	/*Create Name_Text_Box*/
	int name_textbox_width = button_connect->GetSize().GetWidth() * 1.30;
	int name_textbox_height = name_textbox_width / 5;
	int name_textbox_posx = button_connect->GetPosition().x;;
	int name_textbox_posy = (port_text_box->GetPosition().y - name_textbox_height) * 0.975;
	name_text_box = new wxTextCtrl(panel_connect, wxID_ANY, "", wxPoint(name_textbox_posx, name_textbox_posy), wxSize(name_textbox_width, name_textbox_height));
	name_text_box->SetMaxLength(15);
	font_size = name_textbox_height / 2;
	font.SetWeight(wxFONTWEIGHT_BOLD);
	font.SetFractionalPointSize(font_size);
	name_text_box->SetFont(font);


	/*Create IPv4 Label*/
	int IP_Label_width = IP_text_box->GetSize().GetWidth() / 2;
	int IP_Label_height = IP_Label_width * 0.40;
	int IP_Label_posx = IP_text_box->GetPosition().x - IP_Label_width * 1.5;
	int IP_Label_posy = IP_text_box->GetPosition().y + IP_Label_height * 0.25;
	IP_Label = new wxStaticText(panel_connect, wxID_ANY, "I Pv4 Address:", wxPoint(IP_Label_posx, IP_Label_posy), wxSize(IP_Label_width, IP_Label_height));
	font_size = IP_Label_height * 0.40;
	font.SetFractionalPointSize(font_size);
	font.SetWeight(wxFONTWEIGHT_BOLD);
	IP_Label->SetFont(font);


	/*Create Port Label*/
	int Port_Label_width = IP_Label->GetSize().GetWidth();
	int Port_Label_height = IP_Label->GetSize().GetHeight();
	int Port_Label_posx = IP_Label->GetPosition().x;
	int Port_Label_posy = port_text_box->GetPosition().y + IP_Label_height * 0.25;
	Port_Label = new wxStaticText(panel_connect, wxID_ANY, "Port:", wxPoint(Port_Label_posx, Port_Label_posy), wxSize(Port_Label_width, Port_Label_height));
	font_size = Port_Label_height * 0.40;
	font.SetFractionalPointSize(font_size);
	font.SetWeight(wxFONTWEIGHT_BOLD);
	Port_Label->SetFont(font);


	/*Create Name Label*/
	int Name_Label_width = IP_Label->GetSize().GetWidth();
	int Name_Label_height = IP_Label->GetSize().GetHeight();
	int Name_Label_posx = IP_Label->GetPosition().x;
	int Name_Label_posy = name_text_box->GetPosition().y + Name_Label_height * 0.25;
	Name_Label = new wxStaticText(panel_connect, wxID_ANY, "User Name:", wxPoint(Name_Label_posx, Name_Label_posy), wxSize(Name_Label_width, Name_Label_height));
	font_size = Name_Label_height * 0.40;
	font.SetFractionalPointSize(font_size);
	font.SetWeight(wxFONTWEIGHT_BOLD);
	Name_Label->SetFont(font);

	/*Create Pages with panels for notebook*/
	notebook->AddPage(panel_messenger, "Messenger", true, 0);
	notebook->AddPage(panel_connect, "Connect", false, 1);
}


void Client_Form::Notebook_Selection(int page)
{
	notebook->SetSelection(0);
}

void Client_Form::Button_Pressed_Connect(wxCommandEvent& e)
{
	user_name = name_text_box->GetValue().ToStdString();
	server_port = Remove_Spaces(port_text_box->GetValue().ToStdString());
	server_ip_address = Remove_Spaces(IP_text_box->GetValue().ToStdString());

	if (!Name_Is_Valid())
	{
		Create_Pop_Up("Please enter a valid name to continue.", "Error", wxOK | wxICON_ERROR);
		return;
	}
	
	if (!Port_Is_Valid())
	{
		Create_Pop_Up("Please enter a valid port for the server (1024-65535).", "Error", wxOK | wxICON_ERROR);
		return;
	}

	if (!IP_Is_Valid())
	{
		Create_Pop_Up("Please enter a valid IP Address for the server.", "Error", wxOK | wxICON_ERROR);
		return;
	}

	Disable_Connect_Button();
	Message_Box->Clear();
	Client_Connection* client_connection = new Client_Connection(this, Get_User_Name(), Get_IP(), Get_Port());
	client_connection->Create();
	client_connection->Run();
}


void Client_Form::Button_Pressed_Disconnect(wxCommandEvent& e)
{
	Disable_Disconnect_Button();
	user_disconnected = true;
	Set_Connection_Initialized(false);
}

bool Client_Form::User_Disconnected()
{
	return user_disconnected;
}


void Client_Form::Create_Pop_Up(const std::string& message, const std::string& title, long style)
{
	wxMessageBox(message, title, style);
}


void Client_Form::Set_Connection_Initialized(bool flag)
{
	connection_initialized = flag;

	if (connection_initialized == false)
	{
		name_text_box->Enable();
		port_text_box->Enable();
		IP_text_box->Enable();
		Show_Connect_Button();
		Enable_Connect_Button();
		message_box_lines.clear();
	}
	else
	{
		name_text_box->Disable();
		port_text_box->Disable();
		IP_text_box->Disable();
		Show_Disconnect_Button();
		Enable_Disconnect_Button();
	}
}


bool Client_Form::Send_Request()
{
	return send_request;
}



void Client_Form::Post_Message(const std::string& data)
{

	std::string recieved_data = data;
	std::string Name = Get_Name(recieved_data);
	std::string message = Get_Message(recieved_data);

	int font_size = (Message_Box->GetSize().GetHeight() / 2) / CLIENT_MSG_BOX_LINES_VISIBLE;
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


void Client_Form::Set_Send_Request(bool flag)
{
	send_request = flag;
}

std::string Client_Form::Get_Input_Box_Value()
{
	if (send_request)
	{
		Lock_Client_Message.lock();
		std::string message = client_message;
		Lock_Client_Message.unlock();
		send_request = false;
		return message;

	}
	else
	{
		return "";
	}
}

void Client_Form::Clear_Input_Box()
{
	input_box->SetValue("");
}


std::string Client_Form::Get_Port()
{
	return server_port;
}

std::string Client_Form::Get_User_Name()
{
	return user_name;
}

std::string Client_Form::Get_IP()
{
	return server_ip_address;
}

bool Client_Form::Name_Is_Valid()
{
	if (Get_User_Name().empty())
	{
		return false;
	}

	bool found_valid_character = false;
	for (int i = 0; i < GetName().length(); i++)
	{
		if (std::isalnum(Get_User_Name()[i]))
		{
			return true;
		}
	}

	return false;
}



bool Client_Form::Port_Is_Valid()
{
	if (Get_Port().length() == 0)
	{
		return false;
	}

	for (int i = 0; i < Get_Port().length(); i++)
	{
		if (!isdigit(Get_Port()[i]))
		{
			return false;
		}
	}

	if (std::stoi(Get_Port()) >= 1024 && std::stoi(Get_Port()) <= 65535)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool Client_Form::IP_Is_Valid()
{
	if (Get_IP() == "localhost")
	{
		return true;
	}
	else
	{

		if (Get_IP().length() == 0)
		{
			return false;
		}

		for (int i = 0; i < Get_IP().length(); i++)
		{
			if (!isdigit(Get_IP()[i]) && Get_IP()[i] != '.')
			{
				return false;
			}
		}

		std::string text = Get_IP();
		std::string pattern = "[[:digit:]]+\.[[:digit:]]+\.[[:digit:]]+\.[[:digit:]]+";
		std::regex r(pattern);
		std::smatch ip_addr;
		if(std::regex_search(text, ip_addr, r))
		{
			return true;
		}
		else
		{
			return false;
		}

	}
}

std::string Client_Form::Remove_Spaces(std::string s)
{
	std::remove_if(s.begin(), s.end(), [](const char c) {return c == ' '; });
	return s;
}


void Client_Form::Show_Connect_Button()
{
	button_connect->Show();
	button_connect->Enable();

	button_disconnect->Hide();
	button_disconnect->Disable();
}

void Client_Form::Show_Disconnect_Button()
{
	button_disconnect->Show();
	button_disconnect->Enable();

	button_connect->Hide();
	button_connect->Disable();

}


void Client_Form::Enable_Connect_Button()
{
	button_connect->Enable();
}

void Client_Form::Enable_Disconnect_Button()
{
	button_disconnect->Enable();
}

void Client_Form::Disable_Connect_Button()
{
	button_connect->Disable();
}

void Client_Form::Disable_Disconnect_Button()
{
	button_disconnect->Disable();
}


void Client_Form::Client_Connection_Event_Post_Message(wxCommandEvent& event)
{
	Post_Message(event.GetString().ToStdString());
}

void Client_Form::Client_Connection_Event_Set_Connection_Initialized(wxCommandEvent& event)
{
	Set_Connection_Initialized(event.GetInt());
}


void Client_Form::Clear_Message_Box()
{
	Message_Box->SetValue("");
}


void Client_Form::OnKeyDown(wxKeyEvent& event)
{
	if (event.GetKeyCode() == WXK_RETURN && connection_initialized && input_box->GetValue() != "")
	{
		Lock_Client_Message.lock();
		client_message = input_box->GetValue();
		Lock_Client_Message.unlock();
		input_box->SetValue("");
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


bool Client_Form::Connection_Is_Initialized()
{
	return connection_initialized;
}


void Client_Form::Set_Disconnected(bool flag)
{
	user_disconnected = flag;
}
