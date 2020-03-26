#include <wx/wx.h>
#include <wx/display.h>
#include <string>
#include "Startup_Window.h"	
#include "Get_Lowest_Pixel_Size.h" //Returns the smaller number from Screen_Size

wxSize Screen_Size(wxSystemSettings::GetMetric(wxSYS_SCREEN_X), wxSystemSettings::GetMetric(wxSYS_SCREEN_Y) );

class myApp : public wxApp
{
public:

	virtual bool OnInit()
	{

		int startup_window_width = Get_Lowest_Pixel_Size() * 0.50;
		int startup_window_height = startup_window_width / 2;
		int startup_window_x = Screen_Size.GetWidth() / 2 - startup_window_width / 2;
		int startup_window_y = Screen_Size.GetHeight() / 2 - startup_window_height / 2;

		Startup_Window* startup_window = new Startup_Window("Socket Messenger", startup_window_x, startup_window_y, startup_window_width, startup_window_height);
		startup_window->Show();
		return true;
	}

};

IMPLEMENT_APP(myApp);
