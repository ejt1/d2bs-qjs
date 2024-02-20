#pragma once

#include <windows.h>
#include <string>
#include <map>
#include <list>

#include "js32.h"
#include "JSUnit.h"
#include "Events.h"

#include <uv.h>
#include <filesystem>

typedef std::map<std::string, bool> IncludeList;
typedef std::map<std::string, FunctionList> FunctionMap;
typedef std::map<std::filesystem::path, JS::PersistentRootedObject> ModuleRegistry;

enum ScriptMode { kScriptModeGame, kScriptModeMenu, kScriptModeCommand };

enum ScriptState {
  kScriptStateUninitialized,
  kScriptStateStarted,
  kScriptStateRunning,
  kScriptStateRequestStop,
  kScriptStateStopped,
};

struct ThreadState {
  class Script* script;
  uv_loop_t* loop;
  DWORD id;
  HANDLE handle;
  std::chrono::steady_clock::time_point lastSpinTime;

  std::filesystem::path moduleRoot;
  std::filesystem::path moduleEntry;
  ModuleRegistry moduleRegistry;
};

// IMPORTANT NOTE: Using modules implies non-blocking API!
extern bool kScriptUseModules;

class Script {
  friend class ScriptEngine;

  Script(const char* file, ScriptMode mode /*, uint32_t argc = 0, JSAutoStructuredCloneBuffer** argv = NULL*/);
  ~Script();

 public:
  Script(const Script&) = delete;
  Script& operator=(const Script&) = delete;

  bool Start();
  void Stop();
  void Run();

  void Join(void);
  void Pause(void);
  void Resume(void);
  bool IsUninitialized();
  bool IsRunning(void);
  bool IsAborted(void);
  void RunCommand(const char* command);

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

  inline ScriptMode GetMode(void) {
    return m_scriptMode;
  }

  ThreadState* GetThreadState();
  DWORD GetThreadId();

  // UGLY HACK to fix up the player gid on game join for cached scripts/oog scripts
  void UpdatePlayerGid(void);

  bool IsIncluded(const char* file);
  bool Include(const char* file);

  size_t GetListenerCount(const char* evtName, JS::HandleValue evtFunc = JS::UndefinedHandleValue);
  void AddEventListener(const char* evtName, JS::HandleValue evtFunc);
  void RemoveEventListener(const char* evtName, JS::HandleValue evtFunc);
  void RemoveAllListeners(const char* evtName);
  void RemoveAllEventListeners();
  void DispatchEvent(std::shared_ptr<Event> evt);

  // blocks the executing thread for X milliseconds, keeping the event loop running during this time
  bool BlockThread(DWORD delay);
  bool HandleEvent(std::shared_ptr<Event> evt, bool clearList);

 private:
  bool Initialize();
  void Cleanup();

  void RunMain();

  bool RunEventLoop();
  void PurgeEventList();
  void ExecuteEvent(char* evtName, const JS::HandleValueArray& argv, bool* block = nullptr);
  bool ProcessAllEvents();

  bool SetupModuleResolveHook(JSContext* ctx);
  JSObject* EvaluateModule(JSContext* ctx, const std::filesystem::path& entry);

  // static bool InterruptHandler(JSContext* ctx);

  std::string m_fileName;
  ScriptMode m_scriptMode;
  std::atomic<ScriptState> m_scriptState;

  ThreadState m_threadState;
  uv_loop_t m_loop;
  JSContext* m_context;
  // TODO(ejt): is there a reason to keep a reference to global?
  JS::PersistentRootedObject m_globalObject;
  // TODO(ejt): is there a reason to keep a reference to script?
  JS::PersistentRootedScript m_script;
  UnitWrap::UnitData* m_me;

  FunctionMap m_functions;
  HANDLE m_eventSignal;
  std::list<std::shared_ptr<Event>> m_EventList;

  std::atomic_bool m_isPaused, m_isReallyPaused;

  IncludeList m_includes, m_inProgress;

  CRITICAL_SECTION m_lock;
  bool m_debugCaptureStackFrames;
};

struct RUNCOMMANDSTRUCT {
  Script* script;
  const wchar_t* command;
};

DWORD WINAPI ScriptThread(LPVOID lpThreadParameter);

// JSBool contextCallback(JSContext* ctx, uint32_t contextOp);
// void reportError(JSContext* cx, const char* message, JSErrorReport* report);
