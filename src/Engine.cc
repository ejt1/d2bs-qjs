#include "Engine.h"

#include <shlwapi.h>
#include <io.h>
#include <fcntl.h>

#include "dde.h"
#include "Offset.h"
#include "ScriptEngine.h"
#include "Helpers.h"
#include "D2Handlers.h"
#include "Console.h"
#include "D2Ptrs.h"
#include "CommandLine.h"
#include "Core.h"
#include "Unit.h"
#include "D2Handlers.h"

#ifdef _MSVC_DEBUG
#include "D2Loader.h"
#endif

Engine::Engine() : m_hModule(nullptr), m_hThread(INVALID_HANDLE_VALUE) {
}

Engine::~Engine() {
}

bool Engine::Initialize(HMODULE hModule, LPVOID lpReserved) {
  m_hModule = hModule;

  // start of old DllMain DLL_PROCESS_ATTACH
  if (lpReserved != NULL) {
    Vars.pModule = (Module*)lpReserved;

    if (!Vars.pModule)
      return FALSE;

    wcscpy_s(Vars.szPath, MAX_PATH, Vars.pModule->szPath);
    Vars.bLoadedWithCGuard = TRUE;
  } else {
    Vars.hModule = hModule;
    GetModuleFileNameW(hModule, Vars.szPath, MAX_PATH);
    PathRemoveFileSpecW(Vars.szPath);
    wcscat_s(Vars.szPath, MAX_PATH, L"\\");
    Vars.bLoadedWithCGuard = FALSE;
  }

  swprintf_s(Vars.szLogPath, _countof(Vars.szLogPath), L"%slogs\\", Vars.szPath);
  CreateDirectoryW(Vars.szLogPath, NULL);
  InitCommandLine();
  ParseCommandLine(Vars.szCommandLine);
  InitSettings();
  sLine* command = NULL;
  Vars.bUseRawCDKey = FALSE;

  command = GetCommand(L"-title");
  if (command) {
    int len = wcslen((wchar_t*)command->szText);
    wcsncat_s(Vars.szTitle, (wchar_t*)command->szText, len);
  }

  if (GetCommand(L"-sleepy"))
    Vars.bSleepy = TRUE;

  if (GetCommand(L"-cachefix"))
    Vars.bCacheFix = TRUE;

  if (GetCommand(L"-multi"))
    Vars.bMulti = TRUE;

  if (GetCommand(L"-ftj"))
    Vars.bReduceFTJ = TRUE;

  command = GetCommand(L"-d2c");
  if (command) {
    Vars.bUseRawCDKey = TRUE;
    const char* keys = UnicodeToAnsi(command->szText);
    strncat_s(Vars.szClassic, keys, strlen(keys));
    delete[] keys;
  }

  command = GetCommand(L"-d2x");
  if (command) {
    const char* keys = UnicodeToAnsi(command->szText);
    strncat_s(Vars.szLod, keys, strlen(keys));
    delete[] keys;
  }

#if 0
		char errlog[516] = "";
		sprintf_s(errlog, 516, "%sd2bs.log", Vars.szPath);
		AllocConsole();
		int handle = _open_osfhandle((long)GetStdHandle(STD_ERROR_HANDLE), _O_TEXT);
		FILE* f = _fdopen(handle, "wt");
		*stderr = *f;
		setvbuf(stderr, NULL, _IONBF, 0);
		freopen_s(&f, errlog, "a+t", f);
#endif

  Vars.bShutdownFromDllMain = FALSE;
  SetUnhandledExceptionFilter(ExceptionHandler);

  // start of old Startup function
  InitializeCriticalSection(&Vars.cEventSection);
  InitializeCriticalSection(&Vars.cRoomSection);
  InitializeCriticalSection(&Vars.cMiscSection);
  InitializeCriticalSection(&Vars.cScreenhookSection);
  InitializeCriticalSection(&Vars.cPrintSection);
  InitializeCriticalSection(&Vars.cBoxHookSection);
  InitializeCriticalSection(&Vars.cFrameHookSection);
  InitializeCriticalSection(&Vars.cLineHookSection);
  InitializeCriticalSection(&Vars.cImageHookSection);
  InitializeCriticalSection(&Vars.cTextHookSection);
  InitializeCriticalSection(&Vars.cFlushCacheSection);
  InitializeCriticalSection(&Vars.cConsoleSection);
  InitializeCriticalSection(&Vars.cGameLoopSection);
  InitializeCriticalSection(&Vars.cUnitListSection);
  InitializeCriticalSection(&Vars.cFileSection);

  Vars.bNeedShutdown = TRUE;
  Vars.bChangedAct = FALSE;
  Vars.bGameLoopEntered = FALSE;
  Vars.SectionCount = 0;

  // MessageBox(NULL, "qwe", "qwe", MB_OK);
  Genhook::Initialize();
  DefineOffsets();
  InstallPatches();
  InstallConditional();
  CreateDdeServer();

  if ((m_hThread = CreateThread(NULL, NULL, EngineThread, NULL, NULL, NULL)) == NULL)
    return FALSE;

  return TRUE;
}

void Engine::Shutdown() {
  if (!Vars.bNeedShutdown)
    return;

  Vars.bActive = FALSE;
  if (!Vars.bShutdownFromDllMain)
    WaitForSingleObject(m_hThread, INFINITE);

  SetWindowLong(D2GFX_GetHwnd(), GWL_WNDPROC, (LONG)Vars.oldWNDPROC);

  RemovePatches();
  Genhook::Destroy();
  ShutdownDdeServer();

  KillTimer(D2GFX_GetHwnd(), Vars.uTimer);

  UnhookWindowsHookEx(Vars.hMouseHook);
  UnhookWindowsHookEx(Vars.hKeybHook);

  DeleteCriticalSection(&Vars.cRoomSection);
  DeleteCriticalSection(&Vars.cMiscSection);
  DeleteCriticalSection(&Vars.cScreenhookSection);
  DeleteCriticalSection(&Vars.cPrintSection);
  DeleteCriticalSection(&Vars.cBoxHookSection);
  DeleteCriticalSection(&Vars.cFrameHookSection);
  DeleteCriticalSection(&Vars.cLineHookSection);
  DeleteCriticalSection(&Vars.cImageHookSection);
  DeleteCriticalSection(&Vars.cTextHookSection);
  DeleteCriticalSection(&Vars.cFlushCacheSection);
  DeleteCriticalSection(&Vars.cConsoleSection);
  DeleteCriticalSection(&Vars.cGameLoopSection);
  DeleteCriticalSection(&Vars.cUnitListSection);
  DeleteCriticalSection(&Vars.cFileSection);

  Log(L"D2BS Shutdown complete.");
  Vars.bNeedShutdown = false;
}

DWORD __stdcall Engine::EngineThread(LPVOID lpThreadParameter) {
  UNREFERENCED_PARAMETER(lpThreadParameter);

  sLine* command;
  bool beginStarter = true;
  bool bInGame = false;

  if (!InitHooks()) {
    wcscpy_s(Vars.szPath, MAX_PATH, L"common");
    Log(L"D2BS Engine startup failed. %s", Vars.szCommandLine);
    Print(L"\u00FFc2D2BS\u00FFc0 :: Engine startup failed!");
    return FALSE;
  }

  ParseCommandLine(Vars.szCommandLine);

  command = GetCommand(L"-handle");

  if (command) {
    Vars.hHandle = (HWND)_wtoi(command->szText);
  }

  command = GetCommand(L"-mpq");

  if (command) {
    char* mpq = UnicodeToAnsi(command->szText);
    LoadMPQ(mpq);
    delete[] mpq;
  }

  command = GetCommand(L"-profile");

  if (command) {
    const wchar_t* profile = command->szText;
    if (SwitchToProfile(profile))
      Print(L"\u00FFc2D2BS\u00FFc0 :: Switched to profile %s", profile);
    else
      Print(L"\u00FFc2D2BS\u00FFc0 :: Profile %s not found", profile);
  }

  Log(L"D2BS Engine startup complete. %s", D2BS_VERSION);
  Print(L"\u00FFc2D2BS\u00FFc0 :: Engine startup complete!");

  while (Vars.bActive) {
    switch (ClientState()) {
      case ClientStateInGame: {
        if (bInGame) {
          if ((Vars.dwMaxGameTime && Vars.dwGameTime && (GetTickCount() - Vars.dwGameTime) > Vars.dwMaxGameTime) ||
              (!D2COMMON_IsTownByLevelNo(GetPlayerArea()) && (Vars.nChickenHP && Vars.nChickenHP >= GetUnitHP(D2CLIENT_GetPlayerUnit())) ||
               (Vars.nChickenMP && Vars.nChickenMP >= GetUnitMP(D2CLIENT_GetPlayerUnit()))))
            D2CLIENT_ExitGame();
        } else {
          Vars.dwGameTime = GetTickCount();

          Sleep(500);

          D2CLIENT_InitInventory();
          sScriptEngine->ForEachScript(
              [](Script* script, void*, uint) {
                script->UpdatePlayerGid();
                return true;
              },
              NULL, 0);
          sScriptEngine->UpdateConsole();
          Vars.bQuitting = false;
          GameJoined();

          bInGame = true;
        }
        break;
      }
      case ClientStateMenu: {
        while (Vars.bUseProfileScript) {
          Sleep(100);
        }
        MenuEntered(beginStarter);
        beginStarter = false;
        if (bInGame) {
          Vars.dwGameTime = NULL;
          bInGame = false;
        }
        break;
      }
      case ClientStateBusy:
      case ClientStateNull:
        break;
    }
    Sleep(50);
    /*	EnterCriticalSection(&sScriptEngine->lock);
            for(ScriptMap::iterator it = sScriptEngine->scripts.begin(); it != sScriptEngine->scripts.end(); it++)
                    JS_TriggerOperationCallback(it->second->GetContext());
            LeaveCriticalSection(&sScriptEngine->lock);  */
  }

  sScriptEngine->Shutdown();

  return NULL;
}
