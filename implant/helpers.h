#ifndef HELPERS_H
#define HELPERS_H

#include <ShlObj.h>
#include <Windows.h>
#include <string>
#include <iterator>
#include <fstream>

HRESULT CreateLink(LPCSTR lpszPathObj, LPCSTR lpszPathLink, LPCSTR lpszPath, LPCSTR lpszDesc);

void HideConsole();
std::string stream_as_string(std::istream &stm);
std::string getLog(std::string filename);
std::string getExePath();
SYSTEM_INFO getSystemInfo(std::string data);

#endif
