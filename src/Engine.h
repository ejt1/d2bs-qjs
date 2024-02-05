#pragma once

#include "ScreenHook.h"
#include "Globals.h"

#define D2BS_VERSION L"2.0.0"

class Engine final {
 public:
  Engine();
  virtual ~Engine();

  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;

  bool Initialize(HMODULE hModule);
  void Shutdown();

 private:
  // only for use by hooks!
  inline static Engine* m_instance;

  void OnUpdate();
  void OnGameEntered();
  void OnMenuEntered(bool beginStarter);

  void FlushPrint();

  // hooks
  static LPTOP_LEVEL_EXCEPTION_FILTER WINAPI
  HandleSetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter);

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
  std::unique_ptr<ScriptEngine> m_pScriptEngine;
};