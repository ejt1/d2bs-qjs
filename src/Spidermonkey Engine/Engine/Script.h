#pragma once

#include <windows.h>
#include <string>
#include <map>
#include <list>

#include "js32.h"
#include "JSGlobalClasses.h"
#include "AutoRoot.h"
#include "JSUnit.h"
#include "Events.h"

class Script;

typedef std::map<std::wstring, bool> IncludeList;
typedef std::list<AutoRoot*> FunctionList;
typedef std::map<std::string, FunctionList> FunctionMap;
typedef std::list<Script*> ScriptList;

struct Event {
  Event() : argv(nullptr), argc(0), arg1(nullptr), arg2(nullptr), arg3(nullptr), arg4(nullptr), arg5(nullptr){};
  FunctionList functions;
  JSAutoStructuredCloneBuffer** argv;
  uint argc;
  std::string name;
  void* arg1;
  void* arg2;
  void* arg3;
  void* arg4;
  void* arg5;
};

enum ScriptMode { kScriptModeGame, kScriptModeMenu, kScriptModeCommand };

enum ScriptState {
  kScriptStateUninitialized,
  kScriptStateStarted,
  kScriptStateRunning,
  kScriptStateRequestStop,
  kScriptStateStopped,
};

class Script {
  friend class ScriptEngine;

  Script(const wchar_t* file, ScriptMode mode, uint argc = 0, JSAutoStructuredCloneBuffer** argv = NULL);
  ~Script();

 public:
  Script(const Script&) = delete;
  Script& operator=(const Script&) = delete;

  bool Start();
  void Stop(bool force = false, bool reallyForce = false);
  void Run();

  void Join(void);
  void Pause(void);
  void Resume(void);
  bool IsRunning(void);
  bool IsAborted(void);
  void RunCommand(const wchar_t* command);

  inline void TriggerOperationCallback(void) {
    if (m_hasActiveCX)
      JS_TriggerOperationCallback(m_runtime);
  }

  inline void SetPauseState(bool reallyPaused) {
    m_isReallyPaused = reallyPaused;
  }

  inline const wchar_t* GetFilename(void) {
    return m_fileName.c_str();
  }
  const wchar_t* GetShortFilename(void);
  inline JSContext* GetContext(void) {
    return m_context;
  }
  inline JSRuntime* GetRuntime(void) {
    return m_runtime;
  }
  inline ScriptMode GetMode(void) {
    return m_scriptMode;
  }

  int GetExecutionCount(void);
  DWORD GetThreadId(void);

  // UGLY HACK to fix up the player gid on game join for cached scripts/oog scripts
  void UpdatePlayerGid(void);

  bool IsIncluded(const wchar_t* file);
  bool Include(const wchar_t* file);

  bool IsListenerRegistered(const char* evtName);
  void RegisterEvent(const char* evtName, jsval evtFunc);
  bool IsRegisteredEvent(const char* evtName, jsval evtFunc);
  void UnregisterEvent(const char* evtName, jsval evtFunc);
  void ClearEvent(const char* evtName);
  void ClearAllEvents(void);
  void FireEvent(Event*);

  void ClearEventList();
  // blocks the executing thread for X milliseconds, keeping the event loop running during this time
  void BlockThread(DWORD delay);
  void ExecuteEvent(char* evtName, int argc, const jsval* argv, bool* block = nullptr);
  void ExecuteEvent(char* evtName, const JS::AutoValueVector& args, bool* block = nullptr);

  void OnDestroyContext();

  // public because PacketEventCallback calls this directly
  bool HandleEvent(Event* evt, bool clearList);

 private:
  bool Initialize();
  void Cleanup();

  void RunMain();
  bool RunEventLoop();
  bool ProcessAllEvents();

  static JSBool InterruptHandler(JSContext* ctx);

  std::wstring m_fileName;
  int m_execCount;
  ScriptMode m_scriptMode;
  std::atomic<ScriptState> m_scriptState;
  JSRuntime* m_runtime;
  JSContext* m_context;
  JSScript* m_script;
  myUnit* m_me;
  uint m_argc;
  JSAutoStructuredCloneBuffer** m_argv;

  DWORD m_threadId;
  FunctionMap m_functions;
  DWORD m_LastGC;
  // wtf is this trying to do anyway, why not just check m_context or m_runtime?
  bool m_hasActiveCX;  // hack to get away from JS_IsRunning
  HANDLE m_eventSignal;
  std::list<Event*> m_EventList;

  JSObject* m_globalObject;
  bool m_isLocked, m_isPaused, m_isReallyPaused;

  IncludeList m_includes, m_inProgress;

  HANDLE m_threadHandle;

  CRITICAL_SECTION m_lock;
};

struct RUNCOMMANDSTRUCT {
  Script* script;
  const wchar_t* command;
};

DWORD WINAPI ScriptThread(void* data);

JSBool contextCallback(JSContext* ctx, uint contextOp);
void reportError(JSContext* cx, const char* message, JSErrorReport* report);
