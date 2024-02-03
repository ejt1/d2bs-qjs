#pragma once

#ifndef __HELPERS_H__
#define __HELPERS_H__

#include "Script.h"
#include "Localization.h"

bool GetStackWalk();
bool SwitchToProfile(const char* profile);
void InitSettings(void);
bool ExecCommand(const char* command);
bool StartScript(const char* script, ScriptMode mode);
void Reload(void);
bool ProcessCommand(const wchar_t* command, bool unprocessedIsCommand);
void ResumeProcess();
void InitCommandLine();

const char* GetStarterScriptName(void);
ScriptMode GetStarterScriptState(void);

char* DllLoadAddrStrs();
LONG WINAPI ExceptionHandler(EXCEPTION_POINTERS* ptrs);
int __cdecl _purecall(void);
#endif
