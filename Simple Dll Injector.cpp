#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include "shlobj_core.h"
#include <filesystem>
#include <string>

using namespace std;

string GetDesktopPath() {
	char path[_MAX_PATH];

	HRESULT result = SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, SHGFP_TYPE_CURRENT, path);

	if (SUCCEEDED(result)) {
		//cout << "Desktop Path " << path << endl;
		return path;

	} else {
		cout << "Failed to get path!" << endl;
		return "error";

	}
}

DWORD GetProcId(const char* procName) {
	
	DWORD procId = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap != INVALID_HANDLE_VALUE) {

		DWORD firstProcId;

		PROCESSENTRY32 procEntry;
		procEntry.dwSize = sizeof(procEntry);

		if (Process32First(hSnap, &procEntry)) {
			firstProcId = procEntry.th32ProcessID;

			while (Process32Next(hSnap, &procEntry)) {

				if (strcmp(procEntry.szExeFile, procName) == 0) {	//if equal, returns 0

					procId = procEntry.th32ProcessID;
					wcout << L"Proccess found!\nOpening process ID: " << procId << "|" << procName << endl;

					break;
				}
				//wcout << L"Stack number: " << procEntry.th32ProcessID << "|" << procEntry.szExeFile << endl;
				if (procEntry.th32ProcessID == firstProcId) {
					break;
				}
			}
		}

	} else {
		cout << "Invalid handle value! Aborting Search..." << endl;
	}

	CloseHandle(hSnap);
	return procId;
}


int main() {
	DWORD procId = 0;

	string bufferPath = GetDesktopPath();
	if (bufferPath == "error") {
		cout << "Couldn't get desktop path, quitting..." << endl;
		return 0;
	}
	bufferPath = bufferPath + "\\";

	

	//console menu
		//Get process name
	cout << "Insert Process Name (w/o '.exe'): ";
	string proctemp;
	cin >> proctemp;
	proctemp = proctemp + ".exe";
	const char* procName = proctemp.c_str();
	
	//const wchar_t* procName = proctemp;

		//Get dll name
	cout << "Insert dll file Name (w/o '.dll'): " << bufferPath;
	string bufferDll;
	cin >> bufferDll;


	
	bufferDll = bufferPath + bufferDll + ".dll";
	const char* constDllPath = bufferDll.c_str();


	cout << "Proc Name: " << procName << endl;
	cout << "Dll Path selected: " << constDllPath << endl;
	cout << "Searching for " << procName << "..." << endl;
	
	procId = GetProcId(procName);
	if (procId == 0) {
		cout << endl << "Process not found, aborting..." << endl;
		return 0;
	}

	

	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, procId);

	if (hProc && hProc != INVALID_HANDLE_VALUE) {

		void * loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		if (loc) {
			WriteProcessMemory(hProc, loc, constDllPath, strlen(constDllPath) + 1, 0);
		}

		HANDLE hTread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);

		if (hTread) {
			CloseHandle(hTread);
		}

	}
	else {
		cout << "Something went wrong! Exiting..." << endl;
	}


	if (hProc) {
		CloseHandle(hProc);
	}
	cout << "Finished!\nEnter 'r' to re-run or any other letter to exit";
	char var = 0;
	cin >> var;
	
	if (var == 'r') {
		main();
	} else {
		return 0;
	}

	return 0;
}