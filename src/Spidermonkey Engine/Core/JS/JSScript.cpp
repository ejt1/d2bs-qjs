#include "JSScript.h"
#include "Script.h"
#include "ScriptEngine.h"
#include "D2BS.h"
#include "Helpers.h"

EMPTY_CTOR(script)

struct FindHelper {
  DWORD tid;
  wchar_t* name;
  Script* script;
};

bool __fastcall FindScriptByTid(Script* script, void* argv, uint argc);
bool __fastcall FindScriptByName(Script* script, void* argv, uint argc);

JSAPI_PROP(script_getProperty) {
  Script* script = (Script*)JS_GetInstancePrivate(cx, obj, &script_class, NULL);

  // TODO: make this check stronger
  if (!script)
    return JS_TRUE;
  jsval ID;
  JS_IdToValue(cx, id, &ID);
  // char* nShortFilename = NULL;

  switch (JSVAL_TO_INT(ID)) {
    case SCRIPT_FILENAME:
      vp.setString(JS_InternUCString(cx, script->GetShortFilename()));
      break;
    case SCRIPT_GAMETYPE:
      vp.setBoolean(script->GetMode() == kScriptModeGame ? false : true);
      break;
    case SCRIPT_RUNNING:
      vp.setBoolean(script->IsRunning());
      break;
    case SCRIPT_THREADID:
      vp.setInt32(script->GetThreadId());
      break;
    case SCRIPT_MEMORY:
      vp.setInt32(JS_GetGCParameter(script->GetRuntime(), JSGC_BYTES));
      break;
    default:
      break;
  }

  return JS_TRUE;
}

JSAPI_FUNC(script_getNext) {
  (argc);

  Script* iterp = (Script*)JS_GetInstancePrivate(cx, JS_THIS_OBJECT(cx, vp), &script_class, NULL);
  sScriptEngine->LockScriptList("scrip.getNext");
  // EnterCriticalSection(&sScriptEngine->lock);

  for (ScriptMap::iterator it = sScriptEngine->scripts().begin(); it != sScriptEngine->scripts().end(); it++) {
    if (it->second == iterp) {
      it++;
      if (it == sScriptEngine->scripts().end())
        break;
      iterp = it->second;
      JS_SetPrivate(cx, JS_THIS_OBJECT(cx, vp), iterp);
      JS_SET_RVAL(cx, vp, JSVAL_TRUE);
      sScriptEngine->UnLockScriptList("scrip.getNext");
      // LeaveCriticalSection(&sScriptEngine->lock);
      return JS_TRUE;
    }
  }

  // LeaveCriticalSection(&sScriptEngine->lock);
  sScriptEngine->UnLockScriptList("scrip.getNext");

  JS_SET_RVAL(cx, vp, JSVAL_VOID);
  return JS_TRUE;
}

JSAPI_FUNC(script_stop) {
  (argc);

  JS_SET_RVAL(cx, vp, JSVAL_NULL);
  Script* script = (Script*)JS_GetInstancePrivate(cx, JS_THIS_OBJECT(cx, vp), &script_class, NULL);
  if (script->IsRunning())
    script->Stop();

  return JS_TRUE;
}

JSAPI_FUNC(script_pause) {
  (argc);

  JS_SET_RVAL(cx, vp, JSVAL_NULL);
  Script* script = (Script*)JS_GetInstancePrivate(cx, JS_THIS_OBJECT(cx, vp), &script_class, NULL);

  if (script->IsRunning())
    script->Pause();

  return JS_TRUE;
}

JSAPI_FUNC(script_resume) {
  (argc);

  JS_SET_RVAL(cx, vp, JSVAL_NULL);
  Script* script = (Script*)JS_GetInstancePrivate(cx, JS_THIS_OBJECT(cx, vp), &script_class, NULL);

  script->Resume();

  return JS_TRUE;
}

JSAPI_FUNC(script_send) {
  JS_SET_RVAL(cx, vp, JSVAL_NULL);
  Script* script = (Script*)JS_GetInstancePrivate(cx, JS_THIS_OBJECT(cx, vp), &script_class, NULL);
  Event* evt = new Event;
  if (!script || !script->IsRunning())
    return JS_TRUE;
  sScriptEngine->LockScriptList("script.send");
  evt->owner = script;
  evt->argc = argc;
  evt->name = "scriptmsg";
  evt->arg1 = new DWORD(argc);
  evt->argv = new JSAutoStructuredCloneBuffer*[argc];
  for (uint i = 0; i < argc; i++) {
    evt->argv[i] = new JSAutoStructuredCloneBuffer;
    evt->argv[i]->write(cx, JS_ARGV(cx, vp)[i]);
  }

  evt->owner->FireEvent(evt);
  sScriptEngine->UnLockScriptList("script.send");

  return JS_TRUE;
}

JSAPI_FUNC(script_join) {
  (argc);

  JS_SET_RVAL(cx, vp, JSVAL_NULL);
  Script* script = (Script*)JS_GetInstancePrivate(cx, JS_THIS_OBJECT(cx, vp), &script_class, NULL);

  script->Join();

  return JS_TRUE;
}

JSAPI_FUNC(my_getScript) {
  JS_SET_RVAL(cx, vp, JSVAL_NULL);
  Script* iterp = NULL;
  if (argc == 1 && JSVAL_IS_BOOLEAN(JS_ARGV(cx, vp)[0]) && JSVAL_TO_BOOLEAN(JS_ARGV(cx, vp)[0]) == JS_TRUE)
    iterp = (Script*)JS_GetContextPrivate(cx);
  else if (argc == 1 && JSVAL_IS_INT(JS_ARGV(cx, vp)[0])) {
    // loop over the Scripts in ScriptEngine and find the one with the right threadid
    DWORD tid = (DWORD)JSVAL_TO_INT(JS_ARGV(cx, vp)[0]);
    FindHelper args = {tid, NULL, NULL};
    sScriptEngine->ForEachScript(FindScriptByTid, &args, 1);
    if (args.script != NULL)
      iterp = args.script;
    else
      return JS_TRUE;
  } else if (argc == 1 && JSVAL_IS_STRING(JS_ARGV(cx, vp)[0])) {
    wchar_t* name = _wcsdup(JS_GetStringCharsZ(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[0])));
    if (name)
      StringReplace(name, L'/', L'\\', wcslen(name));
    FindHelper args = {0, name, NULL};
    sScriptEngine->ForEachScript(FindScriptByName, &args, 1);
    free(name);
    if (args.script != NULL)
      iterp = args.script;
    else
      return JS_TRUE;
  } else {
    if (sScriptEngine->scripts().size() > 0) {
      //	EnterCriticalSection(&sScriptEngine->lock);
      sScriptEngine->LockScriptList("getScript");
      iterp = sScriptEngine->scripts().begin()->second;
      sScriptEngine->UnLockScriptList("getScript");
      //	LeaveCriticalSection(&sScriptEngine->lock);
    }

    if (iterp == NULL)
      return JS_TRUE;
  }

  JSObject* res = BuildObject(cx, &script_class, script_methods, script_props, iterp);

  if (!res)
    THROW_ERROR(cx, "Failed to build the script object");
  JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(res));

  return JS_TRUE;
}
JSAPI_FUNC(my_getScripts) {
  (argc);

  DWORD dwArrayCount = NULL;

  JSObject* pReturnArray = JS_NewArrayObject(cx, 0, NULL);
  JS_BeginRequest(cx);
  JS_AddRoot(cx, &pReturnArray);
  sScriptEngine->LockScriptList("getScripts");

  for (ScriptMap::iterator it = sScriptEngine->scripts().begin(); it != sScriptEngine->scripts().end(); it++) {
    JSObject* res = BuildObject(cx, &script_class, script_methods, script_props, it->second);
    jsval a = OBJECT_TO_JSVAL(res);
    JS_SetElement(cx, pReturnArray, dwArrayCount, &a);
    dwArrayCount++;
  }

  sScriptEngine->UnLockScriptList("getScripts");
  JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(pReturnArray));
  JS_RemoveRoot(cx, &pReturnArray);
  JS_EndRequest(cx);
  return JS_TRUE;
}
bool __fastcall FindScriptByName(Script* script, void* argv, uint /*argc*/) {
  FindHelper* helper = (FindHelper*)argv;
  // static uint pathlen = wcslen(Vars.szScriptPath) + 1;
  const wchar_t* fname = script->GetShortFilename();
  if (_wcsicmp(fname, helper->name) == 0) {
    helper->script = script;
    return false;
  }
  return true;
}

bool __fastcall FindScriptByTid(Script* script, void* argv, uint /*argc*/) {
  FindHelper* helper = (FindHelper*)argv;
  if (script->GetThreadId() == helper->tid) {
    helper->script = script;
    return false;
  }
  return true;
}
