#include <io.h>
#include <algorithm>

#include "Script.h"
#include "Core.h"
#include "JSUnit.h"
#include "Helpers.h"
#include "ScriptEngine.h"
#include "Engine.h"
#include "JSGlobalFuncs.h"
#include "Console.h"
#include "Bindings.h"

#include <chrono>
#include <cassert>

Script::Script(const char* file, ScriptMode mode /*, uint32_t argc, JSAutoStructuredCloneBuffer** argv*/)
    : m_runtime(NULL),
      m_context(NULL),
      m_globalObject(JS_UNDEFINED),
      m_script(JS_UNDEFINED),
      m_isPaused(false),
      m_isReallyPaused(false),
      m_scriptMode(mode),
      m_scriptState(kScriptStateUninitialized),
      m_threadState({}) {
  InitializeCriticalSection(&m_lock);
  m_eventSignal = CreateEvent(nullptr, true, false, nullptr);

  if (m_scriptMode == kScriptModeCommand && strlen(file) < 1) {
    m_fileName = "Command Line";
  } else {
    if (_access(file, 0) != 0) {
      Log("%s (%s, %d)", file, __FILE__, __LINE__);

      throw std::exception("File not found");
    }

    char* tmpName = _strdup(file);
    if (!tmpName)
      throw std::exception("Could not dup filename");

    _strlwr_s(tmpName, strlen(file) + 1);
    m_fileName = tmpName;
    replace(m_fileName.begin(), m_fileName.end(), '/', '\\');
    free(tmpName);
  }
}

Script::~Script(void) {
  assert(!m_runtime);
  assert(!m_context);

  // if (JS_IsInRequest(m_runtime))
  //   JS_EndRequest(m_context);

  EnterCriticalSection(&m_lock);
  // JS_DestroyContext(m_context);
  // JS_DestroyRuntime(m_runtime);

  // m_context = NULL;
  // m_globalObject = NULL;
  // m_script = NULL;
  CloseHandle(m_eventSignal);
  m_includes.clear();
  if (m_threadState.handle != INVALID_HANDLE_VALUE)
    CloseHandle(m_threadState.handle);
  LeaveCriticalSection(&m_lock);
  DeleteCriticalSection(&m_lock);
}

bool Script::Start() {
  EnterCriticalSection(&m_lock);
  DWORD dwExitCode = STILL_ACTIVE;

  if ((!GetExitCodeThread(m_threadState.handle, &dwExitCode) || dwExitCode != STILL_ACTIVE) &&
      (m_threadState.handle = CreateThread(0, 0, ScriptThread, this, 0, &m_threadState.id)) != NULL) {
    LeaveCriticalSection(&m_lock);
    return true;
  }

  m_threadState.handle = INVALID_HANDLE_VALUE;
  LeaveCriticalSection(&m_lock);
  return false;
}

void Script::Stop(bool force) {
  // make sure the script is actually running
  if (m_scriptState != kScriptStateRunning)
    return;

  EnterCriticalSection(&m_lock);
  // tell everyone else that the script is aborted FIRST
  m_scriptState = kScriptStateRequestStop;
  m_isPaused = false;
  m_isReallyPaused = false;
  if (m_scriptMode != kScriptModeCommand) {
    const char* displayName = m_fileName.c_str() + strlen(Vars.szScriptPath) + 1;
    Print("Script %s ended", displayName);
  }

  // trigger call back so script ends
  SetEvent(m_eventSignal);

  // normal wait: 500ms, forced wait: 300ms, really forced wait: 100ms
  int maxCount = (force ? 10 : 30);
  if (GetCurrentThreadId() != GetThreadId()) {
    for (int i = 0; m_scriptState != kScriptStateStopped; i++) {
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
  HANDLE hThread = m_threadState.handle;
  LeaveCriticalSection(&m_lock);

  if (hThread != INVALID_HANDLE_VALUE)
    WaitForSingleObject(hThread, INFINITE);
}

void Script::Pause(void) {
  if (!(m_scriptState == kScriptStateRunning) && !m_isPaused)
    m_isPaused = true;
  SetEvent(m_eventSignal);
}

void Script::Resume(void) {
  if (!(m_scriptState == kScriptStateRunning) && m_isPaused)
    m_isPaused = false;
  SetEvent(m_eventSignal);
}

bool Script::IsUninitialized() {
  return m_scriptState == kScriptStateUninitialized;
}

bool Script::IsRunning(void) {
  return /*m_context && */ !(m_scriptState != kScriptStateRunning || m_isPaused);
}

bool Script::IsAborted() {
  return m_scriptState == kScriptStateStopped;
}

void Script::RunCommand(const char* command) {
  Event* evt = new Event;
  evt->name = "Command";
  evt->arg1 = _strdup(command);
  DispatchEvent(evt);
}

const char* Script::GetShortFilename() {
  if (strcmp(m_fileName.c_str(), "Command Line") == 0)
    return m_fileName.c_str();
  else
    return (m_fileName.c_str() + strlen(Vars.szScriptPath) + 1);
}

DWORD Script::GetThreadId(void) {
  return (m_threadState.handle == INVALID_HANDLE_VALUE ? -1 : m_threadState.id);
}

ThreadState* Script::GetThreadState() {
  return &m_threadState;
}

void Script::UpdatePlayerGid(void) {
  m_me->dwUnitId = (D2CLIENT_GetPlayerUnit() == NULL ? NULL : D2CLIENT_GetPlayerUnit()->dwUnitId);
}

bool Script::IsIncluded(const char* file) {
  uint32_t count = 0;
  char* fname = _strdup(file);
  if (!fname)
    return false;

  _strlwr_s(fname, strlen(fname) + 1);
  StringReplace(fname, '/', '\\', strlen(fname));
  count = m_includes.count(fname);
  free(fname);

  return !!count;
}

bool Script::Include(const char* file) {
  // since includes will happen on the same thread, locking here is acceptable
  EnterCriticalSection(&m_lock);
  char* fname = _strdup(file);
  if (!fname) {
    LeaveCriticalSection(&m_lock);
    return false;
  }
  _strlwr_s(fname, strlen(fname) + 1);
  StringReplace(fname, '/', '\\', strlen(fname));

  // don't invoke the string ctor more than once...
  std::string currentFileName = std::string(fname);
  // ignore already included, 'in-progress' includes, and self-inclusion
  if (!!m_includes.count(fname) || !!m_inProgress.count(fname) || (currentFileName.compare(m_fileName.c_str()) == 0)) {
    LeaveCriticalSection(&m_lock);
    free(fname);
    return true;
  }

  JSValue rval = JS_CompileFile(m_context, m_globalObject, fname);
  if (!JS_IsException(rval)) {
    m_inProgress[fname] = true;
    rval = JS_EvalFunction(m_context, rval);
    if (!JS_IsException(rval)) {
      m_includes[fname] = true;
    }
    m_inProgress.erase(fname);
  }

  LeaveCriticalSection(&m_lock);
  free(fname);

  if (JS_IsException(rval)) {
    JS_ReportPendingException(m_context);
    return false;
  }

  JS_FreeValue(m_context, rval);
  return true;
}

size_t Script::GetListenerCount(const char* evtName, JSValue evtFunc) {
  // idk old code had this check so keeping it just in case, but such validation is really the callers responsibility // ejt
  if (!evtName || strlen(evtName) == 0) {
    return 0;
  }
  if (JS_IsUndefined(evtFunc)) {
    return m_functions.count(evtName);
  }

  // if there are no events registered under that name at all, then obviously there
  // can't be a specific one registered under that name
  if (!m_functions.contains(evtName)) {
    return 0;
  }

  size_t count = 0;
  for (const auto& func : m_functions[evtName]) {
    if (func == evtFunc) {
      ++count;
    }
  }
  return count;
}

void Script::AddEventListener(const char* evtName, JSValue evtFunc) {
  EnterCriticalSection(&m_lock);
  if (JS_IsFunction(m_context, evtFunc) && strlen(evtName) > 0) {
    m_functions[evtName].push_back(JS_DupValue(m_context, evtFunc));
  }
  LeaveCriticalSection(&m_lock);
}

void Script::RemoveEventListener(const char* evtName, JSValue evtFunc) {
  if (strlen(evtName) < 1)
    return;

  EnterCriticalSection(&m_lock);
  for (FunctionList::iterator it = m_functions[evtName].begin(); it != m_functions[evtName].end(); it++) {
    if (*it == evtFunc) {
      JS_FreeValue(m_context, *it);
      it = m_functions[evtName].erase(it);
      break;
    }
  }

  // Remove event completely if there are no listeners for it.
  if (m_functions.count(evtName) > 0 && m_functions[evtName].size() == 0)
    m_functions.erase(evtName);

  LeaveCriticalSection(&m_lock);
}

void Script::RemoveAllListeners(const char* evtName) {
  EnterCriticalSection(&m_lock);
  for (FunctionList::iterator it = m_functions[evtName].begin(); it != m_functions[evtName].end(); it++) {
    JS_FreeValue(m_context, *it);
  }
  m_functions[evtName].clear();
  LeaveCriticalSection(&m_lock);
}

void Script::RemoveAllEventListeners() {
  EnterCriticalSection(&m_lock);
  for (FunctionMap::iterator it = m_functions.begin(); it != m_functions.end(); it++) {
    RemoveAllListeners(it->first.c_str());
  }
  m_functions.clear();
  LeaveCriticalSection(&m_lock);
}

void Script::DispatchEvent(Event* evt) {
  EnterCriticalSection(&Vars.cEventSection);
  m_EventList.push_front(evt);
  LeaveCriticalSection(&Vars.cEventSection);
  SetEvent(m_eventSignal);
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

    amt = delay - (GetTickCount() - start);
  }
}

bool Script::Initialize() {
  m_threadState.script = this;
  m_threadState.loop = static_cast<uv_loop_t*>(malloc(sizeof(uv_loop_t)));
  uv_loop_init(m_threadState.loop);

  m_runtime = JS_NewRuntime();
  JS_SetRuntimeOpaque(m_runtime, &m_threadState);
  JS_SetInterruptHandler(m_runtime, InterruptHandler, this);
  JS_SetMaxStackSize(m_runtime, 0);
  JS_SetMemoryLimit(m_runtime, 100 * 1024 * 1024);

  m_context = JS_NewContext(m_runtime);
  if (!m_context) {
    Log("Couldn't create the context");
    return false;
  }
  JS_SetContextOpaque(m_context, this);

  m_globalObject = JS_GetGlobalObject(m_context);

  // JSValue console = JS_NewObject(m_context);
  // JS_SetPropertyStr(m_context, console, "trace", JS_NewCFunction(m_context, my_print, "trace", 1));
  // JS_SetPropertyStr(m_context, console, "debug", JS_NewCFunction(m_context, my_print, "debug", 1));
  // JS_SetPropertyStr(m_context, console, "log", JS_NewCFunction(m_context, my_print, "log", 1));
  // JS_SetPropertyStr(m_context, console, "error", JS_NewCFunction(m_context, my_print, "error", 1));
  // JS_SetPropertyStr(m_context, m_globalObject, "console", console);

  // TODO(ejt): this is a solution to minimize changes during migration to quickjs, refactor this in the future
  JS_SetPropertyFunctionList(m_context, m_globalObject, global_funcs, _countof(global_funcs));
  for (JSClassSpec* entry = global_classes; entry->name != NULL; entry++) {
    JSClassDef def{};
    def.class_name = entry->name;
    def.finalizer = entry->finalizer;

    JS_NewClassID(entry->pclass_id);
    JS_NewClass(m_runtime, *entry->pclass_id, &def);
    JSValue proto = JS_NewObject(m_context);
    JSValue obj;

    if (entry->proto_funcs) {
      JS_SetPropertyFunctionList(m_context, proto, entry->proto_funcs, entry->num_proto_funcs);
    }

    if (entry->ctor) {
      obj = JS_NewCFunction2(m_context, entry->ctor, entry->name, 0, JS_CFUNC_constructor, 0);
      JS_SetConstructor(m_context, obj, proto);
    } else {
      obj = JS_NewObjectProtoClass(m_context, proto, *entry->pclass_id);
    }

    if (entry->static_funcs) {
      JS_SetPropertyFunctionList(m_context, obj, entry->static_funcs, entry->num_static_funcs);
    }

    JS_SetClassProto(m_context, *entry->pclass_id, proto);
    JS_SetPropertyStr(m_context, m_globalObject, entry->name, obj);
  }

  RegisterBuiltinBindings(m_context);

  // define 'me' property
  m_me = new JSUnit;
  memset(m_me, NULL, sizeof(JSUnit));

  D2UnitStrc* player = D2CLIENT_GetPlayerUnit();
  m_me->dwMode = (DWORD)-1;
  m_me->dwClassId = (DWORD)-1;
  m_me->dwType = UNIT_PLAYER;
  m_me->dwUnitId = player ? player->dwUnitId : NULL;
  m_me->dwPrivateType = PRIVATE_UNIT;

  JSValue meObject = BuildObject(m_context, unit_class_id, FUNCLIST(me_proto_funcs), m_me);
  if (!meObject) {
    Log("failed to build object 'me'");
    return false;
  }

  JS_SetPropertyStr(m_context, m_globalObject, "me", meObject);

  // compile script file
  if (m_scriptMode == kScriptModeCommand) {
    if (strlen(Vars.szConsole) > 0) {
      m_script = JS_CompileFile(m_context, m_globalObject, m_fileName);
    } else {
      const char* cmd = "function main() {print('ÿc2D2BSÿc0 :: Started Console'); while (true){delay(10000)};}  ";
      m_script = JS_Eval(m_context, cmd, strlen(cmd), "Command Line", JS_EVAL_TYPE_GLOBAL);
    }
  } else {
    m_script = JS_CompileFile(m_context, m_globalObject, m_fileName);
  }

  if (JS_IsException(m_script)) {
    JS_ReportPendingException(m_context);
    return false;
  }

  // TODO(ejt): revisit these
  m_scriptState = kScriptStateRunning;

  return true;
}

void Script::Cleanup() {
  PurgeEventList();
  Genhook::Clean(this);

  if (m_context) {
    JS_FreeValue(m_context, m_script);
    m_script = JS_UNDEFINED;
    JS_FreeValue(m_context, m_globalObject);
    m_globalObject = JS_UNDEFINED;
    JS_FreeContext(m_context);
    m_context = nullptr;
  }
  if (m_runtime) {
    JS_FreeRuntime(m_runtime);
    m_runtime = nullptr;
  }
  if (m_threadState.loop) {
    uv_loop_close(m_threadState.loop);
    free(m_threadState.loop);
  }
  m_scriptState = kScriptStateStopped;

  // TODO(ejt): revisit this
  if (Vars.bDisableCache)
    sScriptEngine->DisposeScript(this);
}

void Script::RunMain() {
  JSValue main;

  // args passed from load
  // JS::AutoValueVector args(m_context);
  // args.resize(m_argc);
  // for (uint32_t i = 0; i < m_argc; i++) {
  //  JSValue v;
  //  m_argv[i]->read(m_context, &v);
  //  args.append(v);
  //}
  m_script = JS_EvalFunction(m_context, m_script);
  if (JS_IsException(m_script)) {
    JS_ReportPendingException(m_context);
    return;
  }
  main = JS_GetPropertyStr(m_context, m_globalObject, "main");
  if (JS_IsException(main)) {
    JS_ReportPendingException(m_context);
    return;
  }
  if (!JS_IsFunction(m_context, main)) {
    JS_ThrowTypeError(m_context, "'main' is not a function");
    JS_ReportPendingException(m_context);
    return;
  }
  JSValue rval = JS_Call(m_context, main, JS_UNDEFINED, 0, nullptr);
  JS_FreeValue(m_context, main);
  if (JS_IsException(rval)) {
    JS_ReportPendingException(m_context);
    return;
  }
  JS_FreeValue(m_context, rval);
}

// return false to stop the script
bool Script::RunEventLoop() {
  bool pause = m_isPaused;
  if (pause)
    SetPauseState(true);
  while (m_isPaused) {
    Sleep(50);
    // JS_MaybeGC(m_context);
  }
  if (pause)
    SetPauseState(false);

  // run loop until there are no more events or script is interrupted
  return ProcessAllEvents();
}

void Script::PurgeEventList() {
  while (m_EventList.size() > 0) {
    EnterCriticalSection(&Vars.cEventSection);
    Event* evt = m_EventList.back();
    m_EventList.pop_back();
    LeaveCriticalSection(&Vars.cEventSection);
    HandleEvent(evt, true);  // clean list and pop events
  }

  RemoveAllEventListeners();
}

void Script::ExecuteEvent(char* evtName, int argc, const JSValue* argv, bool* block) {
  for (const auto& root : m_functions[evtName]) {
    JSValue rval;
    rval = JS_Call(m_context, root, JS_UNDEFINED, argc, const_cast<JSValue*>(argv));
    if (JS_IsException(rval)) {
      JS_ReportPendingException(m_context);
      return;
    }
    if (block) {
      *block |= static_cast<bool>(JS_IsBool(rval) && JS_ToBool(m_context, rval));
    }
    JS_FreeValue(m_context, rval);
  }
}

bool Script::HandleEvent(Event* evt, bool clearList) {
  char* evtName = (char*)evt->name.c_str();

  if (strcmp(evtName, "itemaction") == 0) {
    if (!clearList) {
      DWORD* gid = (DWORD*)evt->arg1;
      char* code = (char*)evt->arg2;
      DWORD* mode = (DWORD*)evt->arg3;
      bool* global = (bool*)evt->arg4;

      JSValue args[] = {
          JS_NewUint32(m_context, *gid),
          JS_NewUint32(m_context, *mode),
          JS_NewString(m_context, code),
          JS_NewBool(m_context, *global),
      };

      ExecuteEvent(evtName, _countof(args), args);

      for (size_t i = 0; i < _countof(args); ++i) {
        JS_FreeValue(m_context, args[i]);
      }
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
      JSValue args[] = {
          JS_NewUint32(m_context, *(BYTE*)evt->arg1),
          JS_NewUint32(m_context, *(DWORD*)evt->arg2),
          JS_NewUint32(m_context, *(DWORD*)evt->arg3),
          evt->arg4 ? JS_NewString(m_context, (const char*)evt->arg4) : JS_UNDEFINED,
          evt->arg5 ? JS_NewString(m_context, (const char*)evt->arg5) : JS_UNDEFINED,
      };

      ExecuteEvent(evtName, _countof(args), args);

      for (size_t i = 0; i < _countof(args); ++i) {
        JS_FreeValue(m_context, args[i]);
      }
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
      JSValue args[] = {
          JS_NewUint32(m_context, *(DWORD*)evt->arg1),
          evt->arg2 ? JS_NewString(m_context, (const char*)evt->arg2) : JS_UNDEFINED,
      };

      ExecuteEvent(evtName, _countof(args), args);

      for (size_t i = 0; i < _countof(args); ++i) {
        JS_FreeValue(m_context, args[i]);
      }
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
      JSValue args[] = {
          evt->arg1 ? JS_NewString(m_context, (const char*)evt->arg1) : JS_UNDEFINED,
          evt->arg2 ? JS_NewString(m_context, (const char*)evt->arg2) : JS_UNDEFINED,
      };

      ExecuteEvent(evtName, _countof(args), args, &block);

      for (size_t i = 0; i < _countof(args); ++i) {
        JS_FreeValue(m_context, args[i]);
      }
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
  if (strcmp(evtName, "mousemove") == 0) {
    if (!clearList) {
      JSValue args[] = {
          JS_NewUint32(m_context, *(DWORD*)evt->arg1),
          JS_NewUint32(m_context, *(DWORD*)evt->arg2),
      };

      ExecuteEvent(evtName, _countof(args), args);

      for (size_t i = 0; i < _countof(args); ++i) {
        JS_FreeValue(m_context, args[i]);
      }
    }
    delete evt->arg1;
    delete evt->arg2;
    delete evt;
    return true;
  }
  if (strcmp(evtName, "ScreenHookHover") == 0) {
    if (!clearList) {
      JSValue args[] = {
          JS_NewUint32(m_context, *(DWORD*)evt->arg1),
          JS_NewUint32(m_context, *(DWORD*)evt->arg2),
      };

      ExecuteEvent(evtName, _countof(args), args);

      for (size_t i = 0; i < _countof(args); ++i) {
        JS_FreeValue(m_context, args[i]);
      }
    }
    delete evt->arg1;
    delete evt->arg2;
    delete evt;
    return true;
  }
  if (strcmp(evtName, "mouseclick") == 0) {
    if (!clearList) {
      JSValue args[] = {
          JS_NewUint32(m_context, *(DWORD*)evt->arg1),
          JS_NewUint32(m_context, *(DWORD*)evt->arg2),
          JS_NewUint32(m_context, *(DWORD*)evt->arg3),
          JS_NewUint32(m_context, *(DWORD*)evt->arg4),
      };

      ExecuteEvent(evtName, _countof(args), args);

      for (size_t i = 0; i < _countof(args); ++i) {
        JS_FreeValue(m_context, args[i]);
      }
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
      JSValue args[] = {
          JS_NewUint32(m_context, *(DWORD*)evt->arg1),
      };

      ExecuteEvent(evtName, _countof(args), args, &block);

      for (size_t i = 0; i < _countof(args); ++i) {
        JS_FreeValue(m_context, args[i]);
      }
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
      JSValue args[] = {
          JS_NewUint32(m_context, *(DWORD*)evt->arg1),
          JS_NewUint32(m_context, *(DWORD*)evt->arg2),
          JS_NewUint32(m_context, *(DWORD*)evt->arg3),
      };

      JSValue rval;
      // diffrent function source for hooks
      for (FunctionList::iterator it = evt->functions.begin(); it != evt->functions.end(); it++) {
        rval = JS_Call(m_context, m_globalObject, *it, _countof(args), args);
        block |= static_cast<bool>(JS_IsBool(rval) && JS_ToBool(m_context, rval));
      }

      for (size_t i = 0; i < _countof(args); ++i) {
        JS_FreeValue(m_context, args[i]);
      }
    }
    *(DWORD*)evt->arg4 = block;
    SetEvent(Vars.eventSignal);

    return true;
  }
  if (strcmp(evtName, "Command") == 0) {
    const char* cmd = (const char*)evt->arg1;
    std::string test;

    test.append("try{ ");
    test.append(cmd);
    test.append(" } catch (error){print(error)}");

    JSValue rval = JS_Eval(m_context, test.data(), test.length(), "Command Line", JS_EVAL_TYPE_GLOBAL);
    if (!JS_IsException(rval)) {
      if (!JS_IsNull(rval) && !JS_IsUndefined(rval)) {
        const char* text = JS_ToCString(m_context, rval);
        Print(text);
        JS_FreeCString(m_context, text);
      }
      JS_FreeValue(m_context, rval);
    } else {
      JS_ReportPendingException(m_context);
    }
    free(evt->arg1);
    delete evt;
  }
  if (strcmp(evtName, "scriptmsg") == 0) {
    if (!clearList) {
      JSValue data_obj = JS_ReadObject(m_context, evt->data, evt->data_len, JS_READ_OBJ_SAB | JS_READ_OBJ_REFERENCE);
      ExecuteEvent(evtName, 1, &data_obj);
      JS_FreeValue(m_context, data_obj);
    }

    // decrease SAB reference count
    for (size_t i = 0; i < evt->sab_tab_len; ++i) {
      js_sab_free(NULL, evt->sab_tab[i]);
    }
    free(evt->data);
    free(evt->sab_tab);
    delete evt;
    return true;
  }
  if (strcmp(evtName, "gamepacket") == 0 || strcmp(evtName, "gamepacketsent") == 0 || strcmp(evtName, "realmpacket") == 0) {
    bool block = false;
    if (!clearList) {
      BYTE* help = (BYTE*)evt->arg1;
      DWORD* size = (DWORD*)evt->arg2;

      JSValue arr = JS_NewArray(m_context);
      for (uint32_t i = 0; i < *size; i++) {
        JS_SetPropertyUint32(m_context, arr, i, JS_NewUint32(m_context, help[i]));
      }

      ExecuteEvent(evtName, 1, &arr, &block);
      JS_FreeValue(m_context, arr);
      *(DWORD*)evt->arg4 = block;
      SetEvent(Vars.eventSignal);
    }

    // delete evt->arg1;
    // delete evt->arg2;

    return true;
  }
  if (strcmp(evtName, "setTimeout") == 0 || strcmp(evtName, "setInterval") == 0) {
    if (!clearList) {
      // JSAutoRequest request(m_context);
      // JSValue dummy;
      // ExecuteEvent(evtName, 0, &dummy);
    }

    // if (strcmp(evtName, "setTimeout") == 0) {
    //   sScriptEngine->RemoveDelayedEvent(*(DWORD*)evt->arg1);
    // }

    return true;
  }
  if (strcmp(evtName, "DisposeMe") == 0) {
    sScriptEngine->DisposeScript(this);
  }

  return true;
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

// return != 0 if the JS code needs to be interrupted
int Script::InterruptHandler(JSRuntime* rt, void* /*opaque*/) {
  ThreadState* ts = (ThreadState*)JS_GetRuntimeOpaque(rt);
  Script* script = ts->script;
  uv_loop_t* loop = ts->loop;

  // run loop once without waiting for now, could run a certain amount of time if handles/requests are piling up
  uv_run(loop, UV_RUN_NOWAIT);

  if (!script->RunEventLoop()) {
    return 1;
  }
  return 0;
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
  }
  return 0;
}