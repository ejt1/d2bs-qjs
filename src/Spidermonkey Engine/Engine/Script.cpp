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

const JSClassOps DefaultGlobalClassOps = {
    nullptr,                         // addProperty
    nullptr,                         // deleteProperty
    nullptr,                         // enumerate
    JS_NewEnumerateStandardClasses,  // newEnumerate
    JS_ResolveStandardClass,         // resolve
    JS_MayResolveStandardClass,      // mayResolve
    nullptr,                         // finalize
    nullptr,                         // call
    nullptr,                         // hasInstance
    nullptr,                         // construct
    JS_GlobalObjectTraceHook         // trace
};

Script::Script(const char* file, ScriptMode mode /*, uint32_t argc, JSAutoStructuredCloneBuffer** argv*/)
    : m_context(NULL),
      m_globalObject(),
      m_script(),
      m_isPaused(false),
      m_isReallyPaused(false),
      m_scriptMode(mode),
      m_scriptState(kScriptStateUninitialized),
      m_threadState({}),
      m_debugCaptureStackFrames(false) {
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

void Script::Stop() {
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

  if (GetCurrentThreadId() != GetThreadId()) {
    for (int i = 0; m_scriptState != kScriptStateStopped; i++) {
      // if we pass the time frame, just ignore the wait because the thread will end forcefully anyway
      if (i >= 30)
        break;
      Sleep(10);
    }
  }
  LeaveCriticalSection(&m_lock);
}

void Script::Run(void) {
  // NOTE(ejt): we're not sure script is alive after cleanup so save filename here
  std::string filename = GetShortFilename();
  Log("Starting thread for %s", filename.c_str());
  if (Initialize()) {
    RunMain();
  }
  Cleanup();
  Log("Ending thread for %s", filename.c_str());
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
  if (m_scriptState == kScriptStateRunning && !m_isPaused)
    m_isPaused = true;
  SetEvent(m_eventSignal);
}

void Script::Resume(void) {
  if (m_scriptState == kScriptStateRunning && m_isPaused)
    m_isPaused = false;
  SetEvent(m_eventSignal);
}

bool Script::IsUninitialized() {
  return m_scriptState == kScriptStateUninitialized;
}

bool Script::IsRunning(void) {
  return m_scriptState == kScriptStateRunning && !m_isPaused;
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

  JS::RootedScript script(m_context, JS_CompileFile(m_context, m_globalObject, fname));
  if (!script) {
    JS_ReportPendingException(m_context);
    LeaveCriticalSection(&m_lock);
    free(fname);
    return false;
  }

  m_inProgress[fname] = true;
  if (!JS_ExecuteScript(m_context, script)) {
    JS_ReportPendingException(m_context);
    LeaveCriticalSection(&m_lock);
    free(fname);
    return false;
  }
  m_includes[fname] = true;
  m_inProgress.erase(fname);

  LeaveCriticalSection(&m_lock);
  free(fname);

  return true;
}

size_t Script::GetListenerCount(const char* evtName, JS::HandleValue evtFunc) {
  // idk old code had this check so keeping it just in case, but such validation is really the callers responsibility // ejt
  if (!evtName || strlen(evtName) == 0) {
    return 0;
  }
  if (evtFunc.isNullOrUndefined()) {
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

void Script::AddEventListener(const char* evtName, JS::HandleValue evtFunc) {
  EnterCriticalSection(&m_lock);
  if (JS_ObjectIsFunction(m_context, evtFunc.toObjectOrNull()) && strlen(evtName) > 0) {
    JS::PersistentRootedValue val(m_context, evtFunc);
    m_functions[evtName].emplace_back(std::move(val));
  }
  LeaveCriticalSection(&m_lock);
}

void Script::RemoveEventListener(const char* evtName, JS::HandleValue evtFunc) {
  if (strlen(evtName) < 1)
    return;

  EnterCriticalSection(&m_lock);
  for (FunctionList::iterator it = m_functions[evtName].begin(); it != m_functions[evtName].end(); it++) {
    if (it->get() == evtFunc) {
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
  m_functions[evtName].clear();
  LeaveCriticalSection(&m_lock);
}

void Script::RemoveAllEventListeners() {
  EnterCriticalSection(&m_lock);
  m_functions.clear();
  LeaveCriticalSection(&m_lock);
}

void Script::DispatchEvent(std::shared_ptr<Event> evt) {
  EnterCriticalSection(&Vars.cEventSection);
  m_EventList.push_front(evt);
  LeaveCriticalSection(&Vars.cEventSection);
  SetEvent(m_eventSignal);
}

bool Script::BlockThread(DWORD delay) {
  DWORD start = GetTickCount();
  int amt = delay - (GetTickCount() - start);
  if (m_scriptState == kScriptStateStopped) {
    Log("Script already stopped (In delay) %s", GetShortFilename());
    return false;
  }

  while (amt > 0) {  // had a script deadlock here, make sure were positve with amt
    if (m_scriptState == kScriptStateRequestStop) {
      Log("Request stop (In delay) %s", GetShortFilename());
      return false;
    }
    WaitForSingleObjectEx(m_eventSignal, amt, true);
    ResetEvent(m_eventSignal);

    if (!RunEventLoop()) {
      break;
    }

    amt = delay - (GetTickCount() - start);
  }
  return true;
}

bool Script::Initialize() {
  uv_loop_init(&m_loop);
  m_threadState.script = this;
  m_threadState.loop = &m_loop;

  // m_runtime = JS_NewRuntime();
  // JS_SetRuntimeOpaque(m_runtime, &m_threadState);
  // JS_SetInterruptHandler(m_runtime, InterruptHandler, this);
  // JS_SetMaxStackSize(m_runtime, 0);
  // JS_SetMemoryLimit(m_runtime, 100 * 1024 * 1024);

  m_context = JS_NewContext(100 * 1024 * 1024);
  JS_SetNativeStackQuota(m_context, 0);
  if (!m_context) {
    Log("Couldn't create the context");
    return false;
  }
  JS_SetContextPrivate(m_context, &m_threadState);
  JS::SetWarningReporter(m_context, JS_LogReport);
  // JS_AddInterruptCallback(m_context, InterruptHandler);

  // TODO(ejt): replace this with JS::SetEnqueuePromiseJobCallback
  // as we use libuv for event loop so promises should do that too
  if (!js::UseInternalJobQueues(m_context)) {
  }

  if (!JS::InitSelfHostedCode(m_context)) {
    Log("Error initializing selfhosted code");
    return false;
  }

  JSAutoRequest request(m_context);
  JS::CompartmentOptions global_options;
  static JSClass kGlobalClass{"D2BSGlobal", JSCLASS_GLOBAL_FLAGS, &DefaultGlobalClassOps};
  m_globalObject.init(m_context, JS_NewGlobalObject(m_context, &kGlobalClass, nullptr, JS::FireOnNewGlobalHook, global_options));

  JSAutoCompartment ac(m_context, m_globalObject);
  m_script.init(m_context);

  // JSValue console = JS_NewObject(m_context);
  // JS_SetPropertyStr(m_context, console, "trace", JS_NewCFunction(m_context, my_print, "trace", 1));
  // JS_SetPropertyStr(m_context, console, "debug", JS_NewCFunction(m_context, my_print, "debug", 1));
  // JS_SetPropertyStr(m_context, console, "log", JS_NewCFunction(m_context, my_print, "log", 1));
  // JS_SetPropertyStr(m_context, console, "error", JS_NewCFunction(m_context, my_print, "error", 1));
  // JS_SetPropertyStr(m_context, m_globalObject, "console", console);

  if (!JS_DefineFunctions(m_context, m_globalObject, global_funcs)) {
    return false;
  }

  RegisterBuiltinBindings(m_context, m_globalObject);

  // define 'me' property
  m_me = new UnitWrap::UnitData;
  memset(m_me, NULL, sizeof(UnitWrap::UnitData));

  D2UnitStrc* player = D2CLIENT_GetPlayerUnit();
  m_me->dwMode = (DWORD)-1;
  m_me->dwClassId = (DWORD)-1;
  m_me->dwType = UNIT_PLAYER;
  m_me->dwUnitId = player ? player->dwUnitId : NULL;
  m_me->dwPrivateType = PRIVATE_UNIT;

  JS::RootedObject meObject(m_context, UnitWrap::Instantiate(m_context, m_me, true));
  if (!meObject) {
    JS_ReportPendingException(m_context);
    Log("failed to build object 'me'");
    return false;
  }
  // JS_SetPropertyFunctionList(m_context, meObject, UnitWrap::me_proto_funcs, _countof(UnitWrap::me_proto_funcs));
  JS_DefineProperty(m_context, m_globalObject, "me", meObject, JSPROP_ENUMERATE | JSPROP_PERMANENT);

  // compile script file
  if (m_scriptMode == kScriptModeCommand) {
    if (strlen(Vars.szConsole) > 0) {
      m_script = JS_CompileFile(m_context, m_globalObject, m_fileName);
    } else {
      JS::CompileOptions opts(m_context);
      opts.setFileAndLine("<eval>", 1);
      const char* cmd = "function main() {print('ÿc2D2BSÿc0 :: Started Console'); while (true){delay(10000)};}  ";
      JS_CompileScript(m_context, cmd, strlen(cmd), opts, &m_script);
    }
  } else {
    m_script = JS_CompileFile(m_context, m_globalObject, m_fileName);
  }

  if (!m_script) {
    JS_ReportPendingException(m_context);
    return false;
  }

  m_scriptState = kScriptStateRunning;
  m_threadState.lastSpinTime = std::chrono::steady_clock::now();

  return true;
}

// TODO(ejt): this is kinda hacky so change this in the future
//static void __walk_loop(uv_handle_t* handle, void* /*arg*/) {
  // NOTE(ejt): as of writing this, the only uv timers are TimerWrap*
  // std::vector<TimerWrap*> wraps;
  // if (handle->type == uv_handle_type::UV_TIMER) {
  //  TimerWrap* wrap = static_cast<TimerWrap*>(handle->data);
  //  if (wrap) {
  //    wrap->Unref();
  //  }
  //}
//}

void Script::Cleanup() {
  PurgeEventList();
  Genhook::Clean(this);

  if (m_context) {
    Log("Destroying context for %s", GetShortFilename());
    m_globalObject.reset();
    m_script.reset();
    JS_DestroyContext(m_context);
    m_context = nullptr;
  }

  uv_loop_close(&m_loop);
  m_scriptState = kScriptStateStopped;

  // TODO(ejt): revisit this
  if (Vars.bDisableCache)
    sScriptEngine->DisposeScript(this);
}

void Script::RunMain() {
  JSAutoRequest ar(m_context);
  JSAutoCompartment ac(m_context, m_globalObject);
  JS::RootedValue main(m_context);
  JS::RootedValue rval(m_context);

  if (!JS_ExecuteScript(m_context, m_script)) {
    JS_ReportPendingException(m_context);
    return;
  }
  if (!JS_GetProperty(m_context, m_globalObject, "main", &main)) {
    JS_ReportPendingException(m_context);
    return;
  }
  if (!JS_CallFunctionValue(m_context, m_globalObject, main, JS::HandleValueArray::empty(), &rval)) {
    JS_ReportPendingException(m_context);
  }
  if (!rval.isUndefined()) {
    char* text = JS_EncodeString(m_context, JS::ToString(m_context, rval));
    if (text) {
      Log(text);
      JS_free(m_context, text);
    }
  }
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

  m_threadState.lastSpinTime = std::chrono::steady_clock::now();
  uv_run(&m_loop, UV_RUN_NOWAIT);
  js::RunJobs(m_context);

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

void Script::ExecuteEvent(char* evtName, const JS::AutoValueVector& args, bool* block) {
  JS::RootedValue rval(m_context);
  for (const auto& fun : m_functions[evtName]) {
    if (!JS_CallFunctionValue(m_context, m_globalObject, fun, args, &rval)) {
      JS_ReportPendingException(m_context);
      continue;
    }
    if (block) {
      *block |= static_cast<bool>(rval.isBoolean() && rval.toBoolean());
    }
  }
}

bool Script::HandleEvent(std::shared_ptr<Event> evt, bool clearList) {
  JS::AutoValueVector args(m_context);
  char* evtName = (char*)evt->name.c_str();

  if (strcmp(evtName, "itemaction") == 0) {
    if (!clearList) {
      std::shared_ptr<ItemEvent> itemEvt = std::dynamic_pointer_cast<ItemEvent>(evt);
      args.resize(4);
      args[0].set(JS_NumberValue(itemEvt->id));
      args[1].set(JS_NumberValue(itemEvt->mode));
      args[2].set(JS::StringValue(JS_NewStringCopyZ(m_context, itemEvt->code.c_str())));
      args[3].set(JS::BooleanValue(itemEvt->global));

      ExecuteEvent(evtName, args);
    }

    return true;
  }
  if (strcmp(evtName, "gameevent") == 0) {
    if (!clearList) {
      std::shared_ptr<GameEvent> gameEvt = std::dynamic_pointer_cast<GameEvent>(evt);
      args.resize(3);
      args[0].set(JS_NumberValue(gameEvt->mode));
      args[1].set(JS_NumberValue(gameEvt->param1));
      args[2].set(JS_NumberValue(gameEvt->param2));
      if (!gameEvt->name1.empty()) {
        args.append(JS::StringValue(JS_NewStringCopyZ(m_context, gameEvt->name1.c_str())));
      }
      if (!gameEvt->name2.empty()) {
        args.append(JS::StringValue(JS_NewStringCopyZ(m_context, gameEvt->name2.c_str())));
      }
      ExecuteEvent(evtName, args);
    }

    return true;
  }
  if (strcmp(evtName, "copydata") == 0) {
    if (!clearList) {
      std::shared_ptr<CopyDataMessageEvent> dataEvt = std::dynamic_pointer_cast<CopyDataMessageEvent>(evt);
      args.resize(2);
      args[0].set(JS_NumberValue(dataEvt->mode));
      if (!dataEvt->msg.empty()) {
        args[1].setString(JS_NewStringCopyZ(m_context, dataEvt->msg.c_str()));
      } else {
        args[1].setUndefined();
      }

      ExecuteEvent(evtName, args);
    }

    return true;
  }
  if (strcmp(evtName, "chatmsg") == 0 || strcmp(evtName, "chatinput") == 0 || strcmp(evtName, "whispermsg") == 0 || strcmp(evtName, "chatmsgblocker") == 0 ||
      strcmp(evtName, "chatinputblocker") == 0 || strcmp(evtName, "whispermsgblocker") == 0) {
    bool block = false;
    if (!clearList) {
      std::shared_ptr<ChatMessageEvent> chatEvt = std::dynamic_pointer_cast<ChatMessageEvent>(evt);
      args.resize(2);
      if (!chatEvt->nickname.empty()) {
        args[0].set(JS::StringValue(JS_NewStringCopyZ(m_context, chatEvt->nickname.c_str())));
      }
      if (!chatEvt->msg.empty()) {
        args[1].set(JS::StringValue(JS_NewStringCopyZ(m_context, chatEvt->msg.c_str())));
      }

      ExecuteEvent(evtName, args, &block);
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
      args.resize(2);
      args[0].setNumber(mouseEvt->x);
      args[1].setNumber(mouseEvt->y);

      ExecuteEvent(evtName, args);
    }

    return true;
  }
  if (strcmp(evtName, "ScreenHookHover") == 0) {
    std::shared_ptr<GenHookEvent> genHookEvt = std::dynamic_pointer_cast<GenHookEvent>(evt);
    if (!clearList) {
      args.resize(2);
      args[0].setNumber(genHookEvt->x);
      args[1].setNumber(genHookEvt->y);

      ExecuteEvent(evtName, args);
    }

    return true;
  }
  if (strcmp(evtName, "mouseclick") == 0) {
    if (!clearList) {
      std::shared_ptr<MouseEvent> mouseEvt = std::dynamic_pointer_cast<MouseEvent>(evt);
      args.resize(4);
      args[0].setNumber(mouseEvt->button);
      args[1].setNumber(mouseEvt->x);
      args[2].setNumber(mouseEvt->y);
      args[3].setNumber(mouseEvt->state);

      ExecuteEvent(evtName, args);
    }

    return true;
  }
  if (strcmp(evtName, "melife") == 0 || strcmp(evtName, "memana") == 0) {
    if (!clearList) {
      std::shared_ptr<HealthManaEvent> hpmpEvt = std::dynamic_pointer_cast<HealthManaEvent>(evt);
      args.resize(1);
      args[0].setNumber(hpmpEvt->value);

      ExecuteEvent(evtName, args);
    }

    return true;
  }
  if (strcmp(evtName, "playerassign") == 0) {
    if (!clearList) {
      std::shared_ptr<PlayerAssignedEvent> assignEvt = std::dynamic_pointer_cast<PlayerAssignedEvent>(evt);
      args.resize(1);
      args[0].setNumber(assignEvt->id);

      ExecuteEvent(evtName, args);
    }

    return true;
  }
  if (strcmp(evtName, "keyup") == 0 || strcmp(evtName, "keydownblocker") == 0 || strcmp(evtName, "keydown") == 0) {
    bool block = false;
    if (!clearList) {
      std::shared_ptr<KeyEvent> keyEvt = std::dynamic_pointer_cast<KeyEvent>(evt);
      args.resize(1);
      args[0].setNumber(keyEvt->key);

      ExecuteEvent(evtName, args, &block);
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
      args.resize(3);
      args[0].setNumber(genHookEvt->x);
      args[1].setNumber(genHookEvt->y);
      args[2].setNumber(genHookEvt->button);

      JS::RootedValue rval(m_context);
      // diffrent function source for hooks
      JS::RootedFunction fun(m_context, genHookEvt->callback);
      if (!JS_CallFunction(m_context, m_globalObject, fun, args, &rval)) {
        JS_ReportPendingException(m_context);
      }
      block |= static_cast<bool>(rval.isBoolean() && rval.toBoolean());
    }
    evt->block = block;
    SetEvent(Vars.eventSignal);

    return true;
  }
  if (strcmp(evtName, "Command") == 0) {
    std::shared_ptr<CommandEvent> cmdEvt = std::dynamic_pointer_cast<CommandEvent>(evt);
    std::string test;
    if (cmdEvt->command == "stack") {
      Log("Capturing stack...");
      m_debugCaptureStackFrames = true;
    }
    test.append("try{ ");
    test.append(cmdEvt->command);
    test.append(" } catch (error){print(error)}");

    JS::RootedValue rval(m_context);
    JS::CompileOptions opts(m_context);
    opts.setFileAndLine("<command line>", 1);
    if (!JS::Evaluate(m_context, opts, test.data(), test.length(), &rval)) {
      JS_ReportPendingException(m_context);
    } else if (!rval.isUndefined()) {
      char* text = JS_EncodeString(m_context, JS::ToString(m_context, rval));
      if (text) {
        Print(text);
        JS_free(m_context, text);
      }
    }
  }
  if (strcmp(evtName, "scriptmsg") == 0) {
    std::shared_ptr<ScriptMsgEvent> msgEvt = std::dynamic_pointer_cast<ScriptMsgEvent>(evt);
    if (!clearList) {
      // not sure if this is how we should use structured clone buffers
      // it works and can't find documentation of how to do it properly
      // so it will do for now unless it starts acting up
      for (auto& buffer : msgEvt->buffers) {
        JS::RootedValue val(m_context);
        buffer.read(m_context, &val);
        args.append(val);
      }

      ExecuteEvent(evtName, args);
    }
    return true;
  }
  if (strcmp(evtName, "gamepacket") == 0 || strcmp(evtName, "gamepacketsent") == 0 || strcmp(evtName, "realmpacket") == 0) {
    bool block = false;
    if (!clearList) {
      std::shared_ptr<PacketEvent> packetEvt = std::dynamic_pointer_cast<PacketEvent>(evt);
      JS::RootedObject arr(m_context, JS_NewArrayObject(m_context, packetEvt->bytes.size()));
      for (size_t i = 0; i < packetEvt->bytes.size(); i++) {
        JS_SetElement(m_context, arr, i, packetEvt->bytes[i]);
      }

      args.resize(1);
      args[0].setObject(*arr);

      ExecuteEvent(evtName, args, &block);
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
      Log("Request stop %s", GetShortFilename());
      return false;
    }

    if (m_scriptMode == kScriptModeGame && ClientState() == ClientStateMenu) {
      Log("Game mode mismatch %s", GetShortFilename());
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

// return false if the JS code needs to be interrupted
// bool Script::InterruptHandler(JSContext* ctx) {
//  ThreadState* ts = (ThreadState*)JS_GetContextPrivate(ctx);
//  Script* script = ts->script;
//
//  // auto t = std::chrono::steady_clock::now() - ts->lastSpinTime;
//  // auto tms = std::chrono::duration_cast<std::chrono::milliseconds>(t);
//  // if (tms.count() > 5000) {
//  //   // interrupt the script if it stalls for more than 5 seconds without yielding to event loop
//  //   return 1;
//  // }
//  if (!script->RunEventLoop()) {
//    return false;
//  }
//  return true;
//}

#ifdef DEBUG
#pragma pack(push, 8)
typedef struct tagTHREADNAME_INFO {
  DWORD dwType;      // Must be 0x1000.
  LPCSTR szName;     // Pointer to name (in user addr space).
  DWORD dwThreadID;  // Thread ID (-1=caller thread).
  DWORD dwFlags;     // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName(DWORD dwThreadID, const char* threadName) {
  THREADNAME_INFO info{};
  info.dwType = 0x1000;
  info.szName = threadName;
  info.dwThreadID = dwThreadID;
  info.dwFlags = 0;

  __try {
    RaiseException(0x406D1388, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
  } __except (EXCEPTION_EXECUTE_HANDLER) {
  }
}
#endif

DWORD WINAPI ScriptThread(LPVOID lpThreadParameter) {
  Script* script = static_cast<Script*>(lpThreadParameter);
  // TODO(ejt): necessary? script should ALWAYS be here otherwise it's a critical error!
  if (script) {
#ifdef DEBUG
    SetThreadName(GetCurrentThreadId(), script->GetShortFilename());
#endif
    script->Run();
  }
  return 0;
}
