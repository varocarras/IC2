
#include <ShlObj.h>
#include <Windows.h>
#include "helpers.h"
#include <string>

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

std::string getExePath() {
	char result[MAX_PATH];
	return std::string(result, GetModuleFileName(NULL, result, MAX_PATH));
}
