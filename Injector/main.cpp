#include "windows.h"
#include "TlHelp32.h"
#include <iostream>

#define DLL "D:\\messageBoxDLL.dll"
#define PROC "Notepad.exe" // Windows 11 - Notepad.exe

bool injectDLL(DWORD processID, const char* dllPath);
DWORD getProcessIDByName(const char* process_name_);

int main()
{
	DWORD processID = getProcessIDByName(PROC);

	std::cout << "Process ID matched: " << processID << std::endl;
	std::cout << "Injection: " << (injectDLL(processID, DLL) ? "Success" : "Fail") << std::endl;

	return 0;
}
/// <summary>
/// Returns the process id by the process name
/// </summary>
/// <param name="process_name_">Process name (for example: Notepad.exe)</param>
/// <returns>Process ID accroding the process name</returns>
DWORD getProcessIDByName(const char* processName)
{
	PROCESSENTRY32 process_entry = { sizeof(PROCESSENTRY32) };
	HANDLE processes_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	// loop through all process to find one that matches the processName
	if (Process32First(processes_snapshot, &process_entry))
	{
		do
		{
			if (strcmp(process_entry.szExeFile, processName) == 0)
			{
				CloseHandle(processes_snapshot);
				return process_entry.th32ProcessID;
			}
		} while (Process32Next(processes_snapshot, &process_entry));
	}

	CloseHandle(processes_snapshot);
	return NULL;
}
/// <summary>
/// Injects the given dll into the process
/// </summary>
/// <param name="process_id_">Process id to inject into the given DLL</param>
/// <param name="dll_path">Path of the dll to inject</param>
/// <returns>True if success, either, false.</returns>
bool injectDLL(DWORD processID, const char* dllPath)
{
	// Get full path of DLL to inject
	char dllFullName[MAX_PATH] = { 0 };
	DWORD err;
	GetFullPathNameA(dllPath, MAX_PATH, dllFullName, NULL);

	// Get LoadLibrary function address –
	// the address doesn't change at remote process
	PVOID addrLoadLibrary = (PVOID)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
	// Open remote process
	HANDLE proc = OpenProcess(PROCESS_ALL_ACCESS, false, processID);

	// Get a pointer to memory location in remote process,
	// big enough to store DLL path
	PVOID memAddr = (PVOID)VirtualAllocEx(proc, 0, strlen(dllFullName), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (NULL == memAddr) {
		err = GetLastError();
		return false;
	}
	// Write DLL name to remote process memory
	BOOL check = WriteProcessMemory(proc, memAddr, dllFullName, strlen(dllFullName), NULL);
	if (0 == check) {
		err = GetLastError();
		return false;
	}
	// Open remote thread, while executing LoadLibrary
	// with parameter DLL name, will trigger DLLMain
	HANDLE hRemote = CreateRemoteThread(proc, 0, 0, (LPTHREAD_START_ROUTINE)addrLoadLibrary, memAddr, 0, 0);
	if (NULL == hRemote) {
		err = GetLastError();
		return false;
	}
	WaitForSingleObject(hRemote, INFINITE);
	check = CloseHandle(hRemote);
	return true;
}