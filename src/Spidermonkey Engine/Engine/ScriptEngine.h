#pragma once

#include <list>
#include <map>
#include <string>

#include "js32.h"
#include "Script.h"

typedef std::map<std::string, Script*> ScriptMap;

typedef bool(__fastcall* ScriptCallback)(Script*, void*, uint32_t);

enum EngineState { Starting, Running, Paused, Stopping, Stopped };

class ScriptEngine {
  friend class Script;

 public:
  ScriptEngine();
  virtual ~ScriptEngine();

  ScriptEngine(const ScriptEngine&) = delete;
  ScriptEngine& operator=(const ScriptEngine&) = delete;

  static ScriptEngine* GetInstance();

  BOOL Initialize();
  void Shutdown();

  Script* NewScript(const char* file, ScriptMode mode/*, uint32_t argc = 0, JSAutoStructuredCloneBuffer** argv = NULL*/, bool recompile = false);

  void FlushCache(void);

  void RunCommand(const char* command);
  void DisposeScript(Script* script);

  void LockScriptList(const char* loc);
  void UnLockScriptList(const char* loc);

  ScriptMap& scripts() {
    return m_scripts;
  }
  bool ForEachScript(ScriptCallback callback, void* argv, uint32_t argc);
  unsigned int GetCount(bool active = true, bool unexecuted = false);

  void StopAll(bool forceStop = false);
  void UpdateConsole();

 private:
  inline static ScriptEngine* m_instance;
  Script* m_console;
  CRITICAL_SECTION m_scriptListLock;
  ScriptMap m_scripts;
  CRITICAL_SECTION m_lock;
};

#define sScriptEngine ScriptEngine::GetInstance()

// these ForEachScript helpers are exposed in case they can be of use somewhere
bool __fastcall StopIngameScript(Script* script, void*, uint32_t);