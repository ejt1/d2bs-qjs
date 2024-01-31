#include <algorithm>
#include <vector>
#include <assert.h>

#include "ScriptEngine.h"
#include "Core.h"
#include "JSGlobalClasses.h"
#include "JSUnit.h"
#include "Constants.h"
#include "D2BS.h"
#include "D2Ptrs.h"
#include "Events.h"
#include "Helpers.h"

// internal ForEachScript helpers
bool __fastcall DisposeScript(Script* script, void*, uint);
bool __fastcall StopScript(Script* script, void* argv, uint argc);

ScriptEngine::ScriptEngine() : m_console(NULL), m_scripts(), m_state(Stopped), m_lock({0}), m_scriptListLock({0}), m_DelayedExecList(), m_delayedExecKey() {
}

ScriptEngine::~ScriptEngine() {
}

ScriptEngine* ScriptEngine::GetInstance() {
  static ScriptEngine instance;
  return &instance;
}

Script* ScriptEngine::NewScript(const wchar_t* file, ScriptMode mode, uint argc, JSAutoStructuredCloneBuffer** argv, bool /*recompile*/) {
  if (GetState() != Running)
    return NULL;

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

void ScriptEngine::RunCommand(const wchar_t* command) {
  if (GetState() != Running)
    return;
  try {
    // EnterCriticalSection(&lock);
    m_console->RunCommand(command);
    // LeaveCriticalSection(&lock);
  } catch (std::exception e) {
    // LeaveCriticalSection(&lock);
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
    evt->owner = script;
    evt->name = _strdup("DisposeMe");
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
unsigned int ScriptEngine::GetCount(bool active, bool unexecuted) {
  if (GetState() != Running)
    return 0;
  LockScriptList("getCount");
  // EnterCriticalSection(&lock);
  int count = m_scripts.size();
  for (ScriptMap::iterator it = m_scripts.begin(); it != m_scripts.end(); it++) {
    if (!active && it->second->IsRunning() && !it->second->IsAborted())
      count--;
    if (!unexecuted && it->second->GetExecutionCount() == 0 && !it->second->IsRunning())
      count--;
  }
  assert(count >= 0);
  UnLockScriptList("getCount");
  // LeaveCriticalSection(&lock);
  return count;
}

BOOL ScriptEngine::Startup(void) {
  if (GetState() == Stopped) {
    m_state = Starting;
    InitializeCriticalSection(&m_scriptListLock);
    // InitializeCriticalSection(&lock);
    // EnterCriticalSection(&lock);
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
    m_state = Running;
    // LeaveCriticalSection(&lock);
    UnLockScriptList("startup - leave");
  }
  return TRUE;
}

void ScriptEngine::Shutdown(void) {
  if (GetState() == Running) {
    // bring the engine down properly
    // EnterCriticalSection(&lock);
    LockScriptList("Shutdown");
    m_state = Stopping;
    StopAll(true);
    m_console->Stop(true, true);

    // clear all scripts now that they're stopped
    ForEachScript(::DisposeScript, NULL, 0);

    if (!m_scripts.empty())
      m_scripts.clear();

    UnLockScriptList("shutdown");
    // LeaveCriticalSection(&lock);
    DeleteCriticalSection(&m_lock);
    m_state = Stopped;
  }
}

void ScriptEngine::StopAll(bool forceStop) {
  if (GetState() != Running)
    return;

  // EnterCriticalSection(&lock);
  ForEachScript(StopScript, &forceStop, 1);

  // LeaveCriticalSection(&lock);
}

void ScriptEngine::UpdateConsole() {
  m_console->UpdatePlayerGid();
}
void ScriptEngine::FlushCache(void) {
  if (GetState() != Running)
    return;

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
  // LeaveCriticalSection(&lock);
}

bool ScriptEngine::ForEachScript(ScriptCallback callback, void* argv, uint argc) {
  if (callback == NULL || m_scripts.size() < 1)
    return false;
  bool block = false;
  // EnterCriticalSection(&lock);

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
  // LeaveCriticalSection(&lock);  // was previously locked after callback calls.

  return block;
}

void ScriptEngine::InitClass(JSContext* _context, JSObject* globalObject, JSClass* classp, JSFunctionSpec* methods, JSPropertySpec* props, JSFunctionSpec* s_methods,
                             JSPropertySpec* s_props) {
  if (!JS_InitClass(_context, globalObject, NULL, classp, classp->construct, 0, props, methods, s_props, s_methods))
    throw std::exception("Couldn't initialize the class");
}

void ScriptEngine::DefineConstant(JSContext* _context, JSObject* globalObject, const char* name, int value) {
  if (!JS_DefineProperty(_context, globalObject, name, INT_TO_JSVAL(value), NULL, NULL, JSPROP_PERMANENT_VAR))
    throw std::exception("Couldn't initialize the constant");
}

// internal ForEachScript helper functions
bool __fastcall DisposeScript(Script* script, void*, uint) {
  sScriptEngine->DisposeScript(script);
  return true;
}

bool __fastcall StopScript(Script* script, void* argv, uint /*argc*/) {
  script->TriggerOperationCallback();
  if (script->GetMode() != kScriptModeCommand)
    script->Stop(*(bool*)(argv), sScriptEngine->GetState() == Stopping);
  return true;
}

bool __fastcall StopIngameScript(Script* script, void*, uint) {
  if (script->GetMode() == kScriptModeGame)
    script->Stop(true);
  return true;
}

int ScriptEngine::AddDelayedEvent(Event* evt, int freq) {
  m_delayedExecKey++;
  evt->arg1 = new DWORD(m_delayedExecKey);
  evt->arg2 = CreateWaitableTimer(NULL, true, NULL);

  __int64 start;
  start = freq * -10000;
  LARGE_INTEGER lStart;
  // Copy the relative time into a LARGE_INTEGER.
  lStart.LowPart = (DWORD)(start & 0xFFFFFFFF);
  lStart.HighPart = (LONG)(start >> 32);
  freq = (strcmp(evt->name, "setInterval") == 0) ? freq : 0;
  EnterCriticalSection(&Vars.cEventSection);
  m_DelayedExecList.push_back(evt);
  SetWaitableTimer((HANDLE*)evt->arg2, &lStart, freq, &EventTimerProc, evt, false);
  LeaveCriticalSection(&Vars.cEventSection);

  return m_delayedExecKey;
}

void ScriptEngine::RemoveDelayedEvent(int key) {
  std::list<Event*>::iterator it;
  it = m_DelayedExecList.begin();
  while (it != m_DelayedExecList.end()) {
    if (*(int*)(*it)->arg1 == key) {
      CancelWaitableTimer((HANDLE*)(*it)->arg2);
      CloseHandle((HANDLE*)(*it)->arg2);
      Event* evt = *it;
      evt->owner->UnregisterEvent(evt->name, *(jsval*)evt->arg3);
      delete evt->arg1;
      delete evt->arg3;
      free(evt->name);
      delete evt;
      it = m_DelayedExecList.erase(it);
    } else
      it++;
  }
  LeaveCriticalSection(&Vars.cEventSection);
}
void CALLBACK EventTimerProc(LPVOID lpArg, DWORD /*dwTimerLowValue*/, DWORD /*dwTimerHighValue*/) {
  Event* evt = (Event*)lpArg;
  evt->owner->FireEvent(evt);
}