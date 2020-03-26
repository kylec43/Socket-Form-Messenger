#include <wx/frame.h>
#include <wx/panel.h>
#include <wx/radiobut.h>
#include <wx/button.h>
#include <wx/msgdlg.h>
#include <string>
#include "Startup_Window.h"
#include "Server_Form.h"
#include "Client_Form.h"
#include "Get_Lowest_Pixel_Size.h"

extern wxSize Screen_Size;

BEGIN_EVENT_TABLE(Startup_Window, wxFrame)
	EVT_BUTTON(ID_INIT_FORM, Startup_Window::InitForm)
	EVT_RADIOBUTTON(ID_SET_SERVER, Startup_Window::SetServer)
	EVT_RADIOBUTTON(ID_SET_CLIENT, Startup_Window::SetClient)
END_EVENT_TABLE()

Startup_Window::Startup_Window(const std::string title, const int x, const int y, int width, int height) :
	wxFrame(NULL, wxID_ANY, title, wxPoint(x, y), wxSize(width, height), wxDEFAULT_FRAME_STYLE & ~wxRESIZE_BORDER & ~wxMAXIMIZE_BOX),
	client_selected(false),
	server_selected(false)
{

	/*Convert width and height to frame size without border*/
	width = GetClientSize().GetWidth();
	height = GetClientSize().GetHeight();

	wxPanel* panel = new wxPanel(this, wxID_ANY);
	wxRadioButton* radio_button_server = new wxRadioButton(panel, ID_SET_SERVER, "Server", wxPoint(width * 0.20, height * 0.40), wxSize(width * 0.20, width*0.10));
	wxRadioButton* radio_button_client = new wxRadioButton(panel, ID_SET_CLIENT, "Client", wxPoint(width * 0.65, height * 0.40), wxSize(width * 0.20, width * 0.10));
	int font_size = radio_button_server->GetSize().GetHeight()/3;
	wxFont font(font_size, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	radio_button_server->SetFont(font);
	radio_button_client->SetFont(font);

	int button_run_width = width * 0.20;
	int button_run_height = height * 0.20;
	int button_run_x = width / 2 - button_run_width / 2;
	int button_run_y = (height - button_run_height) * 0.90;
	wxButton* button_run = new wxButton(panel, ID_INIT_FORM, "Run", wxPoint(button_run_x, button_run_y), wxSize(button_run_width, button_run_height));
}


void Startup_Window::InitForm(wxCommandEvent& event)
{
	//Create Form
	int form_width = Get_Lowest_Pixel_Size();
	int form_height = form_width * 0.90;
	int form_x = Screen_Size.GetWidth() / 2 - form_width / 2;
	int form_y = Screen_Size.GetHeight() / 2 - form_height / 2;

	if (server_selected)
	{
		Server_Form* server_form = new Server_Form("Server", form_x, form_y, form_width, form_height);
		server_form->Show();
	}
	else if (client_selected)
	{
		Client_Form* client_form = new Client_Form("Client", form_x, form_y, form_width, form_height);
		client_form->Show();
	}
	Close();
}



void Startup_Window::SetClient(wxCommandEvent&)
{
	server_selected = false;
	client_selected = true;
}

void Startup_Window::SetServer(wxCommandEvent&)
{
	client_selected = false;
	server_selected = true;
}