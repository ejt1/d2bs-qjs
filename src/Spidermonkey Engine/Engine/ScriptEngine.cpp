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

ScriptEngine::ScriptEngine()
    : console(NULL), runtime(NULL), scripts(), state(Stopped), lock({0}), scriptListLock({0}), DelayedExecList(), delayedExecKey(), context(NULL) {
}

ScriptEngine::~ScriptEngine() {
}

ScriptEngine* ScriptEngine::GetInstance() {
  static ScriptEngine instance;
  return &instance;
}

Script* ScriptEngine::CompileFile(const wchar_t* file, ScriptState state, uint argc, JSAutoStructuredCloneBuffer** argv, bool recompile) {
  if (GetState() != Running)
    return NULL;

  wchar_t* fileName = _wcsdup(file);
  _wcslwr_s(fileName, wcslen(file) + 1);

  try {
    if (scripts.count(fileName))
      scripts[fileName]->Stop();

    Script* script = new Script(fileName, state, argc, argv);
    scripts[fileName] = script;
    free(fileName);
    return script;
  } catch (std::exception e) {
    LeaveCriticalSection(&lock);
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
    console->RunCommand(command);
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

  if (scripts.count(nFilename))
    scripts.erase(nFilename);

  UnLockScriptList("DisposeScript");

  if (GetCurrentThreadId() == script->threadId)
    delete script;
  else {
    // bad things happen if we delete from another thread
    Event* evt = new Event;
    evt->owner = script;
    evt->name = _strdup("DisposeMe");
    script->FireEvent(evt);
  }
}
void ScriptEngine::LockScriptList(char* loc) {
  EnterCriticalSection(&scriptListLock);
  // Log(loc);
}
void ScriptEngine::UnLockScriptList(char* loc) {
  // Log(loc);
  LeaveCriticalSection(&scriptListLock);
}
unsigned int ScriptEngine::GetCount(bool active, bool unexecuted) {
  if (GetState() != Running)
    return 0;
  LockScriptList("getCount");
  // EnterCriticalSection(&lock);
  int count = scripts.size();
  for (ScriptMap::iterator it = scripts.begin(); it != scripts.end(); it++) {
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
    state = Starting;
    InitializeCriticalSection(&scriptListLock);
    // InitializeCriticalSection(&lock);
    // EnterCriticalSection(&lock);
    LockScriptList("startup - enter");
    if (wcslen(Vars.szConsole) > 0) {
      wchar_t file[_MAX_FNAME + _MAX_PATH];
      swprintf_s(file, _countof(file), L"%s\\%s", Vars.szScriptPath, Vars.szConsole);
      console = new Script(file, Command);
    } else {
      console = new Script(L"", Command);
    }
    scripts[L"console"] = console;
    console->BeginThread(ScriptThread);
    state = Running;
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
    state = Stopping;
    StopAll(true);
    console->Stop(true, true);

    // clear all scripts now that they're stopped
    ForEachScript(::DisposeScript, NULL, 0);

    if (!scripts.empty())
      scripts.clear();

    if (runtime) {
      JS_DestroyContext(context);
      JS_DestroyRuntime(runtime);
      JS_ShutDown();
      runtime = NULL;
    }
    UnLockScriptList("shutdown");
    // LeaveCriticalSection(&lock);
    DeleteCriticalSection(&lock);
    state = Stopped;
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
  console->UpdatePlayerGid();
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
  if (callback == NULL || scripts.size() < 1)
    return false;
  bool block = false;
  // EnterCriticalSection(&lock);

  LockScriptList("forEachScript");

  // damn std::list not supporting operator[]...
  std::vector<Script*> list;
  for (ScriptMap::iterator it = scripts.begin(); it != scripts.end(); it++) list.push_back(it->second);

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

void ScriptEngine::InitClass(JSContext* context, JSObject* globalObject, JSClass* classp, JSFunctionSpec* methods, JSPropertySpec* props, JSFunctionSpec* s_methods,
                             JSPropertySpec* s_props) {
  if (!JS_InitClass(context, globalObject, NULL, classp, classp->construct, 0, props, methods, s_props, s_methods))
    throw std::exception("Couldn't initialize the class");
}

void ScriptEngine::DefineConstant(JSContext* context, JSObject* globalObject, const char* name, int value) {
  if (!JS_DefineProperty(context, globalObject, name, INT_TO_JSVAL(value), NULL, NULL, JSPROP_PERMANENT_VAR))
    throw std::exception("Couldn't initialize the constant");
}

// internal ForEachScript helper functions
bool __fastcall DisposeScript(Script* script, void*, uint) {
  sScriptEngine->DisposeScript(script);
  return true;
}

bool __fastcall StopScript(Script* script, void* argv, uint argc) {
  script->TriggerOperationCallback();
  if (script->GetState() != Command)
    script->Stop(*(bool*)(argv), sScriptEngine->GetState() == Stopping);
  return true;
}

bool __fastcall StopIngameScript(Script* script, void*, uint) {
  if (script->GetState() == InGame)
    script->Stop(true);
  return true;
}

int ScriptEngine::AddDelayedEvent(Event* evt, int freq) {
  delayedExecKey++;
  evt->arg1 = new DWORD(delayedExecKey);
  evt->arg2 = CreateWaitableTimer(NULL, true, NULL);

  __int64 start;
  start = freq * -10000;
  LARGE_INTEGER lStart;
  // Copy the relative time into a LARGE_INTEGER.
  lStart.LowPart = (DWORD)(start & 0xFFFFFFFF);
  lStart.HighPart = (LONG)(start >> 32);
  freq = (strcmp(evt->name, "setInterval") == 0) ? freq : 0;
  EnterCriticalSection(&Vars.cEventSection);
  DelayedExecList.push_back(evt);
  SetWaitableTimer((HANDLE*)evt->arg2, &lStart, freq, &EventTimerProc, evt, false);
  LeaveCriticalSection(&Vars.cEventSection);

  return delayedExecKey;
}

void ScriptEngine::RemoveDelayedEvent(int key) {
  std::list<Event*>::iterator it;
  it = DelayedExecList.begin();
  while (it != DelayedExecList.end()) {
    if (*(DWORD*)(*it)->arg1 == key) {
      CancelWaitableTimer((HANDLE*)(*it)->arg2);
      CloseHandle((HANDLE*)(*it)->arg2);
      Event* evt = *it;
      evt->owner->UnregisterEvent(evt->name, *(jsval*)evt->arg3);
      delete evt->arg1;
      delete evt->arg3;
      free(evt->name);
      delete evt;
      it = DelayedExecList.erase(it);
    } else
      it++;
  }
  LeaveCriticalSection(&Vars.cEventSection);
}
void CALLBACK EventTimerProc(LPVOID lpArg, DWORD dwTimerLowValue, DWORD dwTimerHighValue) {
  Event* evt = (Event*)lpArg;
  evt->owner->FireEvent(evt);
}