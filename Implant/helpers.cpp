
#include <ShlObj.h>
#include <Windows.h>
#include "helpers.h"
#include <string>
#include <iterator>
#include <fstream>

HRESULT CreateLink(LPCSTR lpszPathObj, LPCSTR lpszPathLink, LPCSTR lpszPath, LPCSTR lpszDesc)

/*============================================================================*/
{
	IShellLink *psl = NULL;
	HRESULT hres = CoInitialize(NULL);

	// Get a pointer to the IShellLink interface. It is assumed that CoInitialize
	// has already been called.

	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink,
	                        (LPVOID *)&psl);
	if (SUCCEEDED(hres)) {
		IPersistFile *ppf;

		// Set the path to the shortcut target and add the description.
		psl->SetPath(lpszPathObj);
		psl->SetDescription(lpszDesc);
		psl->SetWorkingDirectory(lpszPath);

		// Query IShellLink for the IPersistFile interface, used for saving the
		// shortcut in persistent storage.
		hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);

		if (SUCCEEDED(hres)) {
			WCHAR wsz[MAX_PATH];

			// Ensure that the string is Unicode.
			MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, wsz, MAX_PATH);

			// Add code here to check return value from MultiByteWideChar
			// for success.

			// Save the link by calling IPersistFile::Save.
			hres = ppf->Save(wsz, TRUE);

			ppf->Release();
		}
		psl->Release();
	}

	CoUninitialize();

	return hres;
}

void HideConsole() { ::ShowWindow(::GetConsoleWindow(), SW_HIDE); }

std::string stream_as_string(std::istream &stm) {
	return {std::istreambuf_iterator<char>(stm), std::istreambuf_iterator<char>{}};
}

std::string getLog(std::string filename) {
	std::ifstream file(filename);
	return stream_as_string(file);
}

std::string getExePath() {
	char result[MAX_PATH];
	return std::string(result, GetModuleFileName(NULL, result, MAX_PATH));
}


/***
 * getSystemInfo | Performs calls to WindowsAPI functions to retrieve hardware and software
 *information
 ***/
SYSTEM_INFO getSystemInfo(std::string data) {
	SYSTEM_INFO siSysInfo;

	// Copy the hardware information to the SYSTEM_INFO struct
	GetSystemInfo(&siSysInfo);

	return siSysInfo;
}

