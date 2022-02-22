#include "windows.h"
extern "C" __declspec(dllimport) void showMessageBox();

int main()
{
	showMessageBox();
	return 0;
}