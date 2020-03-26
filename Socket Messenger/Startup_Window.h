#ifndef STARTUP_WINDOW
#define STARTUP_WINDOW

#include <wx/frame.h>

enum
{
	ID_INIT_FORM,
	ID_SET_CLIENT,
	ID_SET_SERVER
};


class Startup_Window : public wxFrame
{
public:
	Startup_Window(const std::string, const int, const int, int, int);
	void InitForm(wxCommandEvent&);
	void SetClient(wxCommandEvent&);
	void SetServer(wxCommandEvent&);
private:
	bool server_selected;
	bool client_selected;
	DECLARE_EVENT_TABLE()
};

#endif