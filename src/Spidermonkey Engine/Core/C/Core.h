#pragma once

#include <windows.h>
#include <string>
#include <list>

#include "D2Structs.h"

bool SplitLines(const std::wstring& str, size_t maxlen, const wchar_t delim, std::list<std::wstring>& lst);
void Print(const wchar_t* szFormat, ...);
void __fastcall Say(const wchar_t* szMessage, ...);
bool ClickMap(DWORD dwClickType, int wX, int wY, BOOL bShift, UnitAny* pUnit);
void LoadMPQ(const char* mpq);
void LoadMPQ(const wchar_t* mpq);
int UTF8FindByteIndex(std::string str, int maxutf8len);
int UTF8Length(std::string str);
int MeasureText(const std::wstring& str, int index);
int MaxLineFit(const std::wstring& str, int start_idx, int end_idx, int maxWidth);