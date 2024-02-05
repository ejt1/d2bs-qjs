#pragma once

#include <Windows.h>
#include <vector>
#include <queue>
#include <string>
#include <map>

#include "Game/D2Gfx.h"

// globals are bad // ejt
struct Variables {
  int nChickenHP;
  int nChickenMP;
  DWORD dwGameTime;
  BOOL bDontCatchNextMsg;
  BOOL bClickAction;
  BOOL bUseGamePrint;
  BOOL bChangedAct; // TODO(ejt): check if this and its hooks are necessary
  BOOL bGameLoopEntered; // TODO(ejt): get rid of this, find better way to run in main thread
  DWORD dwGameThreadId;

  DWORD dwMaxGameTime;
  DWORD dwGameTimeout;
  BOOL bTakeScreenshot;
  BOOL bQuitOnError;
  BOOL bQuitOnHostile;
  BOOL bStartAtMenu;
  BOOL bIgnoreKeys;
  BOOL bBlockKeys;
  BOOL bIgnoreMouse;
  BOOL bBlockMouse;
  BOOL bDisableCache;
  BOOL bUseProfileScript;
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
  HWND hHandle;

  char szPath[_MAX_PATH];
  char szLogPath[_MAX_PATH];
  char szScriptPath[_MAX_PATH];
  char szProfile[256];
  char szStarter[_MAX_FNAME];
  char szConsole[_MAX_FNAME];
  char szDefault[_MAX_FNAME];
  char szHosts[256];
  char szClassic[30];
  char szLod[30];
  char szTitle[256];
  char szCommandLine[256];

  long SectionCount;

  std::queue<std::wstring> qPrintBuffer;
  std::map<unsigned __int32, D2CellFileStrc*> mCachedCellFiles;
  std::vector<std::pair<DWORD, DWORD>> vUnitList;
  // std::list<Event*> EventList;
  CRITICAL_SECTION cEventSection;
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