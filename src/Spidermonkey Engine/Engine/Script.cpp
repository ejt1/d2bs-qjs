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

Script::Script(const wchar_t* file, ScriptState state, uint argc, JSAutoStructuredCloneBuffer** argv)
    : context(NULL),
      globalObject(NULL),
      scriptObject(NULL),
      script(NULL),
      execCount(0),
      isAborted(false),
      isPaused(false),
      isReallyPaused(false),
      scriptState(state),
      threadHandle(INVALID_HANDLE_VALUE),
      threadId(0),
      argc(argc),
      argv(argv) {
  InitializeCriticalSection(&lock);
  // moved the runtime initilization to thread start
  LastGC = 0;
  hasActiveCX = false;
  eventSignal = CreateEvent(nullptr, true, false, nullptr);

  if (scriptState == Command && wcslen(file) < 1) {
    fileName = std::wstring(L"Command Line");
  } else {
    if (_waccess(file, 0) != 0) {
      DEBUG_LOG(file);

      throw std::exception("File not found");
    }

    wchar_t* tmpName = _wcsdup(file);
    if (!tmpName)
      throw std::exception("Could not dup filename");

    _wcslwr_s(tmpName, wcslen(file) + 1);
    fileName = std::wstring(tmpName);
    replace(fileName.begin(), fileName.end(), L'/', L'\\');
    free(tmpName);
  }
}

Script::~Script(void) {
  isAborted = true;
  hasActiveCX = false;
  // JS_SetPendingException(context, JSVAL_NULL);
  if (JS_IsInRequest(runtime))
    JS_EndRequest(context);

  EnterCriticalSection(&lock);
  //    JS_SetRuntimeThread(rt);
  JS_DestroyContext(context);
  // JS_ClearRuntimeThread(rt);
  JS_DestroyRuntime(runtime);

  context = NULL;
  scriptObject = NULL;
  globalObject = NULL;
  script = NULL;
  CloseHandle(eventSignal);
  includes.clear();
  if (threadHandle != INVALID_HANDLE_VALUE)
    CloseHandle(threadHandle);
  LeaveCriticalSection(&lock);
  DeleteCriticalSection(&lock);
}

int Script::GetExecutionCount(void) {
  return execCount;
}

DWORD Script::GetThreadId(void) {
  return (threadHandle == INVALID_HANDLE_VALUE ? -1 : threadId);
}

void Script::RunCommand(const wchar_t* command) {
  // RUNCOMMANDSTRUCT* rcs = new RUNCOMMANDSTRUCT;
  // rcs->script = this;
  // rcs->command = _wcsdup(command);

  if (isAborted) {  // this should never happen -bob
    // RUNCOMMANDSTRUCT* rcs = new RUNCOMMANDSTRUCT;

    // rcs->script = this;
    // rcs->command = _wcsdup(L"delay(1000000);");

    Log(L"Console Aborted HELP!");
  }

  Event* evt = new Event;
  evt->owner = this;
  evt->argc = argc;
  evt->name = _strdup("Command");
  evt->arg1 = _wcsdup(command);
  EnterCriticalSection(&Vars.cEventSection);
  evt->owner->EventList.push_front(evt);
  LeaveCriticalSection(&Vars.cEventSection);
  evt->owner->TriggerOperationCallback();
  SetEvent(evt->owner->eventSignal);
}

bool Script::BeginThread(LPTHREAD_START_ROUTINE ThreadFunc) {
  EnterCriticalSection(&lock);
  DWORD dwExitCode = STILL_ACTIVE;

  if ((!GetExitCodeThread(threadHandle, &dwExitCode) || dwExitCode != STILL_ACTIVE) && (threadHandle = CreateThread(0, 0, ThreadFunc, this, 0, &threadId)) != NULL) {
    LeaveCriticalSection(&lock);
    return true;
  }

  threadHandle = INVALID_HANDLE_VALUE;
  LeaveCriticalSection(&lock);
  return false;
}

void Script::Run(void) {
  try {
    runtime = JS_NewRuntime(Vars.dwMemUsage, JS_NO_HELPER_THREADS);
    // JS_SetNativeStackQuota(runtime, (size_t)50000);
    //  JS_SetRuntimeThread(runtime);
    JS_SetContextCallback(runtime, contextCallback);

    context = JS_NewContext(runtime, 0x800000);
    if (!context)
      throw std::exception("Couldn't create the context");

    JS_SetErrorReporter(context, reportError);
    JS_SetOperationCallback(context, operationCallback);
    JS_SetOptions(context, JSOPTION_STRICT | JSOPTION_VAROBJFIX);
    JS_SetVersion(context, JSVERSION_LATEST);
    //

    JS_SetContextPrivate(context, this);

    JS_BeginRequest(context);

    globalObject = JS_GetGlobalObject(context);
    jsval meVal = JSVAL_VOID;
    if (JS_GetProperty(GetContext(), globalObject, "me", &meVal) != JS_FALSE) {
      JSObject* meObject = JSVAL_TO_OBJECT(meVal);
      me = (myUnit*)JS_GetPrivate(GetContext(), meObject);
    }

    if (scriptState == Command) {
      if (wcslen(Vars.szConsole) > 0) {
        script = JS_CompileFile(context, globalObject, fileName);
      } else {
        char* cmd = "function main() {print('ÿc2D2BSÿc0 :: Started Console'); while (true){delay(10000)};}  ";
        script = JS_CompileScript(context, globalObject, cmd, strlen(cmd), "Command Line", 1);
      }
      JS_AddNamedScriptRoot(context, &script, "console script");
    } else
      script = JS_CompileFile(context, globalObject, fileName);

    if (!script)
      throw std::exception("Couldn't compile the script");

    JS_EndRequest(context);
    // JS_RemoveScriptRoot(context, &script);

  } catch (std::exception&) {
    if (scriptObject)
      JS_RemoveRoot(context, &scriptObject);
    if (context) {
      JS_EndRequest(context);
      JS_DestroyContext(context);
    }
    LeaveCriticalSection(&lock);
    throw;
  }
  // only let the script run if it's not already running
  if (IsRunning())
    return;
  hasActiveCX = true;
  isAborted = false;

  jsval main = INT_TO_JSVAL(1), dummy = INT_TO_JSVAL(1);
  JS_BeginRequest(GetContext());

  // args passed from load
  jsval* argvalue = new jsval[argc];
  for (uint i = 0; i < argc; i++) argv[i]->read(context, &argvalue[i]);

  for (uint j = 0; j < argc; j++) JS_AddValueRoot(context, &argvalue[j]);

  JS_AddValueRoot(GetContext(), &main);
  JS_AddValueRoot(GetContext(), &dummy);
  if (JS_ExecuteScript(GetContext(), globalObject, script, &dummy) != JS_FALSE && JS_GetProperty(GetContext(), globalObject, "main", &main) != JS_FALSE &&
      JSVAL_IS_FUNCTION(GetContext(), main)) {
    JS_CallFunctionValue(GetContext(), globalObject, main, this->argc, argvalue, &dummy);
  }
  JS_RemoveValueRoot(GetContext(), &main);
  JS_RemoveValueRoot(GetContext(), &dummy);
  for (uint j = 0; j < argc; j++) JS_RemoveValueRoot(GetContext(), &argvalue[j]);

  /*for(uint i = 0; i < argc; i++)  //crashes spidermonkey cleans itself up?
  {
          argv[i]->clear();
          delete argv[i];
  }*/

  JS_EndRequest(GetContext());

  execCount++;
  // Stop();
}

void Script::UpdatePlayerGid(void) {
  me->dwUnitId = (D2CLIENT_GetPlayerUnit() == NULL ? NULL : D2CLIENT_GetPlayerUnit()->dwUnitId);
}

void Script::Pause(void) {
  if (!IsAborted() && !IsPaused())
    isPaused = true;
  TriggerOperationCallback();
  SetEvent(eventSignal);
}

void Script::Join() {
  EnterCriticalSection(&lock);
  HANDLE hThread = threadHandle;
  LeaveCriticalSection(&lock);

  if (hThread != INVALID_HANDLE_VALUE)
    WaitForSingleObject(hThread, INFINITE);
}

void Script::Resume(void) {
  if (!IsAborted() && IsPaused())
    isPaused = false;
  TriggerOperationCallback();
  SetEvent(eventSignal);
}

bool Script::IsPaused(void) {
  return isPaused;
}

const wchar_t* Script::GetShortFilename() {
  if (wcscmp(fileName.c_str(), L"Command Line") == 0)
    return fileName.c_str();
  else
    return (fileName.c_str() + wcslen(Vars.szScriptPath) + 1);
}

void Script::Stop(bool force, bool reallyForce) {
  // if we've already stopped, just return
  if (isAborted)
    return;
  EnterCriticalSection(&lock);
  // tell everyone else that the script is aborted FIRST
  isAborted = true;
  isPaused = false;
  isReallyPaused = false;
  if (GetState() != Command) {
    const wchar_t* displayName = fileName.c_str() + wcslen(Vars.szScriptPath) + 1;
    Print(L"Script %s ended", displayName);
  }

  // trigger call back so script ends
  TriggerOperationCallback();
  SetEvent(eventSignal);

  // normal wait: 500ms, forced wait: 300ms, really forced wait: 100ms
  int maxCount = (force ? (reallyForce ? 10 : 30) : 50);
  if (GetCurrentThreadId() != GetThreadId()) {
    for (int i = 0; hasActiveCX == true; i++) {
      // if we pass the time frame, just ignore the wait because the thread will end forcefully anyway
      if (i >= maxCount)
        break;
      Sleep(10);
    }
  }
  LeaveCriticalSection(&lock);
}

bool Script::IsIncluded(const wchar_t* file) {
  uint count = 0;
  wchar_t* fname = _wcsdup(file);
  if (!fname)
    return false;

  _wcslwr_s(fname, wcslen(fname) + 1);
  StringReplace(fname, '/', '\\', wcslen(fname));
  count = includes.count(std::wstring(fname));
  free(fname);

  return !!count;
}

bool Script::Include(const wchar_t* file) {
  // since includes will happen on the same thread, locking here is acceptable
  EnterCriticalSection(&lock);
  wchar_t* fname = _wcsdup(file);
  if (!fname) {
    LeaveCriticalSection(&lock);
    return false;
  }
  _wcslwr_s(fname, wcslen(fname) + 1);
  StringReplace(fname, L'/', L'\\', wcslen(fname));

  // don't invoke the string ctor more than once...
  std::wstring currentFileName = std::wstring(fname);
  // ignore already included, 'in-progress' includes, and self-inclusion
  if (!!includes.count(fname) || !!inProgress.count(fname) || (currentFileName.compare(fileName.c_str()) == 0)) {
    LeaveCriticalSection(&lock);
    free(fname);
    return true;
  }
  bool rval = false;

  JSContext* cx = GetContext();

  JS_BeginRequest(cx);

  JSScript* _script = JS_CompileFile(cx, GetGlobalObject(), fname);
  if (_script) {
    jsval dummy;
    inProgress[fname] = true;
    rval = !!JS_ExecuteScript(cx, GetGlobalObject(), _script, &dummy);
    if (rval)
      includes[fname] = true;
    else
      JS_ReportPendingException(cx);
    inProgress.erase(fname);
    // JS_RemoveRoot(&scriptObj);
  } else
    JS_ReportPendingException(cx);

  JS_EndRequest(cx);
  // JS_RemoveScriptRoot(cx, &script);
  LeaveCriticalSection(&lock);
  free(fname);
  return rval;
}

bool Script::IsRunning(void) {
  return context && !(IsAborted() || IsPaused() || !hasActiveCX);
}

bool Script::IsAborted() {
  return isAborted;
}

bool Script::IsListenerRegistered(const char* evtName) {
  return strlen(evtName) > 0 && functions.count(evtName) > 0;
}

void Script::RegisterEvent(const char* evtName, jsval evtFunc) {
  EnterCriticalSection(&lock);
  if (JSVAL_IS_FUNCTION(context, evtFunc) && strlen(evtName) > 0) {
    AutoRoot* root = new AutoRoot(context, evtFunc);
    functions[evtName].push_back(root);
  }
  LeaveCriticalSection(&lock);
}

bool Script::IsRegisteredEvent(const char* evtName, jsval evtFunc) {
  // nothing can be registered under an empty name
  if (strlen(evtName) < 1)
    return false;

  // if there are no events registered under that name at all, then obviously there
  // can't be a specific one registered under that name
  if (functions.count(evtName) < 1)
    return false;

  for (FunctionList::iterator it = functions[evtName].begin(); it != functions[evtName].end(); it++)
    if (*(*it)->value() == evtFunc)
      return true;

  return false;
}

void Script::UnregisterEvent(const char* evtName, jsval evtFunc) {
  if (strlen(evtName) < 1)
    return;

  EnterCriticalSection(&lock);
  AutoRoot* func = NULL;
  for (FunctionList::iterator it = functions[evtName].begin(); it != functions[evtName].end(); it++) {
    if (*(*it)->value() == evtFunc) {
      func = *it;
      break;
    }
  }
  functions[evtName].remove(func);
  // func->Release();
  delete func;

  // Remove event completely if there are no listeners for it.
  if (functions.count(evtName) > 0 && functions[evtName].size() == 0)
    functions.erase(evtName);

  LeaveCriticalSection(&lock);
}

void Script::ClearEvent(const char* evtName) {
  EnterCriticalSection(&lock);
  for (FunctionList::iterator it = functions[evtName].begin(); it != functions[evtName].end(); it++) {
    AutoRoot* func = *it;
    func->Release();
    delete func;
  }
  functions[evtName].clear();
  LeaveCriticalSection(&lock);
}

void Script::ClearAllEvents(void) {
  EnterCriticalSection(&lock);
  for (FunctionMap::iterator it = functions.begin(); it != functions.end(); it++) ClearEvent(it->first.c_str());
  functions.clear();
  LeaveCriticalSection(&lock);
}
void Script::FireEvent(Event* evt) {
  // EnterCriticalSection(&sScriptEngine->lock);
  EnterCriticalSection(&Vars.cEventSection);
  evt->owner->EventList.push_front(evt);
  LeaveCriticalSection(&Vars.cEventSection);

  if (evt->owner && evt->owner->IsRunning()) {
    evt->owner->TriggerOperationCallback();
  }
  SetEvent(eventSignal);
  // LeaveCriticalSection(&sScriptEngine->lock);
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

  // moved this to delay. operation callback isnt a good spot for this anymore
  // event heavy stuff like shop bot would trigger gc alot while normal playing would not.
  /*if (callBackCount % 30 == 0){
  JS_GC(JS_GetRuntime(cx));
  callBackCount = 0;
  }*/

  HWND debug_wnd = NULL;  // FindWindow(NULL, "JavaZone");
  if (debug_wnd && D2GFX_GetHwnd() != debug_wnd) {
    JS_BeginRequest(cx);
    JS::Value err;
    if (JS_CallFunctionName(cx, JS_GetGlobalObject(cx), "Error", 0, NULL, &err)) {
      JS::Value stack;
      if (JS_GetProperty(cx, JSVAL_TO_OBJECT(err), "stack", &stack)) {
        char str[1024];
        int l = 0;
        COPYDATASTRUCT aCopy = {164344, 0, str};
        if (JSVAL_IS_STRING(stack)) {
          l = JS_EncodeStringToBuffer(cx, JSVAL_TO_STRING(stack), str, 1024);
          if (l) {
            str[l] = 0;
            aCopy.cbData = l + 1;
            SendMessage(debug_wnd, WM_COPYDATA, (WPARAM)D2GFX_GetHwnd(), (LPARAM)&aCopy);
            // Print("%s", str);
          }
        } else if (JSVAL_IS_OBJECT(stack)) {
          if (JS_CallFunctionName(cx, JSVAL_TO_OBJECT(stack), "ToString", 0, NULL, &err)) {
            l = JS_EncodeStringToBuffer(cx, JSVAL_TO_STRING(err), str, 1024);
            if (l) {
              str[l] = 0;
              aCopy.cbData = l + 1;
              SendMessage(debug_wnd, WM_COPYDATA, (WPARAM)D2GFX_GetHwnd(), (LPARAM)&aCopy);
              // Print("%s", str);
            }
          }
        }
      }
    }
    JS_EndRequest(cx);
  }
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
    while (script->EventList.size() > 0 && !!!(JSBool)(script->IsAborted() || ((script->GetState() == InGame) && ClientState() == ClientStateMenu))) {
      EnterCriticalSection(&Vars.cEventSection);
      Event* evt = script->EventList.back();
      script->EventList.pop_back();
      LeaveCriticalSection(&Vars.cEventSection);
      ExecScriptEvent(evt, false);
    }
    return !!!(JSBool)(script->IsAborted() || ((script->GetState() == InGame) && ClientState() == ClientStateMenu));
  } else {
    return false;
  }
}

JSBool contextCallback(JSContext* cx, uint contextOp) {
  if (contextOp == JSCONTEXT_NEW) {
    JS_BeginRequest(cx);

    JS_SetErrorReporter(cx, reportError);
    JS_SetOperationCallback(cx, operationCallback);

    JS_SetVersion(cx, JSVERSION_LATEST);
    JS_SetOptions(cx, JSOPTION_METHODJIT | JSOPTION_TYPE_INFERENCE | JSOPTION_ION | JSOPTION_VAROBJFIX | JSOPTION_ASMJS | JSOPTION_STRICT);

    // JS_SetGCZeal(cx, 2, 1);
    JSObject* globalObject = JS_NewGlobalObject(cx, &global_obj, NULL);
    JS_SetGCParameter(JS_GetRuntime(cx), JSGC_MODE, 2);

    if (JS_InitStandardClasses(cx, globalObject) == JS_FALSE)
      return JS_FALSE;
    if (JS_DefineFunctions(cx, globalObject, global_funcs) == JS_FALSE)
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
      sScriptEngine->InitClass(cx, globalObject, entry->classp, entry->methods, entry->properties, entry->static_methods, entry->static_properties);

    JSObject* meObject = BuildObject(cx, &unit_class, unit_methods, me_props, lpUnit);
    if (!meObject)
      return JS_FALSE;

    if (JS_DefineProperty(cx, globalObject, "me", OBJECT_TO_JSVAL(meObject), NULL, NULL, JSPROP_PERMANENT_VAR) == JS_FALSE)
      return JS_FALSE;

#define DEFCONST(vp) sScriptEngine->DefineConstant(cx, globalObject, #vp, vp)
    DEFCONST(FILE_READ);
    DEFCONST(FILE_WRITE);
    DEFCONST(FILE_APPEND);
#undef DEFCONST

    JS_EndRequest(cx);
  }
  if (contextOp == JSCONTEXT_DESTROY) {
    Script* script = (Script*)JS_GetContextPrivate(cx);
    script->hasActiveCX = false;
    while (script->EventList.size() > 0) {
      EnterCriticalSection(&Vars.cEventSection);
      Event* evt = script->EventList.back();
      script->EventList.pop_back();
      LeaveCriticalSection(&Vars.cEventSection);
      ExecScriptEvent(evt, true);  // clean list and pop events
    }

    script->ClearAllEvents();
    Genhook::Clean(script);
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
      jsval rval;
      bool block;
      for (FunctionList::iterator it = evt->owner->functions[evtName].begin(); it != evt->owner->functions[evtName].end(); it++) {
        JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), *(*it)->value(), 4, argv, &rval);
        block |= static_cast<bool>(JSVAL_IS_BOOLEAN(rval) && JSVAL_TO_BOOLEAN(rval));
      }
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

      jsval rval;
      for (FunctionList::iterator it = evt->owner->functions[evtName].begin(); it != evt->owner->functions[evtName].end(); it++) {
        JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), *(*it)->value(), 5, argv, &rval);
      }
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

      jsval rval;
      for (FunctionList::iterator it = evt->owner->functions[evtName].begin(); it != evt->owner->functions[evtName].end(); it++) {
        JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), *(*it)->value(), 2, argv, &rval);
      }
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

      jsval rval;
      for (FunctionList::iterator it = evt->owner->functions[evtName].begin(); it != evt->owner->functions[evtName].end(); it++) {
        JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), *(*it)->value(), 2, argv, &rval);
        block |= static_cast<bool>(JSVAL_IS_BOOLEAN(rval) && JSVAL_TO_BOOLEAN(rval));
      }
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

      jsval rval;
      if (strcmp(evtName, "ScreenHookHover") == 0) {
        for (FunctionList::iterator it = evt->functions.begin(); it != evt->functions.end(); it++)
          JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), *(*it)->value(), evt->argc + 1, argv, &rval);
      } else {
        for (FunctionList::iterator it = evt->owner->functions[evtName].begin(); it != evt->owner->functions[evtName].end(); it++)
          JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), *(*it)->value(), 2, argv, &rval);
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

      jsval rval;
      for (FunctionList::iterator it = evt->owner->functions[evtName].begin(); it != evt->owner->functions[evtName].end(); it++) {
        JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), *(*it)->value(), 4, argv, &rval);
      }
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
      jsval rval;
      for (FunctionList::iterator it = evt->owner->functions[evtName].begin(); it != evt->owner->functions[evtName].end(); it++) {
        JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), *(*it)->value(), 1, argv, &rval);
        block |= static_cast<bool>(JSVAL_IS_BOOLEAN(rval) && JSVAL_TO_BOOLEAN(rval));
      }
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

      jsval rval;
      for (FunctionList::iterator it = evt->owner->functions[evtName].begin(); it != evt->owner->functions[evtName].end(); it++) {
        JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), *(*it)->value(), *argc, argv, &rval);
      }
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

      jsval rval;
      for (FunctionList::iterator it = evt->owner->functions[evtName].begin(); it != evt->owner->functions[evtName].end(); it++) {
        JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), *(*it)->value(), 1, &argv, &rval);
        block |= static_cast<bool>(JSVAL_IS_BOOLEAN(rval) && JSVAL_TO_BOOLEAN(rval));
      }
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
      jsval rval;
      for (FunctionList::iterator it = evt->owner->functions[evtName].begin(); it != evt->owner->functions[evtName].end(); it++) {
        JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), *(*it)->value(), 0, &rval, &rval);
      }
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