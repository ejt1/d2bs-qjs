#pragma once

#include "ScreenHook.h"

#include <Windows.h>
#include <vector>
#include <queue>

#define D2BS_VERSION L"1.6.4U"

// struct Variables;

#define ArraySize(x) (sizeof((x)) / sizeof((x)[0]))

#define PRIVATE_UNIT 1
#define PRIVATE_ITEM 3

struct Private {
  DWORD dwPrivateType;
};

struct Module {
  union {
    HMODULE hModule;
    DWORD dwBaseAddress;
  };
  DWORD _1;
  wchar_t szPath[MAX_PATH];
};

struct Variables {
  int nChickenHP;
  int nChickenMP;
  DWORD dwInjectTime;
  DWORD dwGameTime;
  BOOL bDontCatchNextMsg;
  BOOL bClickAction;
  BOOL bNeedShutdown;
  BOOL bUseGamePrint;
  BOOL bShutdownFromDllMain;
  BOOL bChangedAct;
  BOOL bGameLoopEntered;
  DWORD dwGameThreadId;
  DWORD dwLocale;

  DWORD dwMaxGameTime;
  DWORD dwGameTimeout;
  BOOL bTakeScreenshot;
  BOOL bQuitOnError;
  BOOL bQuitOnHostile;
  BOOL bStartAtMenu;
  BOOL bActive;
  BOOL bIgnoreKeys;
  BOOL bBlockKeys;
  BOOL bIgnoreMouse;
  BOOL bBlockMouse;
  BOOL bDisableCache;
  BOOL bUseProfileScript;
  BOOL bLoadedWithCGuard;
  BOOL bLogConsole;
  BOOL bEnableUnsupported;
  BOOL bForwardMessageBox;
  BOOL bUseRawCDKey;
  BOOL bQuitting;
  BOOL bCacheFix;
  BOOL bMulti;
  BOOL bSleepy;
  BOOL bReduceFTJ;
  int dwMemUsage;
  int dwConsoleFont;
  HANDLE eventSignal;
  Module* pModule;
  HMODULE hModule;
  HWND hHandle;

  wchar_t szPath[_MAX_PATH];
  wchar_t szLogPath[_MAX_PATH];
  wchar_t szScriptPath[_MAX_PATH];
  wchar_t szProfile[256];
  wchar_t szStarter[_MAX_FNAME];
  wchar_t szConsole[_MAX_FNAME];
  wchar_t szDefault[_MAX_FNAME];
  char szHosts[256];
  char szClassic[30];
  char szLod[30];
  wchar_t szTitle[256];
  wchar_t szCommandLine[256];

  long SectionCount;

  std::queue<std::wstring> qPrintBuffer;
  std::map<unsigned __int32, CellFile*> mCachedCellFiles;
  std::vector<std::pair<DWORD, DWORD>> vUnitList;
  // std::list<Event*> EventList;
  CRITICAL_SECTION cEventSection;
  CRITICAL_SECTION cRoomSection;
  CRITICAL_SECTION cMiscSection;
  CRITICAL_SECTION cScreenhookSection;
  CRITICAL_SECTION cPrintSection;
  CRITICAL_SECTION cTextHookSection;
  CRITICAL_SECTION cImageHookSection;
  CRITICAL_SECTION cBoxHookSection;
  CRITICAL_SECTION cFrameHookSection;
  CRITICAL_SECTION cLineHookSection;
  CRITICAL_SECTION cFlushCacheSection;
  CRITICAL_SECTION cConsoleSection;
  CRITICAL_SECTION cGameLoopSection;
  CRITICAL_SECTION cUnitListSection;
  CRITICAL_SECTION cFileSection;

  DWORD dwSelectedUnitId;
  DWORD dwSelectedUnitType;
  POINT pMouseCoords;
};

extern Variables Vars;

class Engine final {
 public:
  Engine();
  virtual ~Engine();

  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;

  bool Initialize(HMODULE hModule, LPVOID lpReserved);
  void Shutdown();

 private:
  // only for use by hooks!
  inline static Engine* m_instance;

  static DWORD WINAPI EngineThread(LPVOID lpThreadParameter);

  void OnUpdate();
  void OnGameEntered();
  void OnMenuEntered(bool beginStarter);

  // hooks
  inline static WNDPROC m_fnWndProc;
  static LRESULT WINAPI HandleWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  typedef int(WINAPI* CreateWindow_t)(HINSTANCE, WNDPROC, int a3, int a4);
  inline static CreateWindow_t m_fnCreateWindow;
  static int WINAPI HandleCreateWindow(HINSTANCE hInstance, WNDPROC lpWndProc, int a3, int a4);

  typedef void (*GameDraw_t)();
  inline static GameDraw_t m_fnGameDraw;
  static void HandleGameDraw();

  typedef void (*GameDrawMenu_t)();
  inline static GameDrawMenu_t m_fnGameDrawMenu;
  static void HandleGameDrawMenu();

  HMODULE m_hModule;
};