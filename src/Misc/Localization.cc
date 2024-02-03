#include "Localization.h"

#include <oleauto.h>
#include <wtypes.h>

std::wstring AnsiToWide(const std::string& str, DWORD codepage) {
  std::wstring out;

  if (str.empty())
    return out;

  // Calculate length of out string
  int32_t len = MultiByteToWideChar(codepage, 0, str.c_str(), static_cast<int32_t>(str.length()), 0, 0);
  out.resize(len);

  // Convert
  MultiByteToWideChar(codepage, 0, str.c_str(), static_cast<int32_t>(str.length()), out.data(), len);

  return out;
}

std::string WideToAnsi(const std::wstring& str, DWORD codepage) {
  std::string out;

  if (str.empty())
    return out;

  // Calculate length of out string
  int32_t len = WideCharToMultiByte(codepage, 0, str.c_str(), static_cast<int32_t>(str.length()), 0, 0, nullptr, nullptr);
  out.resize(len);

  // Convert
  WideCharToMultiByte(codepage, 0, str.c_str(), static_cast<int32_t>(str.length()), out.data(), len, nullptr, nullptr);

  return out;
}

// https://stackoverflow.com/questions/8298081/convert-utf-8-to-ansi-in-c
std::string UTF8ToANSI(const std::string& str) {
  BSTR bstrWide;
  char* pszAnsi;
  int nLength;
  const char* pszCode = str.c_str();

  nLength = MultiByteToWideChar(CP_UTF8, 0, pszCode, strlen(pszCode) + 1, NULL, NULL);
  bstrWide = SysAllocStringLen(NULL, nLength);
  if (!bstrWide) {
    return "CONVERSION ERROR";
  }
  MultiByteToWideChar(CP_UTF8, 0, pszCode, strlen(pszCode) + 1, bstrWide, nLength);

  nLength = WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, NULL, 0, NULL, NULL);
  pszAnsi = new char[nLength];

  WideCharToMultiByte(CP_ACP, 0, bstrWide, -1, pszAnsi, nLength, NULL, NULL);
  SysFreeString(bstrWide);

  std::string r(pszAnsi);
  delete[] pszAnsi;
  return r;
}

void StringToLower(char* p) {
  for (; *p; ++p) *p = static_cast<char>(tolower(*p));
}

void StringToLower(wchar_t* p) {
  for (; *p; ++p) *p = static_cast<wchar_t>(towlower(*p));
}

bool StringToBool(const char* str) {
  switch (tolower(str[0])) {
    case 't':
    case '1':
      return true;
    case 'f':
    case '0':
    default:
      return false;
  }
}

bool StringToBool(const wchar_t* str) {
  switch (tolower(str[0])) {
    case 't':
    case '1':
      return true;
    case 'f':
    case '0':
    default:
      return false;
  }
}

void StringReplace(char* str, const char find, const char replace, size_t buflen) {
  for (size_t i = 0; i < buflen; i++) {
    if (str[i] == find)
      str[i] = replace;
  }
}

void StringReplace(wchar_t* str, const wchar_t find, const wchar_t replace, size_t buflen) {
  for (size_t i = 0; i < buflen; i++) {
    if (str[i] == find)
      str[i] = replace;
  }
}