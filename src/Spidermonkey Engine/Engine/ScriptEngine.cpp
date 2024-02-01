#include <algorithm>
#include <vector>
#include <assert.h>

#include "ScriptEngine.h"
#include "Core.h"
#include "JSGlobalClasses.h"
#include "JSUnit.h"
#include "Constants.h"
#include "Engine.h"
#include "D2Ptrs.h"
#include "Events.h"
#include "Helpers.h"

// internal ForEachScript helper functions
bool __fastcall DisposeScript(Script* script, void*, uint) {
  sScriptEngine->DisposeScript(script);
  return true;
}

bool __fastcall StopScript(Script* script, void* argv, uint /*argc*/) {
  script->TriggerOperationCallback();
  if (script->GetMode() != kScriptModeCommand)
    script->Stop(*(bool*)(argv));
  return true;
}

ScriptEngine::ScriptEngine() : m_console(NULL), m_lock({0}), m_scriptListLock({0}) {
  m_instance = this;
}

ScriptEngine::~ScriptEngine() {
}

ScriptEngine* ScriptEngine::GetInstance() {
  return m_instance;
}

BOOL ScriptEngine::Initialize(void) {
  InitializeCriticalSection(&m_scriptListLock);
  LockScriptList("startup - enter");
  if (wcslen(Vars.szConsole) > 0) {
    wchar_t file[_MAX_FNAME + _MAX_PATH];
    swprintf_s(file, _countof(file), L"%s\\%s", Vars.szScriptPath, Vars.szConsole);
    m_console = new Script(file, kScriptModeCommand);
  } else {
    m_console = new Script(L"", kScriptModeCommand);
  }
  m_scripts[L"console"] = m_console;
  m_console->Start();
  UnLockScriptList("startup - leave");
  return TRUE;
}

void ScriptEngine::Shutdown(void) {
  // bring the engine down properly
  LockScriptList("Shutdown");
  StopAll(true);
  m_console->Stop(true);

  // clear all scripts now that they're stopped
  ForEachScript(::DisposeScript, NULL, 0);

  if (!m_scripts.empty())
    m_scripts.clear();

  UnLockScriptList("shutdown");
  DeleteCriticalSection(&m_lock);
}

Script* ScriptEngine::NewScript(const wchar_t* file, ScriptMode mode, uint argc, JSAutoStructuredCloneBuffer** argv, bool /*recompile*/) {
  wchar_t* fileName = _wcsdup(file);
  _wcslwr_s(fileName, wcslen(file) + 1);

  try {
    if (m_scripts.count(fileName))
      m_scripts[fileName]->Stop();

    Script* script = new Script(fileName, mode, argc, argv);
    m_scripts[fileName] = script;
    free(fileName);
    return script;
  } catch (std::exception e) {
    LeaveCriticalSection(&m_lock);
    wchar_t* what = AnsiToUnicode(e.what());
    Print(what);
    delete[] what;
    free(fileName);
    return NULL;
  }
}

void ScriptEngine::FlushCache(void) {
  static bool isFlushing = false;

  if (isFlushing || Vars.bDisableCache)
    return;

  // EnterCriticalSection(&lock);
  // TODO: examine if this lock is necessary any more
  EnterCriticalSection(&Vars.cFlushCacheSection);

  isFlushing = true;

  ForEachScript(::DisposeScript, NULL, 0);

  isFlushing = false;

  LeaveCriticalSection(&Vars.cFlushCacheSection);
}

void ScriptEngine::RunCommand(const wchar_t* command) {
  try {
    m_console->RunCommand(command);
  } catch (std::exception e) {
    wchar_t* what = AnsiToUnicode(e.what());
    Print(what);
    delete[] what;
  }
}

void ScriptEngine::DisposeScript(Script* script) {
  LockScriptList("DisposeScript");

  const wchar_t* nFilename = script->GetFilename();

  if (m_scripts.count(nFilename))
    m_scripts.erase(nFilename);

  UnLockScriptList("DisposeScript");

  if (GetCurrentThreadId() == script->GetThreadId())
    delete script;
  else {
    // bad things happen if we delete from another thread
    Event* evt = new Event;
    evt->name = "DisposeMe";
    script->FireEvent(evt);
  }
}

void ScriptEngine::LockScriptList(char* /*loc*/) {
  EnterCriticalSection(&m_scriptListLock);
  // Log(loc);
}

void ScriptEngine::UnLockScriptList(char* /*loc*/) {
  // Log(loc);
  LeaveCriticalSection(&m_scriptListLock);
}

bool ScriptEngine::ForEachScript(ScriptCallback callback, void* argv, uint argc) {
  if (callback == NULL || m_scripts.size() < 1)
    return false;
  bool block = false;

  LockScriptList("forEachScript");

  // damn std::list not supporting operator[]...
  std::vector<Script*> list;
  for (ScriptMap::iterator it = m_scripts.begin(); it != m_scripts.end(); it++) list.push_back(it->second);

  UnLockScriptList("forEachScript");

  int count = list.size();
  // damn std::iterator not supporting manipulating the list...
  for (int i = 0; i < count; i++) {
    if (callback(list[i], argv, argc))
      block = true;
  }

  return block;
}

unsigned int ScriptEngine::GetCount(bool active, bool unexecuted) {
  LockScriptList("getCount");
  int count = m_scripts.size();
  for (ScriptMap::iterator it = m_scripts.begin(); it != m_scripts.end(); it++) {
    if (!active && it->second->IsRunning() && !it->second->IsAborted())
      count--;
    if (!unexecuted && it->second->IsUninitialized() && !it->second->IsRunning())
      count--;
  }
  assert(count >= 0);
  UnLockScriptList("getCount");
  return count;
}

void ScriptEngine::StopAll(bool forceStop) {
  ForEachScript(StopScript, &forceStop, 1);
}

void ScriptEngine::UpdateConsole() {
  m_console->UpdatePlayerGid();
}

bool __fastcall StopIngameScript(Script* script, void*, uint) {
  if (script->GetMode() == kScriptModeGame)
    script->Stop(true);
  return true;
}