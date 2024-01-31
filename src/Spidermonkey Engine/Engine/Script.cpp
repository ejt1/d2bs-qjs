#include <io.h>
#include <algorithm>

#include "Script.h"
#include "Core.h"
#include "Constants.h"
#include "D2Ptrs.h"
#include "JSUnit.h"
#include "Helpers.h"
#include "ScriptEngine.h"
#include "D2BS.h"
#include "JSGlobalFuncs.h"
#include "Console.h"

#include <chrono>

Script::Script(const wchar_t* file, ScriptState state, uint argc, JSAutoStructuredCloneBuffer** argv)
    : m_context(NULL),
      m_globalObject(NULL),
      m_scriptObject(NULL),
      m_script(NULL),
      m_execCount(0),
      m_isAborted(false),
      m_isPaused(false),
      m_isReallyPaused(false),
      m_scriptState(state),
      m_threadHandle(INVALID_HANDLE_VALUE),
      m_threadId(0),
      m_argc(argc),
      m_argv(argv) {
  InitializeCriticalSection(&m_lock);
  // moved the runtime initilization to thread start
  m_LastGC = 0;
  m_hasActiveCX = false;
  m_eventSignal = CreateEvent(nullptr, true, false, nullptr);

  if (m_scriptState == Command && wcslen(file) < 1) {
    m_fileName = std::wstring(L"Command Line");
  } else {
    if (_waccess(file, 0) != 0) {
      DEBUG_LOG(file);

      throw std::exception("File not found");
    }

    wchar_t* tmpName = _wcsdup(file);
    if (!tmpName)
      throw std::exception("Could not dup filename");

    _wcslwr_s(tmpName, wcslen(file) + 1);
    m_fileName = std::wstring(tmpName);
    replace(m_fileName.begin(), m_fileName.end(), L'/', L'\\');
    free(tmpName);
  }
}

Script::~Script(void) {
  m_isAborted = true;
  m_hasActiveCX = false;
  // JS_SetPendingException(context, JSVAL_NULL);
  if (JS_IsInRequest(m_runtime))
    JS_EndRequest(m_context);

  EnterCriticalSection(&m_lock);
  //    JS_SetRuntimeThread(rt);
  JS_DestroyContext(m_context);
  // JS_ClearRuntimeThread(rt);
  JS_DestroyRuntime(m_runtime);

  m_context = NULL;
  m_scriptObject = NULL;
  m_globalObject = NULL;
  m_script = NULL;
  CloseHandle(m_eventSignal);
  m_includes.clear();
  if (m_threadHandle != INVALID_HANDLE_VALUE)
    CloseHandle(m_threadHandle);
  LeaveCriticalSection(&m_lock);
  DeleteCriticalSection(&m_lock);
}

void Script::Run(void) {
  try {
    m_runtime = JS_NewRuntime(Vars.dwMemUsage, JS_NO_HELPER_THREADS);
    // JS_SetNativeStackQuota(runtime, (size_t)50000);
    //  JS_SetRuntimeThread(runtime);
    JS_SetContextCallback(m_runtime, contextCallback);

    m_context = JS_NewContext(m_runtime, 0x800000);
    if (!m_context)
      throw std::exception("Couldn't create the context");

    JS_SetErrorReporter(m_context, reportError);
    JS_SetOperationCallback(m_context, operationCallback);
    JS_SetOptions(m_context, JSOPTION_STRICT | JSOPTION_VAROBJFIX);
    JS_SetVersion(m_context, JSVERSION_LATEST);
    //

    JS_SetContextPrivate(m_context, this);

    JS_BeginRequest(m_context);

    m_globalObject = JS_GetGlobalObject(m_context);
    jsval meVal = JSVAL_VOID;
    if (JS_GetProperty(GetContext(), m_globalObject, "me", &meVal) != JS_FALSE) {
      JSObject* meObject = JSVAL_TO_OBJECT(meVal);
      m_me = (myUnit*)JS_GetPrivate(GetContext(), meObject);
    }

    if (m_scriptState == Command) {
      if (wcslen(Vars.szConsole) > 0) {
        m_script = JS_CompileFile(m_context, m_globalObject, m_fileName);
      } else {
        char* cmd = "function main() {print('ÿc2D2BSÿc0 :: Started Console'); while (true){delay(10000)};}  ";
        m_script = JS_CompileScript(m_context, m_globalObject, cmd, strlen(cmd), "Command Line", 1);
      }
      JS_AddNamedScriptRoot(m_context, &m_script, "console script");
    } else
      m_script = JS_CompileFile(m_context, m_globalObject, m_fileName);

    if (!m_script)
      throw std::exception("Couldn't compile the script");

    JS_EndRequest(m_context);
    // JS_RemoveScriptRoot(context, &script);

  } catch (std::exception&) {
    if (m_scriptObject)
      JS_RemoveRoot(m_context, &m_scriptObject);
    if (m_context) {
      JS_EndRequest(m_context);
      JS_DestroyContext(m_context);
    }
    LeaveCriticalSection(&m_lock);
    throw;
  }
  // only let the script run if it's not already running
  if (IsRunning())
    return;
  m_hasActiveCX = true;
  m_isAborted = false;

  jsval main = INT_TO_JSVAL(1), dummy = INT_TO_JSVAL(1);
  JS_BeginRequest(GetContext());

  // args passed from load
  jsval* argvalue = new jsval[m_argc];
  for (uint i = 0; i < m_argc; i++) m_argv[i]->read(m_context, &argvalue[i]);

  for (uint j = 0; j < m_argc; j++) JS_AddValueRoot(m_context, &argvalue[j]);

  JS_AddValueRoot(GetContext(), &main);
  JS_AddValueRoot(GetContext(), &dummy);
  if (JS_ExecuteScript(GetContext(), m_globalObject, m_script, &dummy) != JS_FALSE && JS_GetProperty(GetContext(), m_globalObject, "main", &main) != JS_FALSE &&
      JSVAL_IS_FUNCTION(GetContext(), main)) {
    JS_CallFunctionValue(GetContext(), m_globalObject, main, this->m_argc, argvalue, &dummy);
  }
  JS_RemoveValueRoot(GetContext(), &main);
  JS_RemoveValueRoot(GetContext(), &dummy);
  for (uint j = 0; j < m_argc; j++) JS_RemoveValueRoot(GetContext(), &argvalue[j]);

  /*for(uint i = 0; i < argc; i++)  //crashes spidermonkey cleans itself up?
  {
          argv[i]->clear();
          delete argv[i];
  }*/

  JS_EndRequest(GetContext());

  m_execCount++;
  // Stop();
}

void Script::Join() {
  EnterCriticalSection(&m_lock);
  HANDLE hThread = m_threadHandle;
  LeaveCriticalSection(&m_lock);

  if (hThread != INVALID_HANDLE_VALUE)
    WaitForSingleObject(hThread, INFINITE);
}

void Script::Pause(void) {
  if (!IsAborted() && !IsPaused())
    m_isPaused = true;
  TriggerOperationCallback();
  SetEvent(m_eventSignal);
}

void Script::Resume(void) {
  if (!IsAborted() && IsPaused())
    m_isPaused = false;
  TriggerOperationCallback();
  SetEvent(m_eventSignal);
}

void Script::Stop(bool force, bool reallyForce) {
  // if we've already stopped, just return
  if (m_isAborted)
    return;
  EnterCriticalSection(&m_lock);
  // tell everyone else that the script is aborted FIRST
  m_isAborted = true;
  m_isPaused = false;
  m_isReallyPaused = false;
  if (GetState() != Command) {
    const wchar_t* displayName = m_fileName.c_str() + wcslen(Vars.szScriptPath) + 1;
    Print(L"Script %s ended", displayName);
  }

  // trigger call back so script ends
  TriggerOperationCallback();
  SetEvent(m_eventSignal);

  // normal wait: 500ms, forced wait: 300ms, really forced wait: 100ms
  int maxCount = (force ? (reallyForce ? 10 : 30) : 50);
  if (GetCurrentThreadId() != GetThreadId()) {
    for (int i = 0; m_hasActiveCX == true; i++) {
      // if we pass the time frame, just ignore the wait because the thread will end forcefully anyway
      if (i >= maxCount)
        break;
      Sleep(10);
    }
  }
  LeaveCriticalSection(&m_lock);
}

bool Script::IsPaused(void) {
  return m_isPaused;
}

bool Script::IsRunning(void) {
  return m_context && !(IsAborted() || IsPaused() || !m_hasActiveCX);
}

bool Script::IsAborted() {
  return m_isAborted;
}

bool Script::BeginThread(LPTHREAD_START_ROUTINE ThreadFunc) {
  EnterCriticalSection(&m_lock);
  DWORD dwExitCode = STILL_ACTIVE;

  if ((!GetExitCodeThread(m_threadHandle, &dwExitCode) || dwExitCode != STILL_ACTIVE) &&
      (m_threadHandle = CreateThread(0, 0, ThreadFunc, this, 0, &m_threadId)) != NULL) {
    LeaveCriticalSection(&m_lock);
    return true;
  }

  m_threadHandle = INVALID_HANDLE_VALUE;
  LeaveCriticalSection(&m_lock);
  return false;
}

void Script::RunCommand(const wchar_t* command) {
  // RUNCOMMANDSTRUCT* rcs = new RUNCOMMANDSTRUCT;
  // rcs->script = this;
  // rcs->command = _wcsdup(command);

  if (m_isAborted) {  // this should never happen -bob
    // RUNCOMMANDSTRUCT* rcs = new RUNCOMMANDSTRUCT;

    // rcs->script = this;
    // rcs->command = _wcsdup(L"delay(1000000);");

    Log(L"Console Aborted HELP!");
  }

  Event* evt = new Event;
  evt->owner = this;
  evt->argc = m_argc;
  evt->name = _strdup("Command");
  evt->arg1 = _wcsdup(command);
  evt->owner->FireEvent(evt);
}

const wchar_t* Script::GetShortFilename() {
  if (wcscmp(m_fileName.c_str(), L"Command Line") == 0)
    return m_fileName.c_str();
  else
    return (m_fileName.c_str() + wcslen(Vars.szScriptPath) + 1);
}

int Script::GetExecutionCount(void) {
  return m_execCount;
}

DWORD Script::GetThreadId(void) {
  return (m_threadHandle == INVALID_HANDLE_VALUE ? -1 : m_threadId);
}

void Script::UpdatePlayerGid(void) {
  m_me->dwUnitId = (D2CLIENT_GetPlayerUnit() == NULL ? NULL : D2CLIENT_GetPlayerUnit()->dwUnitId);
}

bool Script::IsIncluded(const wchar_t* file) {
  uint count = 0;
  wchar_t* fname = _wcsdup(file);
  if (!fname)
    return false;

  _wcslwr_s(fname, wcslen(fname) + 1);
  StringReplace(fname, '/', '\\', wcslen(fname));
  count = m_includes.count(std::wstring(fname));
  free(fname);

  return !!count;
}

bool Script::Include(const wchar_t* file) {
  // since includes will happen on the same thread, locking here is acceptable
  EnterCriticalSection(&m_lock);
  wchar_t* fname = _wcsdup(file);
  if (!fname) {
    LeaveCriticalSection(&m_lock);
    return false;
  }
  _wcslwr_s(fname, wcslen(fname) + 1);
  StringReplace(fname, L'/', L'\\', wcslen(fname));

  // don't invoke the string ctor more than once...
  std::wstring currentFileName = std::wstring(fname);
  // ignore already included, 'in-progress' includes, and self-inclusion
  if (!!m_includes.count(fname) || !!m_inProgress.count(fname) || (currentFileName.compare(m_fileName.c_str()) == 0)) {
    LeaveCriticalSection(&m_lock);
    free(fname);
    return true;
  }
  bool rval = false;

  JSContext* cx = GetContext();

  JS_BeginRequest(cx);

  JSScript* _script = JS_CompileFile(cx, m_globalObject, fname);
  if (_script) {
    jsval dummy;
    m_inProgress[fname] = true;
    rval = !!JS_ExecuteScript(cx, m_globalObject, _script, &dummy);
    if (rval)
      m_includes[fname] = true;
    else
      JS_ReportPendingException(cx);
    m_inProgress.erase(fname);
    // JS_RemoveRoot(&scriptObj);
  } else
    JS_ReportPendingException(cx);

  JS_EndRequest(cx);
  // JS_RemoveScriptRoot(cx, &script);
  LeaveCriticalSection(&m_lock);
  free(fname);
  return rval;
}

bool Script::IsListenerRegistered(const char* evtName) {
  return strlen(evtName) > 0 && m_functions.count(evtName) > 0;
}

void Script::RegisterEvent(const char* evtName, jsval evtFunc) {
  EnterCriticalSection(&m_lock);
  if (JSVAL_IS_FUNCTION(m_context, evtFunc) && strlen(evtName) > 0) {
    AutoRoot* root = new AutoRoot(m_context, evtFunc);
    m_functions[evtName].push_back(root);
  }
  LeaveCriticalSection(&m_lock);
}

bool Script::IsRegisteredEvent(const char* evtName, jsval evtFunc) {
  // nothing can be registered under an empty name
  if (strlen(evtName) < 1)
    return false;

  // if there are no events registered under that name at all, then obviously there
  // can't be a specific one registered under that name
  if (m_functions.count(evtName) < 1)
    return false;

  for (FunctionList::iterator it = m_functions[evtName].begin(); it != m_functions[evtName].end(); it++)
    if (*(*it)->value() == evtFunc)
      return true;

  return false;
}

void Script::UnregisterEvent(const char* evtName, jsval evtFunc) {
  if (strlen(evtName) < 1)
    return;

  EnterCriticalSection(&m_lock);
  AutoRoot* func = NULL;
  for (FunctionList::iterator it = m_functions[evtName].begin(); it != m_functions[evtName].end(); it++) {
    if (*(*it)->value() == evtFunc) {
      func = *it;
      break;
    }
  }
  m_functions[evtName].remove(func);
  // func->Release();
  delete func;

  // Remove event completely if there are no listeners for it.
  if (m_functions.count(evtName) > 0 && m_functions[evtName].size() == 0)
    m_functions.erase(evtName);

  LeaveCriticalSection(&m_lock);
}

void Script::ClearEvent(const char* evtName) {
  EnterCriticalSection(&m_lock);
  for (FunctionList::iterator it = m_functions[evtName].begin(); it != m_functions[evtName].end(); it++) {
    AutoRoot* func = *it;
    func->Release();
    delete func;
  }
  m_functions[evtName].clear();
  LeaveCriticalSection(&m_lock);
}

void Script::ClearAllEvents(void) {
  EnterCriticalSection(&m_lock);
  for (FunctionMap::iterator it = m_functions.begin(); it != m_functions.end(); it++) ClearEvent(it->first.c_str());
  m_functions.clear();
  LeaveCriticalSection(&m_lock);
}

void Script::FireEvent(Event* evt) {
  // EnterCriticalSection(&sScriptEngine->lock);
  EnterCriticalSection(&Vars.cEventSection);
  evt->owner->m_EventList.push_front(evt);
  LeaveCriticalSection(&Vars.cEventSection);

  if (evt->owner && evt->owner->IsRunning()) {
    evt->owner->TriggerOperationCallback();
  }
  SetEvent(m_eventSignal);
  // LeaveCriticalSection(&sScriptEngine->lock);
}

void Script::ClearEventList() {
  while (m_EventList.size() > 0) {
    EnterCriticalSection(&Vars.cEventSection);
    Event* evt = m_EventList.back();
    m_EventList.pop_back();
    LeaveCriticalSection(&Vars.cEventSection);
    ExecScriptEvent(evt, true);  // clean list and pop events
  }

  ClearAllEvents();
}

void Script::BlockThread(DWORD delay) {
  DWORD start = GetTickCount();
  int amt = delay - (GetTickCount() - start);
  if (IsAborted()) {
    return;
  }

  while (amt > 0) {  // had a script deadlock here, make sure were positve with amt
    WaitForSingleObjectEx(m_eventSignal, amt, true);
    ResetEvent(m_eventSignal);
    ProcessAllEvents();

    if (JS_GetGCParameter(m_runtime, JSGC_BYTES) - m_LastGC > 524288)  // gc every .5 mb
    {
      JS_GC(m_runtime);
      m_LastGC = JS_GetGCParameter(m_runtime, JSGC_BYTES);
    }

    amt = delay - (GetTickCount() - start);
  }
}

void Script::ProcessAllEvents() {
  while (m_EventList.size() > 0 && !!!(JSBool)(IsAborted() || ((GetState() == InGame) && ClientState() == ClientStateMenu))) {
    ProcessOneEvent();
  }
}

void Script::ProcessOneEvent() {
  EnterCriticalSection(&Vars.cEventSection);
  Event* evt = m_EventList.back();
  m_EventList.pop_back();
  LeaveCriticalSection(&Vars.cEventSection);
  ExecScriptEvent(evt, false);
}

void Script::ExecuteEvent(char* evtName, int argc, jsval* argv, bool* block) {
  for (const auto& root : m_functions[evtName]) {
    jsval rval;
    JS_CallFunctionValue(m_context, JS_GetGlobalObject(m_context), *root->value(), argc, argv, &rval);
    if (block) {
      *block |= static_cast<bool>(JSVAL_IS_BOOLEAN(rval) && JSVAL_TO_BOOLEAN(rval));
    }
  }
}

void Script::OnDestroyContext() {
  m_hasActiveCX = false;
  ClearEventList();
  Genhook::Clean(this);
}

#ifdef DEBUG
typedef struct tagTHREADNAME_INFO {
  DWORD dwType;      // must be 0x1000
  LPCWSTR szName;    // pointer to name (in user addr space)
  DWORD dwThreadID;  // thread ID (-1=caller thread)
  DWORD dwFlags;     // reserved for future use, must be zero
} THREADNAME_INFO;

void SetThreadName(DWORD dwThreadID, LPCWSTR szThreadName) {
  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = szThreadName;
  info.dwThreadID = dwThreadID;
  info.dwFlags = 0;

  __try {
    RaiseException(0x406D1388, 0, sizeof(info) / sizeof(DWORD), (DWORD*)&info);
  } __except (EXCEPTION_CONTINUE_EXECUTION) {
  }
}
#endif

DWORD WINAPI ScriptThread(void* data) {
  Script* script = (Script*)data;
  if (script) {
#ifdef DEBUG
    SetThreadName(0xFFFFFFFF, script->GetShortFilename());
#endif
    script->Run();
    if (Vars.bDisableCache)
      sScriptEngine->DisposeScript(script);
  }
  return 0;
}

JSBool operationCallback(JSContext* cx) {
  Script* script = (Script*)JS_GetContextPrivate(cx);
  static int callBackCount = 0;
  callBackCount++;
  bool pause = script->IsPaused();

  if (pause)
    script->SetPauseState(true);
  while (script->IsPaused()) {
    Sleep(50);
    JS_MaybeGC(cx);
  }
  if (pause)
    script->SetPauseState(false);

  if (!!!(JSBool)(script->IsAborted() || ((script->GetState() == InGame) && ClientState() == ClientStateMenu))) {
    script->ProcessAllEvents();
    return !!!(JSBool)(script->IsAborted() || ((script->GetState() == InGame) && ClientState() == ClientStateMenu));
  } else {
    return false;
  }
}

JSBool contextCallback(JSContext* ctx, uint contextOp) {
  if (contextOp == JSCONTEXT_NEW) {
    JS_BeginRequest(ctx);

    JS_SetErrorReporter(ctx, reportError);
    JS_SetOperationCallback(ctx, operationCallback);

    JS_SetVersion(ctx, JSVERSION_LATEST);
    JS_SetOptions(ctx, JSOPTION_METHODJIT | JSOPTION_TYPE_INFERENCE | JSOPTION_ION | JSOPTION_VAROBJFIX | JSOPTION_ASMJS | JSOPTION_STRICT);

    // JS_SetGCZeal(cx, 2, 1);
    JSObject* globalObject = JS_NewGlobalObject(ctx, &global_obj, NULL);
    JS_SetGCParameter(JS_GetRuntime(ctx), JSGC_MODE, 2);

    if (JS_InitStandardClasses(ctx, globalObject) == JS_FALSE)
      return JS_FALSE;
    if (JS_DefineFunctions(ctx, globalObject, global_funcs) == JS_FALSE)
      return JS_FALSE;

    myUnit* lpUnit = new myUnit;
    memset(lpUnit, NULL, sizeof(myUnit));

    UnitAny* player = D2CLIENT_GetPlayerUnit();
    lpUnit->dwMode = (DWORD)-1;
    lpUnit->dwClassId = (DWORD)-1;
    lpUnit->dwType = UNIT_PLAYER;
    lpUnit->dwUnitId = player ? player->dwUnitId : NULL;
    lpUnit->_dwPrivateType = PRIVATE_UNIT;

    for (JSClassSpec* entry = global_classes; entry->classp != NULL; entry++)
      sScriptEngine->InitClass(ctx, globalObject, entry->classp, entry->methods, entry->properties, entry->static_methods, entry->static_properties);

    JSObject* meObject = BuildObject(ctx, &unit_class, unit_methods, me_props, lpUnit);
    if (!meObject)
      return JS_FALSE;

    if (JS_DefineProperty(ctx, globalObject, "me", OBJECT_TO_JSVAL(meObject), NULL, NULL, JSPROP_PERMANENT_VAR) == JS_FALSE)
      return JS_FALSE;

#define DEFCONST(vp) sScriptEngine->DefineConstant(ctx, globalObject, #vp, vp)
    DEFCONST(FILE_READ);
    DEFCONST(FILE_WRITE);
    DEFCONST(FILE_APPEND);
#undef DEFCONST

    JS_EndRequest(ctx);
  }
  if (contextOp == JSCONTEXT_DESTROY) {
    Script* script = (Script*)JS_GetContextPrivate(ctx);
    script->OnDestroyContext();
  }
  return JS_TRUE;
}

void reportError(JSContext* cx, const char* message, JSErrorReport* report) {
  (cx);

  bool warn = JSREPORT_IS_WARNING(report->flags);
  bool isStrict = JSREPORT_IS_STRICT(report->flags);
  const char* type = (warn ? "Warning" : "Error");
  const char* strict = (isStrict ? "Strict " : "");
  wchar_t* filename = report->filename ? AnsiToUnicode(report->filename) : _wcsdup(L"<unknown>");
  wchar_t* displayName = filename;
  if (_wcsicmp(L"Command Line", filename) != 0 && _wcsicmp(L"<unknown>", filename) != 0)
    displayName = filename + wcslen(Vars.szPath);

  Log(L"[%hs%hs] Code(%d) File(%s:%d) %hs\nLine: %hs", strict, type, report->errorNumber, filename, report->lineno, message, report->linebuf);
  Print(L"[\u00FFc%d%hs%hs\u00FFc0 (%d)] File(%s:%d) %hs", (warn ? 9 : 1), strict, type, report->errorNumber, displayName, report->lineno, message);

  if (filename[0] == L'<')
    free(filename);
  else
    delete[] filename;

  if (Vars.bQuitOnError && !JSREPORT_IS_WARNING(report->flags) && ClientState() == ClientStateInGame)
    D2CLIENT_ExitGame();
  else
    Console::ShowBuffer();
}

bool ExecScriptEvent(Event* evt, bool clearList) {
  JSContext* cx = nullptr;

  if (!clearList)
    cx = evt->owner->GetContext();
  char* evtName = (char*)evt->name;
  if (strcmp(evtName, "itemaction") == 0) {
    if (!clearList) {
      DWORD* gid = (DWORD*)evt->arg1;
      char* code = (char*)evt->arg2;
      DWORD* mode = (DWORD*)evt->arg3;
      bool* global = (bool*)evt->arg4;

      jsval* argv = new jsval[evt->argc];
      JS_BeginRequest(cx);

      argv[0] = JS_NumberValue(*gid);
      argv[1] = JS_NumberValue(*mode);
      argv[2] = (STRING_TO_JSVAL(JS_NewStringCopyZ(cx, code)));
      argv[3] = (BOOLEAN_TO_JSVAL(*global));
      for (int j = 0; j < 4; j++) JS_AddValueRoot(cx, &argv[j]);

      bool block;
      evt->owner->ExecuteEvent(evtName, 4, argv, &block);
      JS_EndRequest(cx);
      for (int j = 0; j < 4; j++) JS_RemoveValueRoot(cx, &argv[j]);
    }
    delete evt->arg1;
    free(evt->arg2);
    delete evt->arg3;
    delete evt->arg4;
    free(evt->name);
    delete evt;
    return true;
  }
  if (strcmp(evtName, "gameevent") == 0) {
    if (!clearList) {
      jsval* argv = new jsval[5];
      JS_BeginRequest(cx);
      argv[0] = JS_NumberValue(*(BYTE*)evt->arg1);
      argv[1] = JS_NumberValue(*(DWORD*)evt->arg2);
      argv[2] = JS_NumberValue(*(DWORD*)evt->arg3);
      argv[3] = (STRING_TO_JSVAL(JS_NewStringCopyZ(cx, (char*)evt->arg4)));
      argv[4] = (STRING_TO_JSVAL(JS_NewUCStringCopyZ(cx, (wchar_t*)evt->arg5)));

      for (int j = 0; j < 5; j++) JS_AddValueRoot(cx, &argv[j]);

      evt->owner->ExecuteEvent(evtName, 5, argv);

      JS_EndRequest(cx);
      for (int j = 0; j < 5; j++) JS_RemoveValueRoot(cx, &argv[j]);
    }
    delete evt->arg1;
    delete evt->arg2;
    delete evt->arg3;
    free(evt->arg4);
    free(evt->arg5);
    free(evt->name);
    delete evt;
    return true;
  }
  if (strcmp(evtName, "copydata") == 0) {
    if (!clearList) {
      jsval* argv = new jsval[2];
      JS_BeginRequest(cx);
      argv[0] = JS_NumberValue(*(DWORD*)evt->arg1);
      argv[1] = STRING_TO_JSVAL(JS_NewUCStringCopyZ(cx, (wchar_t*)evt->arg2));

      for (int j = 0; j < 2; j++) JS_AddValueRoot(cx, &argv[j]);

      evt->owner->ExecuteEvent(evtName, 2, argv);
      JS_EndRequest(cx);
      for (int j = 0; j < 2; j++) JS_RemoveValueRoot(cx, &argv[j]);
    }
    delete evt->arg1;
    free(evt->arg2);
    free(evt->name);
    delete evt;
    return true;
  }
  if (strcmp(evtName, "chatmsg") == 0 || strcmp(evtName, "chatinput") == 0 || strcmp(evtName, "whispermsg") == 0 || strcmp(evtName, "chatmsgblocker") == 0 ||
      strcmp(evtName, "chatinputblocker") == 0 || strcmp(evtName, "whispermsgblocker") == 0) {
    bool block = false;
    if (!clearList) {
      jsval* argv = new jsval[2];
      JS_BeginRequest(cx);
      argv[0] = (STRING_TO_JSVAL(JS_NewStringCopyZ(cx, (char*)evt->arg1)));
      argv[1] = (STRING_TO_JSVAL(JS_NewUCStringCopyZ(cx, (wchar_t*)evt->arg2)));

      for (int j = 0; j < 2; j++) JS_AddValueRoot(cx, &argv[j]);

      evt->owner->ExecuteEvent(evtName, 2, argv, &block);
      JS_EndRequest(cx);
      for (int j = 0; j < 2; j++) JS_RemoveValueRoot(cx, &argv[j]);
    }
    if (strcmp(evtName, "chatmsgblocker") == 0 || strcmp(evtName, "chatinputblocker") == 0 || strcmp(evtName, "whispermsgblocker") == 0) {
      *(DWORD*)evt->arg4 = block;
      SetEvent(Vars.eventSignal);
    } else {
      free(evt->arg1);
      free(evt->arg2);
      free(evt->name);
      delete evt;
    }
    return true;
  }
  if (strcmp(evtName, "mousemove") == 0 || strcmp(evtName, "ScreenHookHover") == 0) {
    if (!clearList) {
      jsval* argv = new jsval[2];
      JS_BeginRequest(cx);
      argv[0] = JS_NumberValue(*(DWORD*)evt->arg1);
      argv[1] = JS_NumberValue(*(DWORD*)evt->arg2);

      for (int j = 0; j < 2; j++) JS_AddValueRoot(cx, &argv[j]);

      if (strcmp(evtName, "ScreenHookHover") == 0) {
        evt->owner->ExecuteEvent(evtName, evt->argc + 1, argv);
      } else {
        evt->owner->ExecuteEvent(evtName, 2, argv);
      }
      JS_EndRequest(cx);
      for (int j = 0; j < 2; j++) JS_RemoveValueRoot(cx, &argv[j]);
    }
    delete evt->arg1;
    delete evt->arg2;
    free(evt->name);
    delete evt;
    return true;
  }
  if (strcmp(evtName, "mouseclick") == 0) {
    if (!clearList) {
      jsval* argv = new jsval[4];
      JS_BeginRequest(cx);
      argv[0] = JS_NumberValue(*(DWORD*)evt->arg1);
      argv[1] = JS_NumberValue(*(DWORD*)evt->arg2);
      argv[2] = JS_NumberValue(*(DWORD*)evt->arg3);
      argv[3] = JS_NumberValue(*(DWORD*)evt->arg4);

      for (uint j = 0; j < evt->argc; j++) JS_AddValueRoot(cx, &argv[j]);

      evt->owner->ExecuteEvent(evtName, 4, argv);
      JS_EndRequest(cx);
      for (int j = 0; j < 4; j++) JS_RemoveValueRoot(cx, &argv[j]);
    }
    delete evt->arg1;
    delete evt->arg2;
    delete evt->arg3;
    delete evt->arg4;
    free(evt->name);
    delete evt;
    return true;
  }
  if (strcmp(evtName, "keyup") == 0 || strcmp(evtName, "keydownblocker") == 0 || strcmp(evtName, "keydown") == 0 || strcmp(evtName, "memana") == 0 ||
      strcmp(evtName, "melife") == 0 || strcmp(evtName, "playerassign") == 0) {
    bool block = false;
    if (!clearList) {
      jsval* argv = new jsval[1];
      JS_BeginRequest(cx);
      argv[0] = JS_NumberValue(*(DWORD*)evt->arg1);
      JS_AddValueRoot(cx, &argv[0]);
      evt->owner->ExecuteEvent(evtName, 1, argv, &block);
      JS_EndRequest(cx);
      JS_RemoveValueRoot(cx, &argv[0]);
    }
    if (strcmp(evtName, "keydownblocker") == 0) {
      *(DWORD*)evt->arg4 = block;
      SetEvent(Vars.eventSignal);
    } else {
      delete evt->arg1;
      free(evt->name);
      delete evt;
    }
    return true;
  }
  if (strcmp(evtName, "ScreenHookClick") == 0) {
    bool block = false;
    if (!clearList) {
      jsval* argv = new jsval[3];
      JS_BeginRequest(cx);
      argv[0] = JS_NumberValue(*(DWORD*)evt->arg1);
      argv[1] = JS_NumberValue(*(DWORD*)evt->arg2);
      argv[2] = JS_NumberValue(*(DWORD*)evt->arg3);
      for (int j = 0; j < 3; j++) JS_AddValueRoot(cx, &argv[j]);

      jsval rval;
      // diffrent function source for hooks
      for (FunctionList::iterator it = evt->functions.begin(); it != evt->functions.end(); it++) {
        JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), *(*it)->value(), 3, argv, &rval);
        block |= static_cast<bool>(JSVAL_IS_BOOLEAN(rval) && JSVAL_TO_BOOLEAN(rval));
      }

      JS_EndRequest(cx);
      for (int j = 0; j < 3; j++) JS_RemoveValueRoot(cx, &argv[j]);
    }
    *(DWORD*)evt->arg4 = block;
    SetEvent(Vars.eventSignal);

    return true;
  }
  if (strcmp(evtName, "Command") == 0) {
    wchar_t* cmd = (wchar_t*)evt->arg1;
    std::wstring test;

    test.append(L"try{ ");
    test.append(cmd);
    test.append(L" } catch (error){print(error)}");
    JS_BeginRequest(cx);
    jsval rval;

    if (JS_EvaluateUCScript(cx, JS_GetGlobalObject(cx), test.data(), test.length(), "Command Line", 0, &rval)) {
      if (!JSVAL_IS_NULL(rval) && !JSVAL_IS_VOID(rval)) {
        JS_ConvertValue(cx, rval, JSTYPE_STRING, &rval);
        const wchar_t* text = JS_GetStringCharsZ(cx, JS_ValueToString(cx, rval));
        Print(L"%s", text);
      }
    }
    JS_EndRequest(cx);
    free(evt->arg1);
    free(evt->name);
    delete evt;
  }
  if (strcmp(evtName, "scriptmsg") == 0) {
    if (!clearList) {
      DWORD* argc = (DWORD*)evt->arg1;
      JS_BeginRequest(cx);
      jsval* argv = new jsval[*argc];
      for (uint i = 0; i < *argc; i++) evt->argv[i]->read(cx, &argv[i]);

      for (uint j = 0; j < *argc; j++) JS_AddValueRoot(cx, &argv[j]);

      evt->owner->ExecuteEvent(evtName, *argc, argv);
      JS_EndRequest(cx);

      for (uint j = 0; j < *argc; j++) JS_RemoveValueRoot(cx, &argv[j]);
    }
    for (uint i = 0; i < evt->argc; i++) {
      evt->argv[i]->clear();
      delete evt->argv[i];
    }
    delete evt->arg1;
    free(evt->name);
    delete evt;
    return true;
  }
  if (strcmp(evtName, "gamepacket") == 0 || strcmp(evtName, "gamepacketsent") == 0 || strcmp(evtName, "realmpacket") == 0) {
    bool block = false;
    if (!clearList) {
      BYTE* help = (BYTE*)evt->arg1;
      DWORD* size = (DWORD*)evt->arg2;
      //  DWORD* argc = (DWORD*)1;
      JS_BeginRequest(cx);

      JSObject* arr = JS_NewUint8Array(cx, *size);
      // JSObject* arr = JS_NewArrayObject(cx, 0, NULL);

      JS_AddRoot(cx, &arr);
      for (uint i = 0; i < *size; i++) {
        jsval jsarr = UINT_TO_JSVAL(help[i]);
        JS_SetElement(cx, arr, i, &jsarr);
      }
      jsval argv = OBJECT_TO_JSVAL(arr);
      // evt->argv[0]->read(cx, &argv[0]);
      // JS_AddValueRoot(cx, &argv[0]);

      evt->owner->ExecuteEvent(evtName, 1, &argv, &block);
      *(DWORD*)evt->arg4 = block;
      JS_RemoveRoot(cx, &arr);
      SetEvent(Vars.eventSignal);
      JS_EndRequest(cx);
      //	for(int j = 0 ; j < *argc; j++)
      //	JS_RemoveValueRoot(cx, &argv[j]);
    }

    // delete evt->arg1;
    // delete evt->arg2;

    return true;
  }
  if (strcmp(evtName, "setTimeout") == 0 || strcmp(evtName, "setInterval") == 0) {
    if (!clearList) {
      JS_BeginRequest(cx);
      jsval dummy;
      evt->owner->ExecuteEvent(evtName, 0, &dummy);
      JS_EndRequest(cx);
    }

    if (strcmp(evtName, "setTimeout") == 0) {
      sScriptEngine->RemoveDelayedEvent(*(DWORD*)evt->arg1);
    }

    return true;
  }
  if (strcmp(evtName, "DisposeMe") == 0) {
    sScriptEngine->DisposeScript(evt->owner);
  }

  return true;
}