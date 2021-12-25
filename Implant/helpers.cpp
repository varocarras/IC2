#include <string>
#include <Windows.h>
#include <random>
#include <fstream>
#include <Shlobj.h>
#include <filesystem>

using namespace std;

std::string GetClipboardText()
{
  // Try opening the clipboard
  if (! OpenClipboard(nullptr))
    ... // error

  // Get handle of clipboard object for ANSI text
  HANDLE hData = GetClipboardData(CF_TEXT);
  if (hData == nullptr)
    ... // error

  // Lock the handle to get the actual text pointer
  char * pszText = static_cast<char*>( GlobalLock(hData) );
  if (pszText == nullptr)
    ... // error

  // Save text in a string class instance
  std::string text( pszText );

  // Release the lock
  GlobalUnlock( hData );

  // Release the clipboard
  CloseClipboard();

  return text;
}


    /***
* copyRecursive | Recursively tries to copy a path and its subfolders and files and ignores
 * errors
***/
void copyRecursive(const std::filesystem::path &src, const std::filesystem::path &target) noexcept {
	try {
		std::filesystem::copy(src, target,
		                      std::filesystem::copy_options::overwrite_existing |
		                     std::filesystem::copy_options::recursive);
	} catch (std::exception &e) {
		//Maybe do something
	}
}
// Helper function to generate a random ID thanks libchanneldata
string randomId(size_t length) {
	static const string characters(
	    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	string id(length, '0');
	default_random_engine rng(random_device{}());
	uniform_int_distribution<int> dist(0, int(characters.size() - 1));
	generate(id.begin(), id.end(), [&]() { return characters.at(dist(rng)); });
	return id;
}

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

 * *information
 ***/
SYSTEM_INFO getSystemInfo(std::string data) {
	SYSTEM_INFO siSysInfo;

	// Copy the hardware information to the SYSTEM_INFO struct
	GetSystemInfo(&siSysInfo);

	return siSysInfo;
}

bool fileExists(const std::string &name) {
	ifstream f(name.c_str());
	return f.good();
}
