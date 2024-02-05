#pragma once

#include <windows.h>
#include <string>
#include <list>

#include "Game/Units/Units.h"

bool SplitLines(const std::string& str, size_t maxlen, const char delim, std::list<std::string>& lst);
void Print(const char* szFormat, ...);
void __fastcall Say(const wchar_t* szMessage, ...);
bool ClickMap(DWORD dwClickType, int wX, int wY, BOOL bShift, D2UnitStrc* pUnit);
void LoadMPQ(const char* mpq);
int UTF8FindByteIndex(std::string str, int maxutf8len);
int UTF8Length(std::string str);
int MeasureText(const std::string& str, int index);
int MaxLineFit(const std::string& str, int start_idx, int end_idx, int maxWidth);