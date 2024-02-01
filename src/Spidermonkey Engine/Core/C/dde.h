#pragma once
#include <windows.h>
#include <ddeml.h>

HDDEDATA CALLBACK DdeCallback(UINT uType, UINT uFmt, HCONV hconv, HSZ hsz1, HSZ hsz2, HDDEDATA hdata, DWORD dwData1, DWORD dwData2);
DWORD CreateDdeServer();
BOOL ShutdownDdeServer();
BOOL SendDDE(int mode, const char* pszDDEServer, const char* pszTopic, const char* pszItem, const char* pszData, char** result, unsigned int size);
