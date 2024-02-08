#include "JSScript.h"

#include "Bindings.h"
#include "Localization.h"
#include "ScriptEngine.h"

struct FindHelper {
  DWORD tid;
  char* name;
  Script* script;
};

static bool __fastcall FindScriptByName(Script* script, void* argv) {
  FindHelper* helper = (FindHelper*)argv;
  // static uint32_t pathlen = wcslen(Vars.szScriptPath) + 1;
  const char* fname = script->GetShortFilename();
  if (_stricmp(fname, helper->name) == 0) {
    helper->script = script;
    return false;
  }
  return true;
}

static bool __fastcall FindScriptByTid(Script* script, void* argv) {
  FindHelper* helper = (FindHelper*)argv;
  if (script->GetThreadId() == helper->tid) {
    helper->script = script;
    return false;
  }
  return true;
}

JSValue ScriptWrap::Instantiate(JSContext* ctx, JSValue new_target, Script* script) {
  JSValue proto;
  if (JS_IsUndefined(new_target)) {
    proto = JS_GetClassProto(ctx, m_class_id);
  } else {
    proto = JS_GetPropertyStr(ctx, new_target, "prototype");
    if (JS_IsException(proto)) {
      return JS_EXCEPTION;
    }
  }
  JSValue obj = JS_NewObjectProtoClass(ctx, proto, m_class_id);
  JS_FreeValue(ctx, proto);
  if (JS_IsException(obj)) {
    return obj;
  }

  ScriptWrap* wrap = new ScriptWrap(ctx, script);
  if (!wrap) {
    JS_FreeValue(ctx, obj);
    return JS_ThrowOutOfMemory(ctx);
  }
  JS_SetOpaque(obj, wrap);

  return obj;
}

void ScriptWrap::Initialize(JSContext* ctx, JSValue target) {
  JSClassDef def{};
  def.class_name = "D2BSScript";
  def.finalizer = [](JSRuntime* /*rt*/, JSValue val) {
    ScriptWrap* opaque = static_cast<ScriptWrap*>(JS_GetOpaque(val, m_class_id));
    if (opaque) {
      delete opaque;
    }
  };

  if (m_class_id == 0) {
    JS_NewClassID(&m_class_id);
  }
  JS_NewClass(JS_GetRuntime(ctx), m_class_id, &def);

  JSValue proto = JS_NewObject(ctx);
  JS_SetPropertyFunctionList(ctx, proto, m_proto_funcs, _countof(m_proto_funcs));

  JSValue obj = JS_NewObjectProtoClass(ctx, proto, m_class_id);
  JS_SetClassProto(ctx, m_class_id, proto);
  JS_SetPropertyStr(ctx, target, "D2BSScript", obj);

  // globals
  JS_SetPropertyStr(ctx, target, "getScript", JS_NewCFunction(ctx, GetScript, "getScript", 0));
  JS_SetPropertyStr(ctx, target, "getScripts", JS_NewCFunction(ctx, GetScripts, "getScripts", 0));
}

ScriptWrap::ScriptWrap(JSContext* /*ctx*/, Script* script) : m_script(script) {
}

// properties
JSValue ScriptWrap::GetName(JSContext* ctx, JSValue this_val) {
  ScriptWrap* wrap = static_cast<ScriptWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap || !wrap->m_script) {
    return JS_EXCEPTION;
  }
  Script* script = wrap->m_script;

  return JS_NewString(ctx, script->GetShortFilename());
}

JSValue ScriptWrap::GetType(JSContext* ctx, JSValue this_val) {
  ScriptWrap* wrap = static_cast<ScriptWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap || !wrap->m_script) {
    return JS_EXCEPTION;
  }
  Script* script = wrap->m_script;

  return JS_NewBool(ctx, script->GetMode() == kScriptModeGame ? false : true);
}

JSValue ScriptWrap::GetRunning(JSContext* ctx, JSValue this_val) {
  ScriptWrap* wrap = static_cast<ScriptWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap || !wrap->m_script) {
    return JS_EXCEPTION;
  }
  Script* script = wrap->m_script;

  return JS_NewBool(ctx, script->IsRunning());
}

JSValue ScriptWrap::GetThreadId(JSContext* ctx, JSValue this_val) {
  ScriptWrap* wrap = static_cast<ScriptWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap || !wrap->m_script) {
    return JS_EXCEPTION;
  }
  Script* script = wrap->m_script;

  return JS_NewInt32(ctx, script->GetThreadId());
}

JSValue ScriptWrap::GetMemory(JSContext* ctx, JSValue this_val) {
  ScriptWrap* wrap = static_cast<ScriptWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap || !wrap->m_script) {
    return JS_EXCEPTION;
  }
  Script* script = wrap->m_script;

  JSMemoryUsage mem;
  JS_ComputeMemoryUsage(script->GetRuntime(), &mem);
  return JS_NewInt64(ctx, mem.memory_used_size);
}

// functions
JSValue ScriptWrap::GetNext(JSContext* ctx, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  ScriptWrap* wrap = static_cast<ScriptWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap || !wrap->m_script) {
    return JS_EXCEPTION;
  }
  Script* script = wrap->m_script;

  sScriptEngine->LockScriptList("scrip.getNext");
  for (ScriptMap::iterator it = sScriptEngine->scripts().begin(); it != sScriptEngine->scripts().end(); it++) {
    if (it->second == script) {
      it++;
      if (it == sScriptEngine->scripts().end())
        break;

      JSValue r = ScriptWrap::Instantiate(ctx, JS_UNDEFINED, it->second);
      sScriptEngine->UnLockScriptList("scrip.getNext");
      return r;
    }
  }

  sScriptEngine->UnLockScriptList("scrip.getNext");

  return JS_UNDEFINED;
}

JSValue ScriptWrap::Pause(JSContext* /*ctx*/, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  ScriptWrap* wrap = static_cast<ScriptWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap || !wrap->m_script) {
    return JS_EXCEPTION;
  }
  Script* script = wrap->m_script;

  if (script->IsRunning())
    script->Pause();
  return JS_NULL;
}

JSValue ScriptWrap::Resume(JSContext* /*ctx*/, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  ScriptWrap* wrap = static_cast<ScriptWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap || !wrap->m_script) {
    return JS_EXCEPTION;
  }
  Script* script = wrap->m_script;

  script->Resume();
  return JS_NULL;
}

JSValue ScriptWrap::Stop(JSContext* /*ctx*/, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  ScriptWrap* wrap = static_cast<ScriptWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap || !wrap->m_script) {
    return JS_EXCEPTION;
  }
  Script* script = wrap->m_script;

  if (script->IsRunning())
    script->Stop();
  return JS_NULL;
}

JSValue ScriptWrap::Join(JSContext* /*ctx*/, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  ScriptWrap* wrap = static_cast<ScriptWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap || !wrap->m_script) {
    return JS_EXCEPTION;
  }
  Script* script = wrap->m_script;

  script->Join();
  return JS_NULL;
}

JSValue ScriptWrap::Send(JSContext* ctx, JSValue this_val, int argc, JSValue* argv) {
  if (argc > 1) {
    THROW_ERROR(ctx, "too many arguments");
  }

  ScriptWrap* wrap = static_cast<ScriptWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap || !wrap->m_script) {
    return JS_EXCEPTION;
  }
  Script* script = wrap->m_script;

  if (!script || !script->IsRunning())
    return JS_NULL;

  if (!ScriptMessageEvent(ctx, script, argv[0])) {
    return JS_EXCEPTION;
  }

  return JS_NULL;
}

JSValue ScriptWrap::GetScript(JSContext* ctx, JSValue /*this_val*/, int argc, JSValue* argv) {
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
    sScriptEngine->ForEachScript(FindScriptByTid, &args);
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
    sScriptEngine->ForEachScript(FindScriptByName, &args);
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

  return ScriptWrap::Instantiate(ctx, JS_UNDEFINED, iterp);
}

JSValue ScriptWrap::GetScripts(JSContext* ctx, JSValue /*this_val*/, int /*argc*/, JSValue* /*argv*/) {
  DWORD dwArrayCount = NULL;

  JSValue pReturnArray = JS_NewArray(ctx);
  sScriptEngine->LockScriptList("getScripts");

  for (ScriptMap::iterator it = sScriptEngine->scripts().begin(); it != sScriptEngine->scripts().end(); it++) {
    JSValue res = ScriptWrap::Instantiate(ctx, JS_UNDEFINED, it->second);
    JS_SetPropertyUint32(ctx, pReturnArray, dwArrayCount, res);
    dwArrayCount++;
  }

  sScriptEngine->UnLockScriptList("getScripts");
  return pReturnArray;
}

D2BS_BINDING_INTERNAL(ScriptWrap, ScriptWrap::Initialize)