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
enum ScriptState { InGame, OutOfGame, Command };

class Script;

typedef std::map<std::wstring, bool> IncludeList;
typedef std::list<AutoRoot*> FunctionList;
typedef std::map<std::string, FunctionList> FunctionMap;
typedef std::list<Script*> ScriptList;

struct Event {
  Event() : count(0){};
  Script* owner;
  JSObject* object;
  FunctionList functions;
  JSAutoStructuredCloneBuffer** argv;
  uint argc;
  char* name;
  void* arg1;
  void* arg2;
  void* arg3;
  void* arg4;
  void* arg5;
  volatile long count;
  inline void threadFinished() {
    // clean up after both threads are done with the event
    char* evtName = (char*)name;
    InterlockedIncrement(&count);
    if (count > 1) {
      Event* evt = this;

      if (strcmp(evtName, "itemaction") == 0) {
        delete arg1;
        free(arg2);
        delete arg3;
        delete arg4;
      }
      if (strcmp(evtName, "gameevent") == 0) {
        delete evt->arg1;
        delete evt->arg2;
        delete evt->arg3;
        free(evt->arg4);
        free(evt->arg5);
      }
      if (strcmp(evtName, "copydata") == 0) {
        delete evt->arg1;
        free(evt->arg2);
      }
      if (strcmp(evtName, "chatmsg") == 0 || strcmp(evtName, "chatinput") == 0 || strcmp(evtName, "whispermsg") == 0 || strcmp(evtName, "chatmsgblocker") == 0 ||
          strcmp(evtName, "chatinputblocker") == 0 || strcmp(evtName, "whispermsgblocker") == 0) {
        free(evt->arg1);
        free(evt->arg2);
        delete evt->arg4;
      }
      if (strcmp(evtName, "mousemove") == 0 || strcmp(evtName, "ScreenHookHover") == 0) {
        delete evt->arg1;
        delete evt->arg2;
      }
      if (strcmp(evtName, "mouseclick") == 0) {
        delete evt->arg1;
        delete evt->arg2;
        delete evt->arg3;
        delete evt->arg4;
      }
      if (strcmp(evtName, "keyup") == 0 || strcmp(evtName, "keydownblocker") == 0 || strcmp(evtName, "keydown") == 0 || strcmp(evtName, "memana") == 0 ||
          strcmp(evtName, "melife") == 0 || strcmp(evtName, "playerassign") == 0) {
        delete evt->arg1;
        delete evt->arg4;
      }
      if (strcmp(evtName, "ScreenHookClick") == 0) {
        delete evt->arg1;
        delete evt->arg2;
        delete evt->arg3;
        delete evt->arg4;
      }
      if (strcmp(evtName, "Command") == 0) {
        // cleaned up in ExecScriptEvent
      }
      if (strcmp(evtName, "scriptmsg") == 0) {
        delete evt->arg1;
      }
      if (strcmp(evtName, "gamepacket") == 0 || strcmp(evtName, "gamepacketsent") == 0 || strcmp(evtName, "realmpacket") == 0) {
        delete[] evt->arg1;
        delete evt->arg2;
        delete evt->arg4;
      }

      free(evt->name);
      delete evt;
      Event::~Event();
    }
  };
};

class Script {
  friend class ScriptEngine;

  Script(const wchar_t* file, ScriptState state, uint argc = 0, JSAutoStructuredCloneBuffer** argv = NULL);
  ~Script();

 public:
  Script(const Script&) = delete;
  Script& operator=(const Script&) = delete;

  void Run(void);
  void Join(void);
  void Pause(void);
  void Resume(void);
  void Stop(bool force = false, bool reallyForce = false);
  bool IsPaused(void);
  bool BeginThread(LPTHREAD_START_ROUTINE ThreadFunc);
  void RunCommand(const wchar_t* command);

  inline void TriggerOperationCallback(void) {
    if (m_hasActiveCX)
      JS_TriggerOperationCallback(m_runtime);
  }

  inline void SetPauseState(bool reallyPaused) {
    m_isReallyPaused = reallyPaused;
  }
  inline bool IsReallyPaused(void) {
    return m_isReallyPaused;
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
  inline JSObject* GetGlobalObject(void) {
    return m_globalObject;
  }
  inline JSObject* GetScriptObject(void) {
    return m_scriptObject;
  }
  inline ScriptState GetState(void) {
    return m_scriptState;
  }
  inline FunctionMap& functions() {
    return m_functions;
  }

  inline DWORD threadId() {
    return m_threadId;
  }

  int GetExecutionCount(void);
  DWORD GetThreadId(void);

  // UGLY HACK to fix up the player gid on game join for cached scripts/oog scripts
  void UpdatePlayerGid(void);
  // Hack. Include from console needs to run on the RunCommandThread / cx.
  //		 a better solution may be to keep a list of threadId / cx and have a GetCurrentThreadCX()
  inline void SetContext(JSContext* cx) {
    m_context = cx;
  }
  bool IsRunning(void);
  bool IsAborted(void);
  void Lock() {
    EnterCriticalSection(&m_lock);
  }  // needed for events walking function list
  void Unlock() {
    LeaveCriticalSection(&m_lock);
  }
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
  void ProcessAllEvents();
  void ProcessOneEvent();

  void OnDestroyContext(JSContext* ctx);

 private:
  std::wstring m_fileName;
  int m_execCount;
  ScriptState m_scriptState;
  JSContext* m_context;
  JSScript* m_script;
  JSRuntime* m_runtime;
  myUnit* m_me;
  uint m_argc;
  JSAutoStructuredCloneBuffer** m_argv;

  DWORD m_threadId;
  FunctionMap m_functions;
  DWORD m_LastGC;
  bool m_hasActiveCX;  // hack to get away from JS_IsRunning
  HANDLE m_eventSignal;
  std::list<Event*> m_EventList;

  JSObject *m_globalObject, *m_scriptObject;
  bool m_isLocked, m_isPaused, m_isReallyPaused, m_isAborted;

  IncludeList m_includes, m_inProgress;

  HANDLE m_threadHandle;

  CRITICAL_SECTION m_lock;
};

struct RUNCOMMANDSTRUCT {
  Script* script;
  const wchar_t* command;
};

DWORD WINAPI ScriptThread(void* data);

JSBool operationCallback(JSContext* cx);
JSBool contextCallback(JSContext* ctx, uint contextOp);
void reportError(JSContext* cx, const char* message, JSErrorReport* report);
bool ExecScriptEvent(Event* evt, bool clearList);
