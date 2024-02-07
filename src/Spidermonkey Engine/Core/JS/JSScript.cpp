#include "JSScript.h"
#include "Script.h"
#include "ScriptEngine.h"
#include "Engine.h"
#include "Helpers.h"

EMPTY_CTOR(script)

struct FindHelper {
  DWORD tid;
  char* name;
  Script* script;
};

bool __fastcall FindScriptByTid(Script* script, void* argv, uint32_t argc);
bool __fastcall FindScriptByName(Script* script, void* argv, uint32_t argc);

JSAPI_PROP(script_getProperty) {
  Script* script = (Script*)JS_GetOpaque3(this_val);

  // TODO: make this check stronger
  if (!script)
    return JS_UNDEFINED;

  switch (magic) {
    case SCRIPT_FILENAME: {
      return JS_NewString(ctx, script->GetShortFilename());
    } break;
    case SCRIPT_GAMETYPE:
      return JS_NewBool(ctx, script->GetMode() == kScriptModeGame ? false : true);
      break;
    case SCRIPT_RUNNING:
      return JS_NewBool(ctx, script->IsRunning());
      break;
    case SCRIPT_THREADID:
      return JS_NewInt32(ctx, script->GetThreadId());
      break;
    case SCRIPT_MEMORY: {
      JSMemoryUsage mem;
      JS_ComputeMemoryUsage(script->GetRuntime(), &mem);
      // TODO(ejt): not sure what best represent mozjs GC_BYTES but lets go with this for now
      return JS_NewInt64(ctx, mem.memory_used_size);
    } break;
    default:
      break;
  }

  return JS_UNDEFINED;
}

JSAPI_FUNC(script_getNext) {
  Script* iterp = (Script*)JS_GetOpaque3(this_val);
  sScriptEngine->LockScriptList("scrip.getNext");

  for (ScriptMap::iterator it = sScriptEngine->scripts().begin(); it != sScriptEngine->scripts().end(); it++) {
    if (it->second == iterp) {
      it++;
      if (it == sScriptEngine->scripts().end())
        break;
      iterp = it->second;
      JS_SetOpaque(this_val, iterp);
      sScriptEngine->UnLockScriptList("scrip.getNext");
      return JS_TRUE;
    }
  }

  sScriptEngine->UnLockScriptList("scrip.getNext");

  return JS_UNDEFINED;
}

JSAPI_FUNC(script_stop) {
  Script* script = (Script*)JS_GetOpaque3(this_val);
  if (script->IsRunning())
    script->Stop();

  return JS_NULL;
}

JSAPI_FUNC(script_pause) {
  Script* script = (Script*)JS_GetOpaque3(this_val);

  if (script->IsRunning())
    script->Pause();

  return JS_NULL;
}

JSAPI_FUNC(script_resume) {
  Script* script = (Script*)JS_GetOpaque3(this_val);

  script->Resume();

  return JS_NULL;
}

JSAPI_FUNC(script_send) {
  if (argc > 1) {
    THROW_ERROR(ctx, "too many arguments");
  }

  Script* script = (Script*)JS_GetOpaque3(this_val);
  if (!script || !script->IsRunning())
    return JS_NULL;

  if (!ScriptMessageEvent(ctx, script, argv[0])) {
    return JS_EXCEPTION;
  }

  return JS_NULL;
}

JSAPI_FUNC(script_join) {
  Script* script = (Script*)JS_GetOpaque3(this_val);

  script->Join();

  return JS_NULL;
}

JSAPI_FUNC(my_getScript) {
  Script* iterp = NULL;
  if (argc == 1 && JS_IsBool(argv[0]) && JS_ToBool(ctx, argv[0]) == TRUE)
    iterp = (Script*)JS_GetContextOpaque(ctx);
  else if (argc == 1 && JS_IsNumber(argv[0])) {
    // loop over the Scripts in ScriptEngine and find the one with the right threadid
    uint32_t tid;
    if (JS_ToUint32(ctx, &tid, argv[0])) {
      return JS_EXCEPTION;
    }
    FindHelper args = {tid, NULL, NULL};
    sScriptEngine->ForEachScript(FindScriptByTid, &args, 1);
    if (args.script != NULL)
      iterp = args.script;
    else
      return JS_NULL;
  } else if (argc == 1 && JS_IsString(argv[0])) {
    const char* name = JS_ToCString(ctx, argv[0]);
    if (!name) {
      return JS_EXCEPTION;
    }
    char* fname = _strdup(name);
    JS_FreeCString(ctx, name);
    StringReplace(fname, '/', '\\', strlen(fname));
    FindHelper args = {0, fname, NULL};
    sScriptEngine->ForEachScript(FindScriptByName, &args, 1);
    free(fname);
    if (args.script != NULL)
      iterp = args.script;
    else
      return JS_NULL;
  } else {
    if (sScriptEngine->scripts().size() > 0) {
      //	EnterCriticalSection(&sScriptEngine->lock);
      sScriptEngine->LockScriptList("getScript");
      iterp = sScriptEngine->scripts().begin()->second;
      sScriptEngine->UnLockScriptList("getScript");
      //	LeaveCriticalSection(&sScriptEngine->lock);
    }

    if (iterp == NULL)
      return JS_NULL;
  }

  JSValue res = BuildObject(ctx, script_class_id, FUNCLIST(script_proto_funcs), iterp);

  if (JS_IsException(res)) {
    THROW_ERROR(ctx, "Failed to build the script object");
  }
  return res;
}

JSAPI_FUNC(my_getScripts) {
  DWORD dwArrayCount = NULL;

  JSValue pReturnArray = JS_NewArray(ctx);
  sScriptEngine->LockScriptList("getScripts");

  for (ScriptMap::iterator it = sScriptEngine->scripts().begin(); it != sScriptEngine->scripts().end(); it++) {
    JSValue res = BuildObject(ctx, script_class_id, FUNCLIST(script_proto_funcs), it->second);
    JS_SetPropertyUint32(ctx, pReturnArray, dwArrayCount, res);
    dwArrayCount++;
  }

  sScriptEngine->UnLockScriptList("getScripts");
  return pReturnArray;
}

bool __fastcall FindScriptByName(Script* script, void* argv, uint32_t /*argc*/) {
  FindHelper* helper = (FindHelper*)argv;
  // static uint32_t pathlen = wcslen(Vars.szScriptPath) + 1;
  const char* fname = script->GetShortFilename();
  if (_stricmp(fname, helper->name) == 0) {
    helper->script = script;
    return false;
  }
  return true;
}

bool __fastcall FindScriptByTid(Script* script, void* argv, uint32_t /*argc*/) {
  FindHelper* helper = (FindHelper*)argv;
  if (script->GetThreadId() == helper->tid) {
    helper->script = script;
    return false;
  }
  return true;
}
