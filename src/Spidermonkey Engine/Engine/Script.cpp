#include <io.h>
#include <algorithm>

#include "Script.h"
#include "Core.h"
#include "Constants.h"
#include "D2Ptrs.h"
#include "JSUnit.h"
#include "Helpers.h"
#include "ScriptEngine.h"
#include "Engine.h"
#include "JSGlobalFuncs.h"
#include "Console.h"

#include <chrono>

Script::Script(const wchar_t* file, ScriptMode mode, uint argc, JSAutoStructuredCloneBuffer** argv)
    : m_runtime(NULL),
      m_context(NULL),
      m_globalObject(NULL),
      m_script(NULL),
      m_isPaused(false),
      m_isReallyPaused(false),
      m_scriptMode(mode),
      m_scriptState(kScriptStateUninitialized),
      m_threadHandle(INVALID_HANDLE_VALUE),
      m_threadId(0),
      m_argc(argc),
      m_argv(argv) {
  InitializeCriticalSection(&m_lock);
  // moved the runtime initilization to thread start
  m_LastGC = 0;
  m_hasActiveCX = false;
  m_eventSignal = CreateEvent(nullptr, true, false, nullptr);

  if (m_scriptMode == kScriptModeCommand && wcslen(file) < 1) {
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
  m_globalObject = NULL;
  m_script = NULL;
  CloseHandle(m_eventSignal);
  m_includes.clear();
  if (m_threadHandle != INVALID_HANDLE_VALUE)
    CloseHandle(m_threadHandle);
  LeaveCriticalSection(&m_lock);
  DeleteCriticalSection(&m_lock);
}

bool Script::Start() {
  EnterCriticalSection(&m_lock);
  DWORD dwExitCode = STILL_ACTIVE;

  if ((!GetExitCodeThread(m_threadHandle, &dwExitCode) || dwExitCode != STILL_ACTIVE) &&
      (m_threadHandle = CreateThread(0, 0, ScriptThread, this, 0, &m_threadId)) != NULL) {
    LeaveCriticalSection(&m_lock);
    return true;
  }

  m_threadHandle = INVALID_HANDLE_VALUE;
  LeaveCriticalSection(&m_lock);
  return false;
}

void Script::Stop(bool force, bool reallyForce) {
  // make sure the script is actually running
  if (m_scriptState != kScriptStateRunning)
    return;

  EnterCriticalSection(&m_lock);
  // tell everyone else that the script is aborted FIRST
  m_scriptState = kScriptStateRequestStop;
  m_isPaused = false;
  m_isReallyPaused = false;
  if (m_scriptMode != kScriptModeCommand) {
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

void Script::Run(void) {
  if (Initialize()) {
    RunMain();
  }
  Cleanup();
}

void Script::Join() {
  // TODO(ejt): investigate if locking here is necessary
  EnterCriticalSection(&m_lock);
  HANDLE hThread = m_threadHandle;
  LeaveCriticalSection(&m_lock);

  if (hThread != INVALID_HANDLE_VALUE)
    WaitForSingleObject(hThread, INFINITE);
}

void Script::Pause(void) {
  if (!(m_scriptState == kScriptStateRunning) && !m_isPaused)
    m_isPaused = true;
  TriggerOperationCallback();
  SetEvent(m_eventSignal);
}

void Script::Resume(void) {
  if (!(m_scriptState == kScriptStateRunning) && m_isPaused)
    m_isPaused = false;
  TriggerOperationCallback();
  SetEvent(m_eventSignal);
}

bool Script::IsUninitialized() {
  return m_scriptState == kScriptStateUninitialized;
}

bool Script::IsRunning(void) {
  return m_context && !(m_scriptState != kScriptStateRunning || m_isPaused || !m_hasActiveCX);
}

bool Script::IsAborted() {
  return m_scriptState == kScriptStateStopped;
}

void Script::RunCommand(const wchar_t* command) {
  Event* evt = new Event;
  evt->argc = m_argc;
  evt->name = "Command";
  evt->arg1 = _wcsdup(command);
  FireEvent(evt);
}

const wchar_t* Script::GetShortFilename() {
  if (wcscmp(m_fileName.c_str(), L"Command Line") == 0)
    return m_fileName.c_str();
  else
    return (m_fileName.c_str() + wcslen(Vars.szScriptPath) + 1);
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

  JSAutoRequest request(m_context);

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
  m_EventList.push_front(evt);
  LeaveCriticalSection(&Vars.cEventSection);

  if (IsRunning()) {
    TriggerOperationCallback();
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
    HandleEvent(evt, true);  // clean list and pop events
  }

  ClearAllEvents();
}

void Script::BlockThread(DWORD delay) {
  DWORD start = GetTickCount();
  int amt = delay - (GetTickCount() - start);
  if (m_scriptState == kScriptStateStopped) {
    return;
  }

  while (amt > 0) {  // had a script deadlock here, make sure were positve with amt
    WaitForSingleObjectEx(m_eventSignal, amt, true);
    ResetEvent(m_eventSignal);
    if (!ProcessAllEvents()) {
      break;
    }

    if (JS_GetGCParameter(m_runtime, JSGC_BYTES) - m_LastGC > 524288)  // gc every .5 mb
    {
      JS_GC(m_runtime);
      m_LastGC = JS_GetGCParameter(m_runtime, JSGC_BYTES);
    }

    amt = delay - (GetTickCount() - start);
  }
}

void Script::ExecuteEvent(char* evtName, int argc, const jsval* argv, bool* block) {
  for (const auto& root : m_functions[evtName]) {
    jsval rval;
    JS_CallFunctionValue(m_context, JS_GetGlobalObject(m_context), *root->value(), argc, const_cast<JS::Value*>(argv), &rval);
    if (block) {
      *block |= static_cast<bool>(JSVAL_IS_BOOLEAN(rval) && JSVAL_TO_BOOLEAN(rval));
    }
  }
}

void Script::ExecuteEvent(char* evtName, const JS::AutoValueVector& args, bool* block) {
  ExecuteEvent(evtName, args.length(), args.begin(), block);
}

void Script::OnDestroyContext() {
  m_hasActiveCX = false;
  ClearEventList();
  Genhook::Clean(this);
}

bool Script::HandleEvent(Event* evt, bool clearList) {
  JSContext* cx = m_context;
  char* evtName = (char*)evt->name.c_str();

  if (strcmp(evtName, "itemaction") == 0) {
    if (!clearList) {
      DWORD* gid = (DWORD*)evt->arg1;
      char* code = (char*)evt->arg2;
      DWORD* mode = (DWORD*)evt->arg3;
      bool* global = (bool*)evt->arg4;

      JSAutoRequest request(m_context);
      JS::AutoValueVector args(m_context);
      args.append(JS_NumberValue(*gid));
      args.append(JS_NumberValue(*mode));
      args.append(STRING_TO_JSVAL(JS_NewStringCopyZ(cx, code)));
      args.append(BOOLEAN_TO_JSVAL(*global));
      ExecuteEvent(evtName, args);
    }
    delete evt->arg1;
    free(evt->arg2);
    delete evt->arg3;
    delete evt->arg4;
    delete evt;
    return true;
  }
  if (strcmp(evtName, "gameevent") == 0) {
    if (!clearList) {
      JSAutoRequest request(m_context);
      JS::AutoValueVector args(m_context);
      args.append(JS_NumberValue(*(BYTE*)evt->arg1));
      args.append(JS_NumberValue(*(DWORD*)evt->arg2));
      args.append(JS_NumberValue(*(DWORD*)evt->arg3));
      args.append(STRING_TO_JSVAL(JS_NewStringCopyZ(cx, (char*)evt->arg4)));
      args.append(STRING_TO_JSVAL(JS_NewUCStringCopyZ(cx, (wchar_t*)evt->arg5)));

      ExecuteEvent(evtName, args);
    }
    delete evt->arg1;
    delete evt->arg2;
    delete evt->arg3;
    free(evt->arg4);
    free(evt->arg5);
    delete evt;
    return true;
  }
  if (strcmp(evtName, "copydata") == 0) {
    if (!clearList) {
      JSAutoRequest request(m_context);
      JS::AutoValueVector args(m_context);
      args.append(JS_NumberValue(*(DWORD*)evt->arg1));
      args.append(STRING_TO_JSVAL(JS_NewUCStringCopyZ(cx, (wchar_t*)evt->arg2)));

      ExecuteEvent(evtName, args);
    }
    delete evt->arg1;
    free(evt->arg2);
    delete evt;
    return true;
  }
  if (strcmp(evtName, "chatmsg") == 0 || strcmp(evtName, "chatinput") == 0 || strcmp(evtName, "whispermsg") == 0 || strcmp(evtName, "chatmsgblocker") == 0 ||
      strcmp(evtName, "chatinputblocker") == 0 || strcmp(evtName, "whispermsgblocker") == 0) {
    bool block = false;
    if (!clearList) {
      JSAutoRequest request(m_context);
      JS::AutoValueVector args(m_context);
      args.append(STRING_TO_JSVAL(JS_NewStringCopyZ(cx, (char*)evt->arg1)));
      args.append(STRING_TO_JSVAL(JS_NewUCStringCopyZ(cx, (wchar_t*)evt->arg2)));

      ExecuteEvent(evtName, args, &block);
    }
    if (strcmp(evtName, "chatmsgblocker") == 0 || strcmp(evtName, "chatinputblocker") == 0 || strcmp(evtName, "whispermsgblocker") == 0) {
      *(DWORD*)evt->arg4 = block;
      SetEvent(Vars.eventSignal);
    } else {
      free(evt->arg1);
      free(evt->arg2);
      delete evt;
    }
    return true;
  }
  if (strcmp(evtName, "mousemove") == 0 || strcmp(evtName, "ScreenHookHover") == 0) {
    if (!clearList) {
      JSAutoRequest request(m_context);
      JS::AutoValueVector args(m_context);
      args.append(JS_NumberValue(*(DWORD*)evt->arg1));
      args.append(JS_NumberValue(*(DWORD*)evt->arg2));

      if (strcmp(evtName, "ScreenHookHover") == 0) {
        ExecuteEvent(evtName, evt->argc + 1, args.begin());
      } else {
        ExecuteEvent(evtName, args);
      }
    }
    delete evt->arg1;
    delete evt->arg2;
    delete evt;
    return true;
  }
  if (strcmp(evtName, "mouseclick") == 0) {
    if (!clearList) {
      JSAutoRequest request(m_context);
      JS::AutoValueVector args(m_context);
      args.append(JS_NumberValue(*(DWORD*)evt->arg1));
      args.append(JS_NumberValue(*(DWORD*)evt->arg2));
      args.append(JS_NumberValue(*(DWORD*)evt->arg3));
      args.append(JS_NumberValue(*(DWORD*)evt->arg4));

      ExecuteEvent(evtName, args);
    }
    delete evt->arg1;
    delete evt->arg2;
    delete evt->arg3;
    delete evt->arg4;
    delete evt;
    return true;
  }
  if (strcmp(evtName, "keyup") == 0 || strcmp(evtName, "keydownblocker") == 0 || strcmp(evtName, "keydown") == 0 || strcmp(evtName, "memana") == 0 ||
      strcmp(evtName, "melife") == 0 || strcmp(evtName, "playerassign") == 0) {
    bool block = false;
    if (!clearList) {
      JSAutoRequest request(m_context);
      JS::AutoValueVector args(m_context);
      args.append(JS_NumberValue(*(DWORD*)evt->arg1));
      ExecuteEvent(evtName, args, &block);
    }
    if (strcmp(evtName, "keydownblocker") == 0) {
      *(DWORD*)evt->arg4 = block;
      SetEvent(Vars.eventSignal);
    } else {
      delete evt->arg1;
      delete evt;
    }
    return true;
  }
  if (strcmp(evtName, "ScreenHookClick") == 0) {
    bool block = false;
    if (!clearList) {
      JSAutoRequest request(m_context);
      JS::AutoValueVector args(m_context);
      args.append(JS_NumberValue(*(DWORD*)evt->arg1));
      args.append(JS_NumberValue(*(DWORD*)evt->arg2));
      args.append(JS_NumberValue(*(DWORD*)evt->arg3));

      jsval rval;
      // diffrent function source for hooks
      for (FunctionList::iterator it = evt->functions.begin(); it != evt->functions.end(); it++) {
        JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), *(*it)->value(), args.length(), args.begin(), &rval);
        block |= static_cast<bool>(JSVAL_IS_BOOLEAN(rval) && JSVAL_TO_BOOLEAN(rval));
      }
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
    JSAutoRequest request(m_context);
    jsval rval;

    if (JS_EvaluateUCScript(cx, JS_GetGlobalObject(cx), test.data(), test.length(), "Command Line", 0, &rval)) {
      if (!JSVAL_IS_NULL(rval) && !JSVAL_IS_VOID(rval)) {
        JS_ConvertValue(cx, rval, JSTYPE_STRING, &rval);
        const wchar_t* text = JS_GetStringCharsZ(cx, JS_ValueToString(cx, rval));
        Print(L"%s", text);
      }
    }
    free(evt->arg1);
    delete evt;
  }
  if (strcmp(evtName, "scriptmsg") == 0) {
    if (!clearList) {
      DWORD* argc = (DWORD*)evt->arg1;
      JSAutoRequest request(m_context);
      JS::AutoValueVector args(m_context);

      for (uint i = 0; i < *argc; ++i) {
        jsval v;
        evt->argv[i]->read(m_context, &v);
        args.append(v);
      }

      ExecuteEvent(evtName, args);
    }
    for (uint i = 0; i < evt->argc; i++) {
      evt->argv[i]->clear();
      delete evt->argv[i];
    }
    delete evt->arg1;
    delete evt;
    return true;
  }
  if (strcmp(evtName, "gamepacket") == 0 || strcmp(evtName, "gamepacketsent") == 0 || strcmp(evtName, "realmpacket") == 0) {
    bool block = false;
    if (!clearList) {
      BYTE* help = (BYTE*)evt->arg1;
      DWORD* size = (DWORD*)evt->arg2;
      //  DWORD* argc = (DWORD*)1;
      JSAutoRequest request(m_context);

      JS::RootedValue arr(m_context, JS::ObjectOrNullValue(JS_NewUint8Array(cx, *size)));
      for (uint i = 0; i < *size; i++) {
        jsval jsarr = UINT_TO_JSVAL(help[i]);
        JS_SetElement(cx, arr.toObjectOrNull(), i, &jsarr);
      }
      
      ExecuteEvent(evtName, 1, arr.address(), &block);
      *(DWORD*)evt->arg4 = block;
      SetEvent(Vars.eventSignal);
    }

    // delete evt->arg1;
    // delete evt->arg2;

    return true;
  }
  if (strcmp(evtName, "setTimeout") == 0 || strcmp(evtName, "setInterval") == 0) {
    if (!clearList) {
      JSAutoRequest request(m_context);
      jsval dummy;
      ExecuteEvent(evtName, 0, &dummy);
    }

    //if (strcmp(evtName, "setTimeout") == 0) {
    //  sScriptEngine->RemoveDelayedEvent(*(DWORD*)evt->arg1);
    //}

    return true;
  }
  if (strcmp(evtName, "DisposeMe") == 0) {
    sScriptEngine->DisposeScript(this);
  }

  return true;
}

bool Script::Initialize() {
  m_runtime = JS_NewRuntime(Vars.dwMemUsage, JS_NO_HELPER_THREADS);
  JS_SetContextCallback(m_runtime, contextCallback);

  m_context = JS_NewContext(m_runtime, 0x800000);
  if (!m_context) {
    Log(L"Couldn't create the context");
    return false;
  }
  JS_SetContextPrivate(m_context, this);
  JS_SetErrorReporter(m_context, reportError);
  JS_SetOperationCallback(m_context, InterruptHandler);
  JS_SetOptions(m_context, JSOPTION_STRICT | JSOPTION_VAROBJFIX);
  JS_SetVersion(m_context, JSVERSION_LATEST);

  JSAutoRequest request(m_context);

  m_globalObject = JS_NewGlobalObject(m_context, &global_obj, NULL);
  JS_SetGCParameter(JS_GetRuntime(m_context), JSGC_MODE, 2);

  if (JS_InitStandardClasses(m_context, m_globalObject) == JS_FALSE) {
    Log(L"failed to init standard classes");
    return false;
  }
  if (JS_DefineFunctions(m_context, m_globalObject, global_funcs) == JS_FALSE) {
    Log(L"failed to define functions globalObject");
    return false;
  }
  for (JSClassSpec* entry = global_classes; entry->classp != NULL; entry++) {
    if (!JS_InitClass(m_context, m_globalObject, NULL, entry->classp, entry->classp->construct, 0, entry->properties, entry->methods, entry->static_properties,
                      entry->static_methods)) {
      Log(L"Couldn't initialize the class");
      return false;
    }
  }

#define DEFCONST(vp)                                                                                            \
  if (!JS_DefineProperty(m_context, m_globalObject, #vp, INT_TO_JSVAL(vp), NULL, NULL, JSPROP_PERMANENT_VAR)) { \
    Log(L"Couldn't initialize the constant");                                                                   \
    return false;                                                                                               \
  }
  DEFCONST(FILE_READ);
  DEFCONST(FILE_WRITE);
  DEFCONST(FILE_APPEND);
#undef DEFCONST

  // define 'me' property
  m_me = new myUnit;
  memset(m_me, NULL, sizeof(myUnit));

  UnitAny* player = D2CLIENT_GetPlayerUnit();
  m_me->dwMode = (DWORD)-1;
  m_me->dwClassId = (DWORD)-1;
  m_me->dwType = UNIT_PLAYER;
  m_me->dwUnitId = player ? player->dwUnitId : NULL;
  m_me->_dwPrivateType = PRIVATE_UNIT;

  JSObject* meObject = BuildObject(m_context, &unit_class, unit_methods, me_props, m_me);
  if (!meObject) {
    Log(L"failed to build object 'me'");
    return false;
  }

  if (JS_DefineProperty(m_context, m_globalObject, "me", OBJECT_TO_JSVAL(meObject), NULL, NULL, JSPROP_PERMANENT_VAR) == JS_FALSE) {
    Log(L"failed to define property 'me'");
    return false;
  }

  // compile script file
  if (m_scriptMode == kScriptModeCommand) {
    if (wcslen(Vars.szConsole) > 0) {
      m_script = JS_CompileFile(m_context, m_globalObject, m_fileName);
    } else {
      char* cmd = "function main() {print('ÿc2D2BSÿc0 :: Started Console'); while (true){delay(10000)};}  ";
      m_script = JS_CompileScript(m_context, m_globalObject, cmd, strlen(cmd), "Command Line", 1);
    }
    JS_AddNamedScriptRoot(m_context, &m_script, "console script");
  } else
    m_script = JS_CompileFile(m_context, m_globalObject, m_fileName);

  if (!m_script) {
    Log(L"Couldn't compile the script");
    return false;
  }

  // TODO(ejt): revisit these
  m_scriptState = kScriptStateRunning;
  m_hasActiveCX = true;

  return true;
}

void Script::RunMain() {
  JSAutoRequest request(m_context);
  JS::RootedValue main(m_context, INT_TO_JSVAL(1));
  JS::RootedValue dummy(m_context, INT_TO_JSVAL(1));

  // args passed from load
  JS::AutoValueVector args(m_context);
  args.resize(m_argc);
  for (uint i = 0; i < m_argc; i++) {
    jsval v;
    m_argv[i]->read(m_context, &v);
    args.append(v);
  }

  if (JS_ExecuteScript(m_context, m_globalObject, m_script, dummy.address()) != JS_FALSE &&
      JS_GetProperty(m_context, m_globalObject, "main", main.address()) != JS_FALSE && JSVAL_IS_FUNCTION(m_context, main)) {
    JS_CallFunctionValue(m_context, m_globalObject, main, args.length(), args.begin(), dummy.address());
  }
}

// return false to stop the script
bool Script::RunEventLoop() {
  bool pause = m_isPaused;
  if (pause)
    SetPauseState(true);
  while (m_isPaused) {
    Sleep(50);
    JS_MaybeGC(m_context);
  }
  if (pause)
    SetPauseState(false);

  // run loop until there are no more events or script is interrupted
  return ProcessAllEvents();
}

bool Script::ProcessAllEvents() {
  for (;;) {
    if (m_scriptState == kScriptStateRequestStop) {
      return false;
    }

    if (m_scriptMode == kScriptModeGame && ClientState() == ClientStateMenu) {
      return false;
    }

    EnterCriticalSection(&Vars.cEventSection);
    if (m_EventList.empty()) {
      // no more events
      LeaveCriticalSection(&Vars.cEventSection);
      break;
    }
    Event* evt = m_EventList.back();
    m_EventList.pop_back();
    LeaveCriticalSection(&Vars.cEventSection);
    HandleEvent(evt, false);
  }

  return true;
}

void Script::Cleanup() {
  m_scriptState = kScriptStateStopped;
}

JSBool Script::InterruptHandler(JSContext* ctx) {
  Script* script = (Script*)JS_GetContextPrivate(ctx);
  if (!script->RunEventLoop()) {
    return JS_FALSE;
  }
  return JS_TRUE;
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

DWORD WINAPI ScriptThread(LPVOID lpThreadParameter) {
  Script* script = static_cast<Script*>(lpThreadParameter);
  // TODO(ejt): necessary? script should ALWAYS be here otherwise it's a critical error!
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

JSBool contextCallback(JSContext* ctx, uint contextOp) {
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