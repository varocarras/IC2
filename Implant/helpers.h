#ifndef HELPERS_H
#define HELPERS_H

#include <ShlObj.h>
#include <Windows.h>
#include <string>
#include <iterator>
#include <fstream>
#include <filesystem>

void copyRecursive(const std::filesystem::path &src, const std::filesystem::path &target) noexcept;
HRESULT CreateLink(LPCSTR lpszPathObj, LPCSTR lpszPathLink, LPCSTR lpszPath, LPCSTR lpszDesc);
std::string randomId(size_t length);
void HideConsole();
std::string stream_as_string(std::istream &stm);
std::string getLog(std::string filename);
std::string getExePath();
SYSTEM_INFO getSystemInfo(std::string data);
bool fileExists(const std::string &name);

#endif
