#include <windows.h>

int CALLBACK WinMain(HINSTANCE h_instance, HINSTANCE h_prev_instance, LPSTR lp_cmd_line, int n_cmd_show)
{
	MessageBoxA(0, "Welcome to Handmade Hoss!", "Handmade Hoss", MB_OK | MB_ICONINFORMATION);
	return 0;
}
