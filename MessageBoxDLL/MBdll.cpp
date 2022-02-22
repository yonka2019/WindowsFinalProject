#include "windows.h"
#define DLL_EXPORT
#include "MBdll.h"
#include "stdio.h"

extern "C"
{
	DECLDIR void showMessageBox() {
		MessageBoxA(NULL, "DLL successfully injected into Notepad.exe", "Success.", NULL);
	}
}
BOOL APIENTRY DllMain(HANDLE hModule, // Handle to DLL module
	DWORD ul_reason_for_call,
	LPVOID lpReserved) // Reserved
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		// A process is loading the DLL.
		showMessageBox();
		break;
	}
	return TRUE;
}