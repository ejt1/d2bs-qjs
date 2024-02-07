#pragma once

#include <windows.h>
#include <string>
#include <map>
#include <list>

#include "js32.h"
#include "JSGlobalClasses.h"
#include "JSUnit.h"
#include "Events.h"

#include <uv.h>

typedef std::map<std::string, bool> IncludeList;
typedef std::map<std::string, FunctionList> FunctionMap;

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

  Script(const char* file, ScriptMode mode/*, uint32_t argc = 0, JSAutoStructuredCloneBuffer** argv = NULL*/);
  ~Script();

 public:
  Script(const Script&) = delete;
  Script& operator=(const Script&) = delete;

  bool Start();
  void Stop(bool force = false);
  void Run();

  void Join(void);
  void Pause(void);
  void Resume(void);
  bool IsUninitialized();
  bool IsRunning(void);
  bool IsAborted(void);
  void RunCommand(const char* command);

  inline void TriggerOperationCallback(void) {
    //if (m_hasActiveCX)
    //  JS_TriggerOperationCallback(m_runtime);
  }

  inline void SetPauseState(bool reallyPaused) {
    m_isReallyPaused = reallyPaused;
  }

  inline const char* GetFilename(void) {
    return m_fileName.c_str();
  }

  const char* GetShortFilename(void);

  inline JSContext* GetContext(void) {
    return m_context;
  }

  inline JSRuntime* GetRuntime(void) {
    return m_runtime;
  }

  inline ScriptMode GetMode(void) {
    return m_scriptMode;
  }

  DWORD GetThreadId(void);

  // UGLY HACK to fix up the player gid on game join for cached scripts/oog scripts
  void UpdatePlayerGid(void);

  bool IsIncluded(const char* file);
  bool Include(const char* file);

  size_t GetListenerCount(const char* evtName, JSValue evtFunc = JS_UNDEFINED);
  void AddEventListener(const char* evtName, JSValue evtFunc);
  void RemoveEventListener(const char* evtName, JSValue evtFunc);
  void RemoveAllListeners(const char* evtName);
  void RemoveAllEventListeners();
  void DispatchEvent(Event* evt);

  // blocks the executing thread for X milliseconds, keeping the event loop running during this time
  void BlockThread(DWORD delay);
  bool HandleEvent(Event* evt, bool clearList);

 private:
  bool Initialize();
  void Cleanup();

  void RunMain();

  bool RunEventLoop();
  void PurgeEventList();
  void ExecuteEvent(char* evtName, int argc, const JSValue* argv, bool* block = nullptr);
  bool ProcessAllEvents();

  static int InterruptHandler(JSRuntime* rt, void* opaque);

  std::string m_fileName;
  ScriptMode m_scriptMode;
  std::atomic<ScriptState> m_scriptState;

  JSRuntime* m_runtime;
  JSContext* m_context;
  JSValue m_globalObject;
  JSValue m_script;
  JSUnit* m_me;

  // wtf is this trying to do anyway, why not just check m_context or m_runtime?
  bool m_hasActiveCX;  // hack to get away from JS_IsRunning

  DWORD m_threadId;
  HANDLE m_threadHandle;

  FunctionMap m_functions;
  HANDLE m_eventSignal;
  std::list<Event*> m_EventList;

  bool m_isPaused, m_isReallyPaused;

  IncludeList m_includes, m_inProgress;

  CRITICAL_SECTION m_lock;
};

struct RUNCOMMANDSTRUCT {
  Script* script;
  const wchar_t* command;
};

DWORD WINAPI ScriptThread(LPVOID lpThreadParameter);

//JSBool contextCallback(JSContext* ctx, uint32_t contextOp);
//void reportError(JSContext* cx, const char* message, JSErrorReport* report);
