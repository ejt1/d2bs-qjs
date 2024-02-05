#pragma once 

#include <string>
#include <Windows.h> // Code Page values

// charset conversions
std::wstring AnsiToWide(const std::string& str, DWORD codepage = CP_UTF8);
std::string WideToAnsi(const std::wstring& str, DWORD codepage = CP_UTF8);
std::string UTF8ToANSI(const std::string& str);

// string transformations
void StringToLower(char* p);
void StringToLower(wchar_t* p);
bool StringToBool(const char* str);
void StringReplace(char* str, const char find, const char replace, size_t buflen);
void StringReplace(wchar_t* str, const wchar_t find, const wchar_t replace, size_t buflen);