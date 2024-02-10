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
#include "JSTimer.h"
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

  EnterCriticalSection(&m_lock);

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
  std::shared_ptr<CommandEvent> evt = std::make_shared<CommandEvent>();
  evt->name = "Command";
  evt->command = command;
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

void Script::DispatchEvent(std::shared_ptr<Event> evt) {
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
    if (m_scriptState == kScriptStateRequestStop) {
      return;
    }
    WaitForSingleObjectEx(m_eventSignal, amt, true);
    ResetEvent(m_eventSignal);

    if (!RunEventLoop()) {
      break;
    }

    amt = delay - (GetTickCount() - start);
  }
}

bool Script::Initialize() {
  uv_loop_init(&m_loop);
  m_threadState.script = this;
  m_threadState.loop = &m_loop;

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

  RegisterBuiltinBindings(m_context);

  // define 'me' property
  m_me = new UnitWrap::UnitData;
  memset(m_me, NULL, sizeof(UnitWrap::UnitData));

  D2UnitStrc* player = D2CLIENT_GetPlayerUnit();
  m_me->dwMode = (DWORD)-1;
  m_me->dwClassId = (DWORD)-1;
  m_me->dwType = UNIT_PLAYER;
  m_me->dwUnitId = player ? player->dwUnitId : NULL;
  m_me->dwPrivateType = PRIVATE_UNIT;

  JSValue meObject = UnitWrap::Instantiate(m_context, JS_UNDEFINED, m_me);
  if (JS_IsException(meObject)) {
    Log("failed to build object 'me'");
    return false;
  }
  JS_SetPropertyFunctionList(m_context, meObject, UnitWrap::me_proto_funcs, _countof(UnitWrap::me_proto_funcs));
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

// TODO(ejt): this is kinda hacky so change this in the future
static void __walk_loop(uv_handle_t* handle, void* /*arg*/) {
  // NOTE(ejt): as of writing this, the only uv timers are TimerWrap*
  // if this changes in the future fix this
  std::vector<TimerWrap*> wraps;
  if (handle->type == uv_handle_type::UV_TIMER) {
    TimerWrap* wrap = static_cast<TimerWrap*>(handle->data);
    if (wrap) {
      wrap->Unref();
    }
  }
}

void Script::Cleanup() {
  PurgeEventList();
  Genhook::Clean(this);

  uv_walk(&m_loop, __walk_loop, nullptr);
  uv_loop_close(&m_loop);

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

  uv_run(&m_loop, UV_RUN_NOWAIT);
  JSContext* ctx;
  for (;;) {
    int r = JS_ExecutePendingJob(m_runtime, &ctx);
    if (r <= 0) {
      if (r < 0) {
        JS_ReportPendingException(ctx);
      }
      break;
    }
  }

  // run loop until there are no more events or script is interrupted
  return ProcessAllEvents();
}

void Script::PurgeEventList() {
  while (m_EventList.size() > 0) {
    EnterCriticalSection(&Vars.cEventSection);
    std::shared_ptr<Event> evt = m_EventList.back();
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

bool Script::HandleEvent(std::shared_ptr<Event> evt, bool clearList) {
  char* evtName = (char*)evt->name.c_str();

  if (strcmp(evtName, "itemaction") == 0) {
    if (!clearList) {
      std::shared_ptr<ItemEvent> itemEvt = std::dynamic_pointer_cast<ItemEvent>(evt);
      JSValue args[] = {
          JS_NewUint32(m_context, itemEvt->id),
          JS_NewUint32(m_context, itemEvt->mode),
          JS_NewString(m_context, itemEvt->code.c_str()),
          JS_NewBool(m_context, itemEvt->global),
      };

      ExecuteEvent(evtName, _countof(args), args);

      for (size_t i = 0; i < _countof(args); ++i) {
        JS_FreeValue(m_context, args[i]);
      }
    }

    return true;
  }
  if (strcmp(evtName, "gameevent") == 0) {
    if (!clearList) {
      std::shared_ptr<GameEvent> gameEvt = std::dynamic_pointer_cast<GameEvent>(evt);
      JSValue args[] = {
          JS_NewUint32(m_context, gameEvt->mode),
          JS_NewUint32(m_context, gameEvt->param1),
          JS_NewUint32(m_context, gameEvt->param2),
          !gameEvt->name1.empty() ? JS_NewString(m_context, gameEvt->name1.c_str()) : JS_UNDEFINED,
          !gameEvt->name2.empty() ? JS_NewString(m_context, gameEvt->name2.c_str()) : JS_UNDEFINED,
      };

      ExecuteEvent(evtName, _countof(args), args);

      for (size_t i = 0; i < _countof(args); ++i) {
        JS_FreeValue(m_context, args[i]);
      }
    }

    return true;
  }
  if (strcmp(evtName, "copydata") == 0) {
    if (!clearList) {
      std::shared_ptr<CopyDataMessageEvent> dataEvt = std::dynamic_pointer_cast<CopyDataMessageEvent>(evt);
      JSValue args[] = {
          JS_NewUint32(m_context, dataEvt->mode),
          !dataEvt->msg.empty() ? JS_NewString(m_context, dataEvt->msg.c_str()) : JS_UNDEFINED,
      };

      ExecuteEvent(evtName, _countof(args), args);

      for (size_t i = 0; i < _countof(args); ++i) {
        JS_FreeValue(m_context, args[i]);
      }
    }

    return true;
  }
  if (strcmp(evtName, "chatmsg") == 0 || strcmp(evtName, "chatinput") == 0 || strcmp(evtName, "whispermsg") == 0 || strcmp(evtName, "chatmsgblocker") == 0 ||
      strcmp(evtName, "chatinputblocker") == 0 || strcmp(evtName, "whispermsgblocker") == 0) {
    bool block = false;
    if (!clearList) {
      std::shared_ptr<ChatMessageEvent> chatEvt = std::dynamic_pointer_cast<ChatMessageEvent>(evt);
      JSValue args[] = {
          !chatEvt->nickname.empty() ? JS_NewString(m_context, chatEvt->nickname.c_str()) : JS_UNDEFINED,
          !chatEvt->msg.empty() ? JS_NewString(m_context, chatEvt->msg.c_str()) : JS_UNDEFINED,
      };

      ExecuteEvent(evtName, _countof(args), args, &block);

      for (size_t i = 0; i < _countof(args); ++i) {
        JS_FreeValue(m_context, args[i]);
      }
    }
    if (strcmp(evtName, "chatmsgblocker") == 0 || strcmp(evtName, "chatinputblocker") == 0 || strcmp(evtName, "whispermsgblocker") == 0) {
      evt->block = block;
      SetEvent(Vars.eventSignal);
    }
    return true;
  }
  if (strcmp(evtName, "mousemove") == 0) {
    if (!clearList) {
      std::shared_ptr<MouseEvent> mouseEvt = std::dynamic_pointer_cast<MouseEvent>(evt);
      JSValue args[] = {
          JS_NewUint32(m_context, mouseEvt->x),
          JS_NewUint32(m_context, mouseEvt->y),
      };

      ExecuteEvent(evtName, _countof(args), args);

      for (size_t i = 0; i < _countof(args); ++i) {
        JS_FreeValue(m_context, args[i]);
      }
    }

    return true;
  }
  if (strcmp(evtName, "ScreenHookHover") == 0) {
    std::shared_ptr<GenHookEvent> genHookEvt = std::dynamic_pointer_cast<GenHookEvent>(evt);
    if (!clearList) {
      JSValue args[] = {
          JS_NewUint32(m_context, genHookEvt->x),
          JS_NewUint32(m_context, genHookEvt->y),
      };

      ExecuteEvent(evtName, _countof(args), args);

      for (size_t i = 0; i < _countof(args); ++i) {
        JS_FreeValue(m_context, args[i]);
      }
    }
    JS_FreeValue(m_context, genHookEvt->callback);

    return true;
  }
  if (strcmp(evtName, "mouseclick") == 0) {
    if (!clearList) {
      std::shared_ptr<MouseEvent> mouseEvt = std::dynamic_pointer_cast<MouseEvent>(evt);
      JSValue args[] = {
          JS_NewUint32(m_context, mouseEvt->button),
          JS_NewUint32(m_context, mouseEvt->x),
          JS_NewUint32(m_context, mouseEvt->y),
          JS_NewUint32(m_context, mouseEvt->state),
      };

      ExecuteEvent(evtName, _countof(args), args);

      for (size_t i = 0; i < _countof(args); ++i) {
        JS_FreeValue(m_context, args[i]);
      }
    }

    return true;
  }
  if (strcmp(evtName, "melife") == 0 || strcmp(evtName, "memana") == 0) {
    if (!clearList) {
      std::shared_ptr<HealthManaEvent> hpmpEvt = std::dynamic_pointer_cast<HealthManaEvent>(evt);
      JSValue args[] = {
          JS_NewUint32(m_context, hpmpEvt->value),
      };

      ExecuteEvent(evtName, _countof(args), args);

      for (size_t i = 0; i < _countof(args); ++i) {
        JS_FreeValue(m_context, args[i]);
      }
    }

    return true;
  }
  if (strcmp(evtName, "playerassign") == 0) {
    if (!clearList) {
      std::shared_ptr<PlayerAssignedEvent> assignEvt = std::dynamic_pointer_cast<PlayerAssignedEvent>(evt);
      JSValue args[] = {
          JS_NewUint32(m_context, assignEvt->id),
      };

      ExecuteEvent(evtName, _countof(args), args);

      for (size_t i = 0; i < _countof(args); ++i) {
        JS_FreeValue(m_context, args[i]);
      }
    }

    return true;
  }
  if (strcmp(evtName, "keyup") == 0 || strcmp(evtName, "keydownblocker") == 0 || strcmp(evtName, "keydown") == 0) {
    bool block = false;
    if (!clearList) {
      std::shared_ptr<KeyEvent> keyEvt = std::dynamic_pointer_cast<KeyEvent>(evt);
      JSValue args[] = {
          JS_NewUint32(m_context, keyEvt->key),
      };

      ExecuteEvent(evtName, _countof(args), args, &block);

      for (size_t i = 0; i < _countof(args); ++i) {
        JS_FreeValue(m_context, args[i]);
      }
    }
    if (strcmp(evtName, "keydownblocker") == 0) {
      evt->block = block;
      SetEvent(Vars.eventSignal);
    }
    return true;
  }
  if (strcmp(evtName, "ScreenHookClick") == 0) {
    std::shared_ptr<GenHookEvent> genHookEvt = std::dynamic_pointer_cast<GenHookEvent>(evt);
    bool block = false;
    if (!clearList) {
      JSValue args[] = {
          JS_NewUint32(m_context, genHookEvt->x),
          JS_NewUint32(m_context, genHookEvt->y),
          JS_NewUint32(m_context, genHookEvt->button),
      };

      JSValue rval;
      // diffrent function source for hooks
      rval = JS_Call(m_context, m_globalObject, genHookEvt->callback, _countof(args), args);
      block |= static_cast<bool>(JS_IsBool(rval) && JS_ToBool(m_context, rval));

      for (size_t i = 0; i < _countof(args); ++i) {
        JS_FreeValue(m_context, args[i]);
      }
    }
    JS_FreeValue(m_context, genHookEvt->callback);
    evt->block = block;
    SetEvent(Vars.eventSignal);

    return true;
  }
  if (strcmp(evtName, "Command") == 0) {
    std::shared_ptr<CommandEvent> cmdEvt = std::dynamic_pointer_cast<CommandEvent>(evt);
    std::string test;

    test.append("try{ ");
    test.append(cmdEvt->command);
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
  }
  if (strcmp(evtName, "scriptmsg") == 0) {
    std::shared_ptr<ScriptMsgEvent> msgEvt = std::dynamic_pointer_cast<ScriptMsgEvent>(evt);
    if (!clearList) {
      JSValue data_obj = JS_ReadObject(m_context, msgEvt->data, msgEvt->data_len, JS_READ_OBJ_SAB | JS_READ_OBJ_REFERENCE);
      ExecuteEvent(evtName, 1, &data_obj);
      JS_FreeValue(m_context, data_obj);
    }

    // decrease SAB reference count
    for (size_t i = 0; i < msgEvt->sab_tab_len; ++i) {
      js_sab_free(NULL, msgEvt->sab_tab[i]);
    }
    free(msgEvt->data);
    free(msgEvt->sab_tab);
    return true;
  }
  if (strcmp(evtName, "gamepacket") == 0 || strcmp(evtName, "gamepacketsent") == 0 || strcmp(evtName, "realmpacket") == 0) {
    bool block = false;
    if (!clearList) {
      std::shared_ptr<PacketEvent> packetEvt = std::dynamic_pointer_cast<PacketEvent>(evt);

      JSValue arr = JS_NewArray(m_context);
      for (size_t i = 0; i < packetEvt->bytes.size(); i++) {
        JS_SetPropertyUint32(m_context, arr, i, JS_NewUint32(m_context, packetEvt->bytes[i]));
      }

      ExecuteEvent(evtName, 1, &arr, &block);
      JS_FreeValue(m_context, arr);
    }

    evt->block = block;
    SetEvent(Vars.eventSignal);

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
    std::shared_ptr<Event> evt = m_EventList.back();
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