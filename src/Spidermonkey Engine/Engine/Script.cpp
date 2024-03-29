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
#include "StringWrap.h"

#include <chrono>
#include <cassert>

bool kScriptUseModules = false;

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
  if (JS_ObjectIsFunction(evtFunc.toObjectOrNull()) && strlen(evtName) > 0) {
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

  JS::RealmOptions realm_options;
  static JSClass kGlobalClass{"D2BSGlobal", JSCLASS_GLOBAL_FLAGS, &DefaultGlobalClassOps};
  m_globalObject.init(m_context, JS_NewGlobalObject(m_context, &kGlobalClass, nullptr, JS::FireOnNewGlobalHook, realm_options));

  JSAutoRealm ac(m_context, m_globalObject);
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
      std::string cmd = "function main() {print('ÿc2D2BSÿc0 :: Started Console'); while (true){delay(10000)};}  ";
      JS::SourceText<mozilla::Utf8Unit> source;
      source.init(m_context, cmd.c_str(), cmd.length(), JS::SourceOwnership::Borrowed);
      m_script.set(JS::Compile(m_context, opts, source));
    }
  } else if (kScriptUseModules) {
    m_threadState.moduleEntry = m_fileName;
    m_threadState.moduleRoot = m_threadState.moduleEntry.parent_path();
    Log("%s", m_threadState.moduleRoot.string().c_str());
    Log("%s", m_threadState.moduleEntry.string().c_str());
    if (!SetupModuleResolveHook(m_context)) {
      Log("failed to setup module resolve hook");
      return false;
    }
    if (!EvaluateModule(m_context, m_threadState.moduleEntry)) {
      Log("EvaluateModule failed");
      JS_ReportPendingException(m_context);
      return false;
    }
  } else {
    m_script = JS_CompileFile(m_context, m_globalObject, m_fileName);

    if (!m_script) {
      JS_ReportPendingException(m_context);
      return false;
    }
  }

  m_scriptState = kScriptStateRunning;
  m_threadState.lastSpinTime = std::chrono::steady_clock::now();

  return true;
}

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
  JSAutoRealm ar(m_context, m_globalObject);

  if (kScriptUseModules) {
    for (;;) {
      if (m_scriptState == kScriptStateRequestStop) {
        Log("Request stop (modules) %s", GetShortFilename());
        return;
      }
      WaitForSingleObjectEx(m_eventSignal, 1, true);
      ResetEvent(m_eventSignal);

      if (m_isPaused) {
        // take a small break so we dont eat up the CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        continue;
      }
      // the normal event loop block when paused so have to
      // have the event stuff in here instead
      m_threadState.lastSpinTime = std::chrono::steady_clock::now();
      uv_run(&m_loop, UV_RUN_NOWAIT);
      js::RunJobs(m_context);

      // run loop until there are no more events or script is interrupted
      ProcessAllEvents();
    }
  } else {
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
      StringWrap text(m_context, JS::ToString(m_context, rval));
      if (text) {
        Log(text.c_str());
      }
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

void Script::ExecuteEvent(char* evtName, const JS::HandleValueArray& args, bool* block) {
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
  char* evtName = (char*)evt->name.c_str();

  if (strcmp(evtName, "itemaction") == 0) {
    if (!clearList) {
      std::shared_ptr<ItemEvent> itemEvt = std::dynamic_pointer_cast<ItemEvent>(evt);
      JS::RootedValueArray<4> args(m_context);
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
      JS::RootedValueArray<5> args(m_context);
      args[0].set(JS_NumberValue(gameEvt->mode));
      args[1].set(JS_NumberValue(gameEvt->param1));
      args[2].set(JS_NumberValue(gameEvt->param2));
      args[3].setUndefined();
      args[4].setUndefined();
      if (!gameEvt->name1.empty()) {
        args[3].setString(JS_NewStringCopyZ(m_context, gameEvt->name1.c_str()));
      }
      if (!gameEvt->name2.empty()) {
        args[3].setString(JS_NewStringCopyZ(m_context, gameEvt->name2.c_str()));
      }
      ExecuteEvent(evtName, args);
    }

    return true;
  }
  if (strcmp(evtName, "copydata") == 0) {
    if (!clearList) {
      std::shared_ptr<CopyDataMessageEvent> dataEvt = std::dynamic_pointer_cast<CopyDataMessageEvent>(evt);
      JS::RootedValueArray<2> args(m_context);
      args[0].set(JS_NumberValue(dataEvt->mode));
      args[1].setUndefined();
      if (!dataEvt->msg.empty()) {
        args[1].setString(JS_NewStringCopyZ(m_context, dataEvt->msg.c_str()));
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
      JS::RootedValueArray<2> args(m_context);
      args[0].setUndefined();
      args[1].setUndefined();
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
      JS::RootedValueArray<2> args(m_context);
      args[0].setNumber(mouseEvt->x);
      args[1].setNumber(mouseEvt->y);

      ExecuteEvent(evtName, args);
    }

    return true;
  }
  if (strcmp(evtName, "ScreenHookHover") == 0) {
    std::shared_ptr<GenHookEvent> genHookEvt = std::dynamic_pointer_cast<GenHookEvent>(evt);
    if (!clearList) {
      JS::RootedValueArray<2> args(m_context);
      args[0].setNumber(genHookEvt->x);
      args[1].setNumber(genHookEvt->y);

      ExecuteEvent(evtName, args);
    }

    return true;
  }
  if (strcmp(evtName, "mouseclick") == 0) {
    if (!clearList) {
      std::shared_ptr<MouseEvent> mouseEvt = std::dynamic_pointer_cast<MouseEvent>(evt);
      JS::RootedValueArray<4> args(m_context);
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
      JS::RootedValueArray<1> args(m_context);
      args[0].setNumber(hpmpEvt->value);

      ExecuteEvent(evtName, args);
    }

    return true;
  }
  if (strcmp(evtName, "playerassign") == 0) {
    if (!clearList) {
      std::shared_ptr<PlayerAssignedEvent> assignEvt = std::dynamic_pointer_cast<PlayerAssignedEvent>(evt);
      JS::RootedValueArray<1> args(m_context);
      args[0].setNumber(assignEvt->id);

      ExecuteEvent(evtName, args);
    }

    return true;
  }
  if (strcmp(evtName, "keyup") == 0 || strcmp(evtName, "keydownblocker") == 0 || strcmp(evtName, "keydown") == 0) {
    bool block = false;
    if (!clearList) {
      std::shared_ptr<KeyEvent> keyEvt = std::dynamic_pointer_cast<KeyEvent>(evt);
      JS::RootedValueArray<1> args(m_context);
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
      JS::RootedValueArray<3> args(m_context);
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
    JS::SourceText<mozilla::Utf8Unit> source;
    source.init(m_context, test.c_str(), test.length(), JS::SourceOwnership::Borrowed);

    JS::RootedValue rval(m_context);
    JS::CompileOptions opts(m_context);
    opts.setFileAndLine("<command line>", 1);
    if (!JS::Evaluate(m_context, opts, source, &rval)) {
      JS_ReportPendingException(m_context);
    } else if (!rval.isUndefined()) {
      StringWrap text(m_context, JS::ToString(m_context, rval));
      if (text) {
        Print(text.c_str());
      }
    }
  }
  if (strcmp(evtName, "scriptmsg") == 0) {
    std::shared_ptr<ScriptMsgEvent> msgEvt = std::dynamic_pointer_cast<ScriptMsgEvent>(evt);
    if (!clearList) {
      JS::RootedValueVector args(m_context);
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
      JS::RootedObject arr(m_context, JS::NewArrayObject(m_context, packetEvt->bytes.size()));
      for (size_t i = 0; i < packetEvt->bytes.size(); i++) {
        JS_SetElement(m_context, arr, i, packetEvt->bytes[i]);
      }

      JS::RootedValueArray<1> args(m_context);
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

static JSObject* CompileModule(JSContext* ctx, const JS::ReadOnlyCompileOptions& options, const std::u16string& code) {
  JS::SourceText<char16_t> srcBuf;
  if (!srcBuf.init(ctx, code.c_str(), code.length(), JS::SourceOwnership::Borrowed)) {
    return nullptr;
  }

  return JS::CompileModule(ctx, options, srcBuf);
}

// let requestedModule = HostResolveImportedModule(module, specifier)
static JSObject* ModuleResolveHook(JSContext* ctx, JS::HandleValue modulePrivate, JS::HandleString spec) {
  ThreadState* ts = static_cast<ThreadState*>(JS_GetContextPrivate(ctx));

  // step 1: get the host defined field of the module
  StringWrap moduleStr(ctx, modulePrivate.toString());
  if (!moduleStr) {
    return nullptr;
  }
  std::filesystem::path moduleRoot = moduleStr.c_str();
  // remove filename part as we store full path
  moduleRoot.remove_filename();

  // step 2: get the request specifier i.e the 'from' part
  JS::RootedString specifierString(ctx, spec);
  StringWrap tmp(ctx, specifierString);
  if (!tmp) {
    return nullptr;
  }
  std::string specifier(tmp.c_str());

  // step 3: verify that specifier is relative i.e. ./file1.js or ../file1.js
  while (!specifier.empty()) {
    // starts with /
    if (specifier[0] == '/') {
      specifier.erase(0, 1);
      continue;
    }
    // starts with ./
    if (specifier.length() >= 2 && specifier[0] == '.' && specifier[1] == '/') {
      specifier.erase(0, 2);
      continue;
    }
    break;
  }

  std::filesystem::path resolved = moduleRoot / specifier;
  resolved = std::filesystem::absolute(resolved);
  if (!resolved.has_extension()) {
    resolved.replace_extension(".js");
  }

  // step 4: box the specifier to our root directory
  auto [end, _] = std::mismatch(ts->moduleRoot.begin(), ts->moduleRoot.end(), resolved.begin());
  if (end != ts->moduleRoot.end()) {
    JS_ReportErrorUTF8(ctx, "module specifier did not resolve inside root directory");
    return nullptr;
  }

  // step 5: check if module is already loaded
  if (ts->moduleRegistry.contains(resolved.string())) {
    return ts->moduleRegistry[resolved.string()].get();
  }

  // step 6: load the module
  std::string resolvedStr = resolved.string();
  JS::CompileOptions options(ctx);
  options.setFileAndLine(resolvedStr.c_str(), 1);
  std::optional<std::u16string> code = ReadFileUTF16(resolved);
  if (!code) {
    JS_ReportErrorUTF8(ctx, "failed to open file %s", resolvedStr.c_str());
    return nullptr;
  }
  JS::RootedObject moduleObject(ctx, CompileModule(ctx, options, code.value()));
  if (!moduleObject) {
    //JS_ReportErrorUTF8(ctx, "could not compile module %s", resolvedStr.c_str());
    return nullptr;
  }
  // set custom data on module
  JS::RootedValue resolvedValue(ctx);
  resolvedValue.setString(JS_NewUCStringCopyZ(ctx, (const char16_t*)resolved.c_str()));
  JS::SetModulePrivate(moduleObject, resolvedValue);

  // add to registry and return
  ts->moduleRegistry.emplace(resolved.string(), JS::PersistentRootedObject(ctx, moduleObject));
  return moduleObject;
}

bool Script::SetupModuleResolveHook(JSContext* ctx) {
  JS::SetModuleResolveHook(JS_GetRuntime(ctx), ModuleResolveHook);
  return true;
}

JSObject* Script::EvaluateModule(JSContext* ctx, const std::filesystem::path& entry) {
  std::string filenameStr = entry.string();
  std::optional<std::u16string> code = ReadFileUTF16(entry);
  if (!code) {
    JS_ReportErrorUTF8(ctx, "failed to open file %s", entry.string().c_str());
    return nullptr;
  }

  JS::CompileOptions options(ctx);
  options.setFileAndLine(filenameStr.c_str(), 1);
  JS::RootedObject moduleRecord(ctx, CompileModule(ctx, options, code.value()));
  if (!moduleRecord) {
    return nullptr;
  }

  // set our custom data on the module record
  JS::RootedValue filenameValue(ctx, JS::StringValue(JS_NewUCStringCopyZ(ctx, (const char16_t*)entry.c_str())));
  JS::SetModulePrivate(moduleRecord, filenameValue);

  if (!JS::ModuleInstantiate(ctx, moduleRecord)) {
    return nullptr;
  }
  if (!JS::ModuleEvaluate(ctx, moduleRecord)) {
    return nullptr;
  }
  return moduleRecord;
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
