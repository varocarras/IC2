#ifndef HELPERS
#define HELPERS

#include <string>
#include <Windows.h>

std::string randomId(size_t length);
HRESULT CreateLink(LPCSTR lpszPathObj, LPCSTR lpszPathLink, LPCSTR lpszPath, LPCSTR lpszDesc);
std::string stream_as_string(std::istream &stm);
std::string getLog(std::string filename);
std::string getExePath();
SYSTEM_INFO getSystemInfo(std::string data);
inline bool fileExists(const std::string &name);

#endif
