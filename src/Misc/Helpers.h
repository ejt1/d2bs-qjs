#pragma once

#ifndef __HELPERS_H__
#define __HELPERS_H__

#include "Script.h"
#include "Localization.h"

bool SwitchToProfile(const char* profile);
void InitSettings(void);
bool ExecCommand(const char* command);
bool StartScript(const char* script, ScriptMode mode);
void Reload(void);
bool ProcessCommand(const wchar_t* command, bool unprocessedIsCommand);
void InitCommandLine();

const char* GetStarterScriptName(void);
ScriptMode GetStarterScriptState(void);

LONG WINAPI ExceptionHandler(EXCEPTION_POINTERS* ptrs);

#endif
