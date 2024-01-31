#pragma once

#include <list>
#include <map>
#include <string>

#include "js32.h"
#include "AutoRoot.h"
#include "Script.h"

typedef std::map<std::wstring, Script*> ScriptMap;

typedef bool(__fastcall* ScriptCallback)(Script*, void*, uint);

enum EngineState { Starting, Running, Paused, Stopping, Stopped };

class ScriptEngine {
  friend class Script;

  ScriptEngine();
  virtual ~ScriptEngine();

 public:
  static ScriptEngine* GetInstance();

  ScriptEngine(const ScriptEngine&) = delete;
  ScriptEngine& operator=(const ScriptEngine&) = delete;

  BOOL Startup(void);
  void Shutdown(void);
  EngineState GetState(void) {
    return m_state;
  }

  void FlushCache(void);

  Script* CompileFile(const wchar_t* file, ScriptState state, uint argc = 0, JSAutoStructuredCloneBuffer** argv = NULL, bool recompile = false);
  void RunCommand(const wchar_t* command);
  void DisposeScript(Script* script);

  void LockScriptList(char* loc);
  void UnLockScriptList(char* loc);

  ScriptMap& scripts() {
    return m_scripts;
  }
  bool ForEachScript(ScriptCallback callback, void* argv, uint argc);
  unsigned int GetCount(bool active = true, bool unexecuted = false);

  void StopAll(bool forceStop = false);
  void InitClass(JSContext* context, JSObject* globalObject, JSClass* classp, JSFunctionSpec* methods, JSPropertySpec* props, JSFunctionSpec* s_methods,
                 JSPropertySpec* s_props);
  void DefineConstant(JSContext* context, JSObject* globalObject, const char* name, int value);
  void UpdateConsole();
  int AddDelayedEvent(Event* evt, int freq);
  void RemoveDelayedEvent(int key);

 private:
  Script* m_console;
  EngineState m_state;
  std::list<Event*> m_DelayedExecList;
  int m_delayedExecKey;
  CRITICAL_SECTION m_scriptListLock;
  ScriptMap m_scripts;
  CRITICAL_SECTION m_lock;
};

#define sScriptEngine ScriptEngine::GetInstance()

// these ForEachScript helpers are exposed in case they can be of use somewhere
bool __fastcall StopIngameScript(Script* script, void*, uint);

struct EventHelper {
  char* evtName;
  AutoRoot** argv;
  uint argc;
  bool executed;
};

void CALLBACK EventTimerProc(LPVOID lpArg, DWORD dwTimerLowValue, DWORD dwTimerHighValue);
