#include <wx/wx.h>

extern wxSize Screen_Size;

int Get_Lowest_Pixel_Size()
{

	return Screen_Size.GetWidth() > Screen_Size.GetHeight() ? Screen_Size.GetHeight() : Screen_Size.GetWidth();
}