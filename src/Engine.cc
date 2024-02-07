#include "Engine.h"

#include <shlwapi.h>
#include <io.h>
#include <fcntl.h>

#include "Offset.h"
#include "ScriptEngine.h"
#include "Helpers.h"
#include "D2Handlers.h"
#include "Console.h"
#include "CommandLine.h"
#include "Core.h"
#include "D2Handlers.h"
#include "Control.h"

#include <MinHook.h>
#include <mutex>

Engine::Engine() : m_hModule(nullptr), m_pScriptEngine(std::make_unique<ScriptEngine>()) {
  m_instance = this;
  m_fnWndProc = nullptr;
  m_fnCreateWindow = nullptr;
  m_fnGameDraw = nullptr;
  m_fnGameDrawMenu = nullptr;
}

Engine::~Engine() {
}

bool Engine::Initialize(HMODULE hModule) {
  m_hModule = hModule;

  // start of old DllMain DLL_PROCESS_ATTACH
  GetModuleFileNameA(hModule, Vars.szPath, MAX_PATH);
  PathRemoveFileSpecA(Vars.szPath);
  strcat_s(Vars.szPath, MAX_PATH, "\\");

  sprintf_s(Vars.szLogPath, _countof(Vars.szLogPath), "%slogs\\", Vars.szPath);
  CreateDirectoryA(Vars.szLogPath, NULL);
  InitCommandLine();
  InitSettings();
  Vars.bUseRawCDKey = FALSE;

  CommandLineParser cmdline(Vars.szCommandLine);
  for (const auto& [arg, value] : cmdline.Args()) {
    if (arg == "-console") {
      AllocConsole();
      SetConsoleOutputCP(CP_UTF8);
      FILE* fDummy;
      freopen_s(&fDummy, "CONIN$", "r", stdin);
      freopen_s(&fDummy, "CONOUT$", "w", stderr);
      freopen_s(&fDummy, "CONOUT$", "w", stdout);
    } else if (arg == "-title") {
      strncat_s(Vars.szTitle, value.c_str(), value.length());
    } else if (arg == "-sleepy") {
      Vars.bSleepy = TRUE;
    } else if (arg == "-cachefix") {
      Vars.bCacheFix = TRUE;
    } else if (arg == "-multi") {
      Vars.bMulti = TRUE;
    } else if (arg == "-ftj") {
      Vars.bReduceFTJ = TRUE;
    } else if (arg == "-d2c") {
      Vars.bUseRawCDKey = TRUE;
      strncat_s(Vars.szClassic, value.c_str(), value.length());
    } else if (arg == "-d2x") {
      strncat_s(Vars.szLod, value.c_str(), value.length());
    } else if (arg == "-handle") {
      Vars.hHandle = (HWND)atoi(value.c_str());
    }
  }

  SetUnhandledExceptionFilter(ExceptionHandler);

  MH_Initialize();
  // win32 hooks
  // stop game from replacing our exception handler
  MH_CreateHookApi(L"kernel32.dll", "SetUnhandledExceptionFilter", HandleSetUnhandledExceptionFilter, nullptr);

  // game hooks
  size_t base = reinterpret_cast<size_t>(GetModuleHandle(nullptr));
  MH_CreateHook(reinterpret_cast<void*>(base + 0xF52E0), HandleCreateWindow, (void**)&m_fnCreateWindow);
  MH_CreateHook(reinterpret_cast<void*>(base + 0x53B30), HandleGameDraw, (void**)&m_fnGameDraw);
  MH_CreateHook(reinterpret_cast<void*>(base + 0xF9870), HandleGameDrawMenu, (void**)&m_fnGameDrawMenu);
  MH_EnableHook(MH_ALL_HOOKS);

  // start of old Startup function
  InitializeCriticalSection(&Vars.cEventSection);
  InitializeCriticalSection(&Vars.cPrintSection);
  InitializeCriticalSection(&Vars.cBoxHookSection);
  InitializeCriticalSection(&Vars.cFrameHookSection);
  InitializeCriticalSection(&Vars.cLineHookSection);
  InitializeCriticalSection(&Vars.cImageHookSection);
  InitializeCriticalSection(&Vars.cTextHookSection);
  InitializeCriticalSection(&Vars.cFlushCacheSection);
  InitializeCriticalSection(&Vars.cConsoleSection);
  InitializeCriticalSection(&Vars.cGameLoopSection);
  InitializeCriticalSection(&Vars.cFileSection);

  Vars.bChangedAct = FALSE;
  Vars.bGameLoopEntered = FALSE;
  Vars.SectionCount = 0;
  Vars.bIgnoreKeys = FALSE;
  Vars.bIgnoreMouse = FALSE;

  Genhook::Initialize();
  InitOffsets();
  InstallPatches();
  InstallConditional();

  return TRUE;
}

void Engine::Shutdown() {
  // TODO(ejt): while proper shutdown is not strictly required for D2BS to function it's good practice to cleanup as best we can.
  // During restructuring it is not a priority so revisit this in the future.

  MH_DisableHook(MH_ALL_HOOKS);
  MH_RemoveHook(HandleGameDrawMenu);
  MH_RemoveHook(HandleGameDraw);
  MH_RemoveHook(HandleCreateWindow);
  MH_RemoveHook(HandleSetUnhandledExceptionFilter);
  MH_Uninitialize();

  RemovePatches();
  Genhook::Destroy();

  DeleteCriticalSection(&Vars.cPrintSection);
  DeleteCriticalSection(&Vars.cBoxHookSection);
  DeleteCriticalSection(&Vars.cFrameHookSection);
  DeleteCriticalSection(&Vars.cLineHookSection);
  DeleteCriticalSection(&Vars.cImageHookSection);
  DeleteCriticalSection(&Vars.cTextHookSection);
  DeleteCriticalSection(&Vars.cFlushCacheSection);
  DeleteCriticalSection(&Vars.cConsoleSection);
  DeleteCriticalSection(&Vars.cGameLoopSection);
  DeleteCriticalSection(&Vars.cFileSection);

  Log("D2BS Shutdown complete.");
}

void Engine::OnUpdate() {
  static std::once_flag of;
  std::call_once(of, []() {
    if (!sScriptEngine->Initialize()) {
      strcpy_s(Vars.szPath, MAX_PATH, "common");
      Log("D2BS Engine startup failed. %s", Vars.szCommandLine);
      Print("ÿc2D2BSÿc0 :: Engine startup failed!");
      exit(-1);
    }

    if (ClientState() == ClientStateMenu && Vars.bStartAtMenu)
      clickControl(*D2WIN_FirstControl);

    *D2CLIENT_Lang = D2CLIENT_GetGameLanguageCode();

    // TODO(ejt): use these in Initialize?
    CommandLineParser cmdline(Vars.szCommandLine);
    for (const auto& [arg, value] : cmdline.Args()) {
      if (arg == "-mpq") {
        LoadMPQ(value.c_str());
      } else if (arg == "-profile") {
        if (SwitchToProfile(value.c_str()))
          Print("ÿc2D2BSÿc0 :: Switched to profile %s", value.c_str());
        else
          Print("ÿc2D2BSÿc0 :: Profile %s not found", value.c_str());
      }
    }

    Log("D2BS Engine startup complete. %s", D2BS_VERSION);
    Print("ÿc2D2BSÿc0 :: Engine startup complete!");
  });

  static bool beginStarter = true;
  static bool bInGame = false;

  switch (ClientState()) {
    case ClientStateInGame: {
      if (bInGame) {
        if ((Vars.dwMaxGameTime && Vars.dwGameTime && (GetTickCount() - Vars.dwGameTime) > Vars.dwMaxGameTime) ||
            (!D2COMMON_IsTownByLevelNo(GetPlayerArea()) && (Vars.nChickenHP && Vars.nChickenHP >= D2CLIENT_GetPlayerUnit()->GetHealth()) ||
             (Vars.nChickenMP && Vars.nChickenMP >= D2CLIENT_GetPlayerUnit()->GetMana())))
          D2CLIENT_ExitGame();
      } else {
        Vars.dwGameTime = GetTickCount();

        D2CLIENT_InitInventory();
        sScriptEngine->ForEachScript(
            [](Script* script, void*, uint32_t) {
              script->UpdatePlayerGid();
              return true;
            },
            NULL, 0);
        sScriptEngine->UpdateConsole();
        Vars.bQuitting = false;
        OnGameEntered();

        bInGame = true;
      }
      break;
    }
    case ClientStateMenu: {
      // this will 100% cause a deadlock if bUseProfileScript is true
      while (Vars.bUseProfileScript) {
        Sleep(100);
      }
      OnMenuEntered(beginStarter);
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
}

void Engine::OnGameEntered() {
  if (!Vars.bUseProfileScript) {
    const char* starter = GetStarterScriptName();
    if (starter != NULL) {
      Print("ÿc2D2BSÿc0 :: Starting %s", starter);
      if (StartScript(starter, GetStarterScriptState()))
        Print("ÿc2D2BSÿc0 :: %s running.", starter);
      else
        Print("ÿc2D2BSÿc0 :: Failed to start %s!", starter);
    }
  }
}

void Engine::OnMenuEntered(bool beginStarter) {
  if (beginStarter && !Vars.bUseProfileScript) {
    const char* starter = GetStarterScriptName();
    if (starter != NULL) {
      Print("ÿc2D2BSÿc0 :: Starting %s", starter);
      if (StartScript(starter, GetStarterScriptState()))
        Print("ÿc2D2BSÿc0 :: %s running.", starter);
      else
        Print("ÿc2D2BSÿc0 :: Failed to start %s!", starter);
    }
  }
}

void Engine::FlushPrint() {
  if (!TryEnterCriticalSection(&Vars.cPrintSection))
    return;

  if (Vars.qPrintBuffer.empty()) {
    LeaveCriticalSection(&Vars.cPrintSection);
    return;
  }

  std::queue<std::string> clean;
  std::swap(Vars.qPrintBuffer, clean);
  LeaveCriticalSection(&Vars.cPrintSection);

  while (!clean.empty()) {
    std::string str = clean.front();

    // Break into lines through \n.
    std::list<std::string> lines;
    std::string temp;
    std::stringstream ss(str);

    if (Vars.bUseGamePrint && ClientState() == ClientStateInGame) {
      while (getline(ss, temp)) {
        SplitLines(temp, Console::MaxWidth() - 100, ' ', lines);
        Console::AddLine(temp);
      }

      // Convert and send every line.
      for (std::list<std::string>::iterator it = lines.begin(); it != lines.end(); ++it) {
        std::wstring wstr = AnsiToWide(*it);
        D2CLIENT_PrintGameString(wstr.c_str(), 0);
      }
      /*} else if (Vars.bUseGamePrint && ClientState() == ClientStateMenu && findControl(4, (const wchar_t*)NULL, -1, 28, 410, 354, 298)) {
          while (getline(ss, temp))
              SplitLines(temp, Console::MaxWidth() - 100, ' ', lines);
              // TODO: Double check this function, make sure it is working as intended.
              for (list<string>::iterator it = lines.begin(); it != lines.end(); ++it)
                  D2MULTI_PrintChannelText((char*)it->c_str(), 0);*/
    } else {
      while (getline(ss, temp)) Console::AddLine(temp);
    }

    clean.pop();
  }
}

LPTOP_LEVEL_EXCEPTION_FILTER __stdcall Engine::HandleSetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter) {
  return lpTopLevelExceptionFilter;
}

LRESULT __stdcall Engine::HandleWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    // BUG(ejt): weird behavior when typing into console, adds numbers after some characters
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
    //case WM_CHAR: 
    {
      // TODO(ejt): scuffed solution to ignore keyboard events from scripts
      if (Vars.bIgnoreKeys) {
        break;
      }
      WORD vkCode = LOWORD(wParam);
      WORD repeatCount = LOWORD(lParam);

      WORD keyFlags = HIWORD(lParam);
      bool altState = (keyFlags & KF_ALTDOWN) == KF_ALTDOWN;
      bool isRepeat = ((keyFlags & KF_REPEAT) == KF_REPEAT) && repeatCount > 1;
      bool isDown = (keyFlags & KF_UP) != KF_UP;
      bool isUp = !isDown;

      bool gameState = ClientState() == ClientStateInGame;
      bool chatBoxOpen = gameState ? !!D2CLIENT_GetUIState(UI_CHAT_CONSOLE) : false;
      bool escMenuOpen = gameState ? !!D2CLIENT_GetUIState(UI_ESCMENU_MAIN) : false;

      if (altState && wParam == VK_F4)
        break;

      if (Vars.bBlockKeys)
        return 0;

      if (vkCode == VK_HOME && !(chatBoxOpen || escMenuOpen)) {
        if (isDown && !isRepeat) {
          if (!altState)
            Console::ToggleBuffer();
          else
            Console::TogglePrompt();

          break;
        }
      } else if (vkCode == VK_ESCAPE && Console::IsVisible()) {
        if (isDown && !isRepeat) {
          Console::Hide();
          return 0;
        }
        break;
      } else if (Console::IsEnabled()) {
        BYTE layout[256] = {0};
        WORD out[2] = {0};
        switch (vkCode) {
          case VK_TAB:
            if (isUp)
              for (int i = 0; i < 5; i++) Console::AddKey(' ');
            break;
          case VK_RETURN:
            if (isUp && !isRepeat && !escMenuOpen) {
              Console::ExecuteCommand();
            }
            break;
          case VK_BACK:
            if (isDown)
              Console::RemoveLastKey();
            break;
          case VK_UP:
            if (isUp && !isRepeat)
              Console::PrevCommand();
            break;
          case VK_DOWN:
            if (isUp && !isRepeat)
              Console::NextCommand();
            break;
          case VK_NEXT:
            if (isDown)
              Console::ScrollDown();
            break;
          case VK_PRIOR:
            if (isDown)
              Console::ScrollUp();
            break;
          case VK_MENU:  // alt
            // Send the alt to the scripts to fix sticky alt. There may be a better way.
            KeyDownUpEvent(vkCode, isUp);
            return m_fnWndProc(hWnd, uMsg, vkCode, lParam);
            break;
          default:
            if (isDown) {
              if (GetKeyboardState(layout) && ToAscii(vkCode, (lParam & 0xFF0000), layout, out, 0) != 0) {
                for (int i = 0; i < repeatCount; i++) {
                  Console::AddKey(out[0]);
                }
              }
            }
            break;
        }
        return 0;
      } else if (!isRepeat && !(chatBoxOpen || escMenuOpen))
        if (KeyDownUpEvent(wParam, isUp))
          return 0;
    } break;

    // TODO(ejt): revisit mouse events for cleanup
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MOUSEMOVE: {
      // TODO(ejt): scuffed solution to ignore mouse events from scripts
      if (Vars.bIgnoreMouse) {
        break;
      }
      POINT pt = {static_cast<LONG>(LOWORD(lParam)), static_cast<LONG>(HIWORD(lParam))};
      // filter out clicks on the window border
      if ((pt.x < 0 || pt.y < 0))
        break;

      Vars.pMouseCoords = pt;
      if (Vars.bBlockMouse)
        return 0;

      bool clicked = false;

      HookClickHelper helper = {-1, {pt.x, pt.y}};
      switch (uMsg) {
        case WM_LBUTTONDOWN:
          MouseClickEvent(0, pt, false);
          helper.button = 0;
          if (Genhook::ForEachVisibleHook(ClickHook, &helper, 1))
            clicked = true;
          break;
        case WM_LBUTTONUP:
          MouseClickEvent(0, pt, true);
          break;
        case WM_RBUTTONDOWN:
          MouseClickEvent(1, pt, false);
          helper.button = 1;
          if (Genhook::ForEachVisibleHook(ClickHook, &helper, 1))
            clicked = true;
          break;
        case WM_RBUTTONUP:
          MouseClickEvent(1, pt, true);
          break;
        case WM_MBUTTONDOWN:
          MouseClickEvent(2, pt, false);
          helper.button = 2;
          if (Genhook::ForEachVisibleHook(ClickHook, &helper, 1))
            clicked = true;
          break;
        case WM_MBUTTONUP:
          MouseClickEvent(2, pt, true);
          break;
        case WM_MOUSEMOVE:
          // would be nice to enable these events but they bog down too much
          MouseMoveEvent(pt);
          // Genhook::ForEachVisibleHook(HoverHook, &helper, 1);
          break;
      }

      if (clicked) {
        return 0;
      }
    } break;

    case WM_COPYDATA: {
      COPYDATASTRUCT* pCopy = (COPYDATASTRUCT*)lParam;

      if (pCopy) {
        const char* lpszData = static_cast<const char*>(pCopy->lpData);
        if (pCopy->dwData == 0x1337) {  // 0x1337 = Execute Script
          sScriptEngine->RunCommand(lpszData);
        } else if (pCopy->dwData == 0x31337) {  // 0x31337 = Set Profile
          if (SwitchToProfile(lpszData)) {
            Print("ÿc2D2BSÿc0 :: Switched to profile %s", lpszData);
          } else {
            Print("ÿc2D2BSÿc0 :: Profile %s not found", lpszData);
          }
        } else {
          CopyDataEvent(pCopy->dwData, lpszData);
        }
      }
      return TRUE;
    }
  }
  return m_fnWndProc(hWnd, uMsg, wParam, lParam);
}

int __stdcall Engine::HandleCreateWindow(HINSTANCE hInstance, WNDPROC lpWndProc, int a3, int a4) {
  m_fnWndProc = lpWndProc;
  return m_fnCreateWindow(hInstance, HandleWndProc, a3, a4);
}

void Engine::HandleGameDraw() {
  if (Vars.bGameLoopEntered)
    LeaveCriticalSection(&Vars.cGameLoopSection);
  else {
    Vars.bGameLoopEntered = true;
    Vars.dwGameThreadId = GetCurrentThreadId();
  }
  if (Vars.SectionCount)
    Sleep(5);

  EnterCriticalSection(&Vars.cGameLoopSection);

  m_instance->OnUpdate();

  if (ClientState() == ClientStateInGame) {
    m_instance->FlushPrint();
    Genhook::DrawAll(IG);
    DrawLogo();
    Console::Draw();
  }
  if (Vars.bTakeScreenshot) {
    Vars.bTakeScreenshot = false;
    D2WIN_TakeScreenshot();
  }
  if (Vars.SectionCount) {
    if (Vars.bGameLoopEntered)
      LeaveCriticalSection(&Vars.cGameLoopSection);
    else
      Vars.bGameLoopEntered = true;
    Sleep(0);
    EnterCriticalSection(&Vars.cGameLoopSection);
  } else
    Sleep(10);
  m_fnGameDraw();
}

void Engine::HandleGameDrawMenu() {
  m_fnGameDrawMenu();
  m_instance->OnUpdate();

  if (ClientState() == ClientStateMenu) {
    m_instance->FlushPrint();
    Genhook::DrawAll(OOG);
    DrawLogo();
    Console::Draw();
  }
  if (Vars.bTakeScreenshot) {
    Vars.bTakeScreenshot = false;
    D2WIN_TakeScreenshot();
  }
  if (Vars.SectionCount) {
    if (Vars.bGameLoopEntered)
      LeaveCriticalSection(&Vars.cGameLoopSection);
    else
      Vars.bGameLoopEntered = true;
    Sleep(0);
    EnterCriticalSection(&Vars.cGameLoopSection);
  } else
    Sleep(10);
}
