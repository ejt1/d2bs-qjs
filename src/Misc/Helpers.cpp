#include "Engine.h"
#include "Core.h"
#include "Script.h"
#include "ScriptEngine.h"
#include "string.h"
#include "D2Handlers.h"
#include "Control.h"
#include "D2Ptrs.h"
#include "Helpers.h"
#include "DbgHelp.h"
#include "Profile.h"
#include "StackWalker.h"
#include <tlhelp32.h>
#include <wctype.h>

bool SwitchToProfile(const char* profile) {
  if (Vars.bUseProfileScript != TRUE || !Profile::ProfileExists(profile))
    return false;

  char file[_MAX_FNAME + _MAX_PATH] = "";
  char scriptPath[_MAX_PATH] = "";
  char defaultStarter[_MAX_PATH] = "";
  char defaultConsole[_MAX_PATH] = "";
  char defaultGame[_MAX_PATH] = "";

  sprintf_s(file, _countof(file), "%sd2bs.ini", Vars.szPath);

  GetPrivateProfileStringA(profile, "ScriptPath", "scripts", scriptPath, _MAX_PATH, file);
  GetPrivateProfileStringA(profile, "DefaultConsoleScript", "", defaultConsole, _MAX_FNAME, file);
  GetPrivateProfileStringA(profile, "DefaultGameScript", "", defaultGame, _MAX_FNAME, file);
  GetPrivateProfileStringA(profile, "DefaultStarterScript", "", defaultStarter, _MAX_FNAME, file);

  strcpy_s(Vars.szProfile, 256, profile);
  sprintf_s(Vars.szScriptPath, _MAX_PATH, "%s%s", Vars.szPath, scriptPath);
  if (strlen(defaultConsole) > 0)
    strcpy_s(Vars.szConsole, _MAX_FNAME, defaultConsole);
  if (strlen(defaultGame) > 0)
    strcpy_s(Vars.szDefault, _MAX_FNAME, defaultGame);
  if (strlen(defaultStarter) > 0)
    strcpy_s(Vars.szStarter, _MAX_FNAME, defaultStarter);

  Vars.bUseProfileScript = FALSE;
  // Reload();
  return true;
}

void InitSettings(void) {
  char hosts[256], debug[6], quitOnHostile[6], quitOnError[6], maxGameTime[6], gameTimeout[6], startAtMenu[6], disableCache[6],
      memUsage[6], gamePrint[6], useProfilePath[6], logConsole[6], enableUnsupported[6], forwardMessageBox[6], consoleFont[6];

  char file[_MAX_FNAME + _MAX_PATH] = "";
  char scriptPath[_MAX_PATH] = "";
  char defaultStarter[_MAX_PATH] = "";
  char defaultConsole[_MAX_PATH] = "";
  char defaultGame[_MAX_PATH] = "";
  sprintf_s(file, _countof(file), "%sd2bs.ini", Vars.szPath);

  GetPrivateProfileStringA("settings", "ScriptPath", "scripts", scriptPath, _MAX_PATH, file);
  GetPrivateProfileStringA("settings", "DefaultConsoleScript", "", defaultConsole, _MAX_FNAME, file);
  GetPrivateProfileStringA("settings", "DefaultGameScript", "default.dbj", defaultGame, _MAX_FNAME, file);
  GetPrivateProfileStringA("settings", "DefaultStarterScript", "starter.dbj", defaultStarter, _MAX_FNAME, file);
  GetPrivateProfileStringA("settings", "Hosts", "", hosts, 256, file);
  GetPrivateProfileStringA("settings", "MaxGameTime", "0", maxGameTime, 6, file);
  GetPrivateProfileStringA("settings", "Debug", "false", debug, 6, file);
  GetPrivateProfileStringA("settings", "QuitOnHostile", "false", quitOnHostile, 6, file);
  GetPrivateProfileStringA("settings", "QuitOnError", "false", quitOnError, 6, file);
  GetPrivateProfileStringA("settings", "StartAtMenu", "true", startAtMenu, 6, file);
  GetPrivateProfileStringA("settings", "DisableCache", "true", disableCache, 6, file);
  GetPrivateProfileStringA("settings", "MemoryLimit", "100", memUsage, 6, file);
  GetPrivateProfileStringA("settings", "UseGamePrint", "false", gamePrint, 6, file);
  GetPrivateProfileStringA("settings", "GameReadyTimeout", "5", gameTimeout, 6, file);
  GetPrivateProfileStringA("settings", "UseProfileScript", "false", useProfilePath, 6, file);
  GetPrivateProfileStringA("settings", "LogConsoleOutput", "false", logConsole, 6, file);
  GetPrivateProfileStringA("settings", "EnableUnsupported", "false", enableUnsupported, 6, file);
  GetPrivateProfileStringA("settings", "ForwardMessageBox", "false", forwardMessageBox, 6, file);
  GetPrivateProfileStringA("settings", "ConsoleFont", "0", consoleFont, 6, file);
  sprintf_s(Vars.szScriptPath, _MAX_PATH, "%s%s", Vars.szPath, scriptPath);
  strcpy_s(Vars.szStarter, _MAX_FNAME, defaultStarter);
  strcpy_s(Vars.szConsole, _MAX_FNAME, defaultConsole);
  strcpy_s(Vars.szDefault, _MAX_FNAME, defaultGame);

  strcpy_s(Vars.szHosts, 256, hosts);

  Vars.dwGameTime = GetTickCount();
  Vars.dwMaxGameTime = abs(atoi(maxGameTime) * 1000);
  Vars.dwGameTimeout = abs(atoi(gameTimeout) * 1000);

  Vars.bQuitOnHostile = StringToBool(quitOnHostile);
  Vars.bQuitOnError = StringToBool(quitOnError);
  Vars.bStartAtMenu = StringToBool(startAtMenu);
  Vars.bDisableCache = StringToBool(disableCache);
  Vars.bUseGamePrint = StringToBool(gamePrint);
  Vars.bUseProfileScript = StringToBool(useProfilePath);
  Vars.bLogConsole = StringToBool(logConsole);
  Vars.bEnableUnsupported = StringToBool(enableUnsupported);
  Vars.bForwardMessageBox = StringToBool(forwardMessageBox);
  Vars.eventSignal = CreateEventA(0, true, false, nullptr);
  Vars.dwMemUsage = abs(atoi(memUsage));
  Vars.dwConsoleFont = abs(atoi(consoleFont));
  if (Vars.dwMemUsage < 1)
    Vars.dwMemUsage = 50;
  Vars.dwMemUsage *= 1024 * 1024;
}

const char* GetStarterScriptName(void) {
  return (ClientState() == ClientStateInGame ? Vars.szDefault : ClientState() == ClientStateMenu ? Vars.szStarter : NULL);
}

ScriptMode GetStarterScriptState(void) {
  // the default return is InGame because that's the least harmful of the options
  return (ClientState() == ClientStateInGame ? kScriptModeGame : ClientState() == ClientStateMenu ? kScriptModeMenu : kScriptModeGame);
}

bool ExecCommand(const char* command) {
  sScriptEngine->RunCommand(command);
  return true;
}

bool StartScript(const char* scriptname, ScriptMode mode) {
  char file[_MAX_FNAME + _MAX_PATH];
  sprintf_s(file, _countof(file), "%s\\%s", Vars.szScriptPath, scriptname);
  Script* script = sScriptEngine->NewScript(file, mode);
  return (script && script->Start());
}

void Reload(void) {
  if (sScriptEngine->GetCount() > 0)
    Print(L"\u00FFc2D2BS\u00FFc0 :: Stopping all scripts");
  sScriptEngine->StopAll();

  if (Vars.bDisableCache != TRUE)
    Print(L"\u00FFc2D2BS\u00FFc0 :: Flushing the script cache");
  sScriptEngine->FlushCache();

  // wait for things to catch up
  Sleep(500);

  if (!Vars.bUseProfileScript) {
    const char* script = GetStarterScriptName();
    if (StartScript(script, GetStarterScriptState()))
      Print(L"\u00FFc2D2BS\u00FFc0 :: Started %s", script);
    else
      Print(L"\u00FFc2D2BS\u00FFc0 :: Failed to start %s", script);
  }
}

bool ProcessCommand(const wchar_t* command, bool unprocessedIsCommand) {
  bool result = false;
  wchar_t* buf = _wcsdup(command);
  wchar_t* next_token1 = NULL;
  wchar_t* argv = wcstok_s(buf, L" ", &next_token1);

  // no command?
  if (argv == NULL)
    return false;

  if (_wcsicmp(argv, L"start") == 0) {
    const char* script = GetStarterScriptName();
    if (StartScript(script, GetStarterScriptState()))
      Print(L"\u00FFc2D2BS\u00FFc0 :: Started %S", script);
    else
      Print(L"\u00FFc2D2BS\u00FFc0 :: Failed to start %S", script);
    result = true;
  } else if (_wcsicmp(argv, L"stop") == 0) {
    if (sScriptEngine->GetCount() > 0)
      Print(L"\u00FFc2D2BS\u00FFc0 :: Stopping all scripts");
    sScriptEngine->StopAll();
    result = true;
  } else if (_wcsicmp(argv, L"flush") == 0) {
    if (Vars.bDisableCache != TRUE)
      Print(L"\u00FFc2D2BS\u00FFc0 :: Flushing the script cache");
    sScriptEngine->FlushCache();
    result = true;
  } else if (_wcsicmp(argv, L"load") == 0) {
    std::string script = WideToAnsi(command + 5);
    if (StartScript(script.c_str(), GetStarterScriptState()))
      Print(L"\u00FFc2D2BS\u00FFc0 :: Started %S", script.c_str());
    else
      Print(L"\u00FFc2D2BS\u00FFc0 :: Failed to start %S", script.c_str());
    result = true;
  } else if (_wcsicmp(argv, L"reload") == 0) {
    Reload();
    result = true;
  }
#if DEBUG
  else if (_wcsicmp(argv, L"crash") == 0) {
    DWORD zero = 0;
    double value = 1 / zero;
    Print(L"%d", value);
  } else if (_wcsicmp(argv, L"profile") == 0) {
    const wchar_t* profile = command + 8;
    if (SwitchToProfile(profile))
      Print(L"ÿc2D2BSÿc0 :: Switched to profile %s", profile);
    else
      Print(L"ÿc2D2BSÿc0 :: Profile %s not found", profile);
    result = true;
  }
#endif
  else if (_wcsicmp(argv, L"exec") == 0 && !unprocessedIsCommand) {
    std::string cmd = WideToAnsi(command + 5);
    ExecCommand(cmd.c_str());
    result = true;
  } else if (unprocessedIsCommand) {
    std::string cmd = WideToAnsi(command);
    ExecCommand(cmd.c_str());
    result = true;
  }
  free(buf);
  return result;
}

SYMBOL_INFO* GetSymFromAddr(HANDLE hProcess, DWORD64 addr) {
  char* symbols = new char[sizeof(SYMBOL_INFO) + 512];
  memset(symbols, 0, sizeof(SYMBOL_INFO) + 512);

  SYMBOL_INFO* sym = (SYMBOL_INFO*)(symbols);
  sym->SizeOfStruct = sizeof(SYMBOL_INFO);
  sym->MaxNameLen = 512;

  DWORD64 dummy;
  bool success = SymFromAddr(hProcess, addr, &dummy, sym) == TRUE ? true : false;
  if (!success) {
    delete[] symbols;
    sym = NULL;
  }

  return sym;
}

IMAGEHLP_LINE64* GetLineFromAddr(HANDLE hProcess, DWORD64 addr) {
  IMAGEHLP_LINE64* line = new IMAGEHLP_LINE64;
  line->SizeOfStruct = sizeof(IMAGEHLP_LINE64);

  DWORD dummy;
  bool success = SymGetLineFromAddr64(hProcess, addr, &dummy, line) == TRUE ? true : false;
  if (!success) {
    delete line;
    line = NULL;
  }
  return line;
}

char* DllLoadAddrStrs() {
  const char* dlls[] = {"D2Client.DLL", "D2Common.DLL", "D2Gfx.DLL",    "D2Lang.DLL", "D2Win.DLL", "D2Net.DLL",  "D2Game.DLL",
                        "D2Launch.DLL", "Fog.DLL",      "BNClient.DLL", "Storm.DLL",  "D2Cmp.DLL", "D2Multi.DLL"};
  size_t strMaxLen;
  char* result;
  char lineBuf[80];
  unsigned int i;

  strMaxLen = sizeof(lineBuf) * sizeof(dlls) / sizeof(dlls[0]);
  result = (char*)malloc(strMaxLen);

  result[0] = '\0';

  for (i = 0; i < sizeof(dlls) / sizeof(dlls[0]); ++i) {
    sprintf_s(lineBuf, sizeof(lineBuf), "%s loaded at: 0x%08x.", dlls[i], (uint32_t)(GetModuleHandle(dlls[i])));
    strcat_s(result, strMaxLen, lineBuf);
    if (i != (sizeof(dlls) / sizeof(dlls[0]) - 1)) {
      strcat_s(result, strMaxLen, "\n");
    }
  }

  return result;
}

LONG WINAPI ExceptionHandler(EXCEPTION_POINTERS* ptrs) {
  static bool already_crashed = false;
  if (already_crashed) {
    return EXCEPTION_EXECUTE_HANDLER;
  }
  // just in case we crash in here :)
  already_crashed = true;

  // spdlog::info("Generating exception dump");
  char crashpath[_MAX_PATH]{};
  sprintf_s(crashpath, "%s\\D2BS_crash_%d-%d.dmp", Vars.szPath, GetCurrentProcessId(), GetCurrentThreadId());

  auto hFile = CreateFileA(crashpath, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
  if (hFile == INVALID_HANDLE_VALUE)
    return EXCEPTION_EXECUTE_HANDLER;

  MINIDUMP_EXCEPTION_INFORMATION exceptionInfo{};
  exceptionInfo.ThreadId = GetCurrentThreadId();
  exceptionInfo.ExceptionPointers = ptrs;
  exceptionInfo.ClientPointers = FALSE;

  MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MINIDUMP_TYPE(MiniDumpWithIndirectlyReferencedMemory), ptrs ? &exceptionInfo : nullptr, nullptr,
                    nullptr);

  CloseHandle(hFile);
  std::terminate();
}

int __cdecl _purecall(void) {
  GetStackWalk();
  return 0;
}

class MyStackWalker : public StackWalker {
 public:
  MyStackWalker() : StackWalker() {
  }

 protected:
  virtual void OnOutput(LPCSTR szText) {
    Log(L"%hs", szText);
  }
};

std::vector<DWORD> GetThreadIds() {
  std::vector<DWORD> threadIds;

  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, GetCurrentProcessId());

  if (hSnapshot != INVALID_HANDLE_VALUE) {
    THREADENTRY32 threadEntry;
    threadEntry.dwSize = sizeof(THREADENTRY32);

    if (Thread32First(hSnapshot, &threadEntry)) {
      do {
        if (threadEntry.th32OwnerProcessID == GetCurrentProcessId())
          threadIds.push_back(threadEntry.th32ThreadID);

      } while (Thread32Next(hSnapshot, &threadEntry));
    }
  }
  CloseHandle(hSnapshot);

  return threadIds;
}

void ResumeProcess() {
  std::vector<DWORD> threadIds = GetThreadIds();

  for (std::vector<DWORD>::iterator it = threadIds.begin(); it != threadIds.end(); ++it) {
    HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, *it);
    ResumeThread(hThread);
    CloseHandle(hThread);
  }
}

void InitCommandLine() {
  char* line = GetCommandLineA();
  memcpy(Vars.szCommandLine, line, min(sizeof(Vars.szCommandLine), sizeof(char) * strlen(line)));

  wchar_t* wline = GetCommandLineW();
  LPCWSTR cline = L"C:\\Program Files (x86)\\Diablo II\\Game.exe -w";
  memcpy(wline, line, sizeof(LPCWSTR) * wcslen(cline));
}

bool GetStackWalk() {
  std::vector<DWORD> threadIds = GetThreadIds();
  DWORD current = GetCurrentThreadId();

  MyStackWalker sw;
  for (std::vector<DWORD>::iterator it = threadIds.begin(); it != threadIds.end(); ++it) {
    if (*it == current)
      continue;

    HANDLE hThread = OpenThread(THREAD_GET_CONTEXT, false, *it);

    if (hThread == INVALID_HANDLE_VALUE)
      return false;

    Log(L"Stack Walk Thread: %d", *it);
    sw.ShowCallstack(hThread);
  }

  Log(L"Stack Walk Thread: %d", current);
  sw.ShowCallstack();

  return true;
}
