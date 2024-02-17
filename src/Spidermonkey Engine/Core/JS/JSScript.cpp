#include "JSScript.h"

#include "Bindings.h"
#include "D2Helpers.h"
#include "Localization.h"
#include "ScriptEngine.h"
#include "StringWrap.h"

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

JSObject* ScriptWrap::Instantiate(JSContext* ctx, Script* script) {
  JS::RootedObject global(ctx, JS::CurrentGlobalOrNull(ctx));
  JS::RootedValue constructor_val(ctx);
  if (!JS_GetProperty(ctx, global, "D2BSScript", &constructor_val)) {
    JS_ReportErrorASCII(ctx, "Could not find constructor object for D2BSScript");
    return nullptr;
  }
  if (!constructor_val.isObject()) {
    JS_ReportErrorASCII(ctx, "D2BSScript is not a constructor");
    return nullptr;
  }
  JS::RootedObject constructor(ctx, &constructor_val.toObject());

  JS::RootedObject obj(ctx, JS_New(ctx, constructor, JS::HandleValueArray::empty()));
  if (!obj) {
    JS_ReportErrorASCII(ctx, "Calling D2BSScript constructor failed");
    return nullptr;
  }
  ScriptWrap* wrap = new ScriptWrap(ctx, obj, script);
  if (!wrap) {
    JS_ReportOutOfMemory(ctx);
    return nullptr;
  }
  return obj;
}

void ScriptWrap::Initialize(JSContext* ctx, JS::HandleObject target) {
  static JSPropertySpec props[] = {
      JS_PSG("name", trampoline<GetName>, JSPROP_ENUMERATE),          //
      JS_PSG("type", trampoline<GetType>, JSPROP_ENUMERATE),          //
      JS_PSG("running", trampoline<GetRunning>, JSPROP_ENUMERATE),    //
      JS_PSG("threadid", trampoline<GetThreadId>, JSPROP_ENUMERATE),  //
      JS_PSG("memory", trampoline<GetMemory>, JSPROP_ENUMERATE),      //
      JS_PS_END,
  };
  static JSFunctionSpec methods[] = {
      JS_FN("getNext", trampoline<GetNext>, 0, JSPROP_ENUMERATE),  //
      JS_FN("pause", trampoline<Pause>, 0, JSPROP_ENUMERATE),      //
      JS_FN("resume", trampoline<Resume>, 0, JSPROP_ENUMERATE),    //
      JS_FN("stop", trampoline<Stop>, 0, JSPROP_ENUMERATE),        //
      JS_FN("join", trampoline<Join>, 0, JSPROP_ENUMERATE),        //
      JS_FN("send", trampoline<Send>, 1, JSPROP_ENUMERATE),        //
      JS_FS_END,
  };

  JS::RootedObject proto(ctx, JS_InitClass(ctx, target, nullptr, &m_class, trampoline<New>, 0, props, methods, nullptr, nullptr));
  if (!proto) {
    Log("failed to initialize class Script");
    return;
  }

  // globals
  JS_DefineFunction(ctx, target, "getScript", trampoline<GetScript>, 0, JSPROP_ENUMERATE);
  JS_DefineFunction(ctx, target, "getScripts", trampoline<GetScripts>, 0, JSPROP_ENUMERATE);
}

ScriptWrap::ScriptWrap(JSContext* ctx, JS::HandleObject obj, Script* script) : BaseObject(ctx, obj), m_script(script) {
}

void ScriptWrap::finalize(JSFreeOp* /*fop*/, JSObject* obj) {
  BaseObject* wrap = BaseObject::FromJSObject(obj);
  if (wrap) {
    delete wrap;
  }
}

bool ScriptWrap::New(JSContext* ctx, JS::CallArgs& args) {
  JS::RootedObject newObject(ctx, JS_NewObjectForConstructor(ctx, &m_class, args));
  if (!newObject) {
    THROW_ERROR(ctx, "failed to instantiate script");
  }
  args.rval().setObject(*newObject);
  return true;
}

// properties
bool ScriptWrap::GetName(JSContext* ctx, JS::CallArgs& args) {
  ScriptWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Script* script = wrap->m_script;

  args.rval().setString(JS_NewStringCopyZ(ctx, script->GetShortFilename()));
  return true;
}

bool ScriptWrap::GetType(JSContext* ctx, JS::CallArgs& args) {
  ScriptWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Script* script = wrap->m_script;

  args.rval().setBoolean(script->GetMode() == kScriptModeGame ? false : true);
  return true;
}

bool ScriptWrap::GetRunning(JSContext* ctx, JS::CallArgs& args) {
  ScriptWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Script* script = wrap->m_script;
  args.rval().setBoolean(script->IsRunning());
  return true;
}

bool ScriptWrap::GetThreadId(JSContext* ctx, JS::CallArgs& args) {
  ScriptWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Script* script = wrap->m_script;
  args.rval().setInt32(script->GetThreadId());
  return true;
}

bool ScriptWrap::GetMemory(JSContext* ctx, JS::CallArgs& args) {
  args.rval().setInt32(JS_GetGCParameter(ctx, JSGCParamKey::JSGC_BYTES));
  return true;
}

// functions
bool ScriptWrap::GetNext(JSContext* ctx, JS::CallArgs& args) {
  ScriptWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Script* script = wrap->m_script;

  sScriptEngine->LockScriptList("scrip.getNext");
  for (ScriptMap::iterator it = sScriptEngine->scripts().begin(); it != sScriptEngine->scripts().end(); it++) {
    if (it->second == script) {
      it++;
      if (it == sScriptEngine->scripts().end())
        break;

      JS::RootedObject r(ctx, ScriptWrap::Instantiate(ctx, it->second));
      sScriptEngine->UnLockScriptList("scrip.getNext");
      args.rval().setObject(*r);
      return true;
    }
  }

  sScriptEngine->UnLockScriptList("scrip.getNext");
  args.rval().setUndefined();
  return true;
}

bool ScriptWrap::Pause(JSContext* ctx, JS::CallArgs& args) {
  ScriptWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Script* script = wrap->m_script;

  if (script->IsRunning())
    script->Pause();
  args.rval().setNull();
  return true;
}

bool ScriptWrap::Resume(JSContext* ctx, JS::CallArgs& args) {
  ScriptWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Script* script = wrap->m_script;

  script->Resume();
  args.rval().setNull();
  return true;
}

bool ScriptWrap::Stop(JSContext* ctx, JS::CallArgs& args) {
  ScriptWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Script* script = wrap->m_script;

  if (script->IsRunning())
    script->Stop();
  args.rval().setNull();
  return true;
}

bool ScriptWrap::Join(JSContext* ctx, JS::CallArgs& args) {
  ScriptWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Script* script = wrap->m_script;

  script->Join();
  args.rval().setNull();
  return true;
}

bool ScriptWrap::Send(JSContext* ctx, JS::CallArgs& args) {
  if (!args.requireAtLeast(ctx, "send", 1)) {
    return false;
  }

  ScriptWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Script* script = wrap->m_script;

  if (!script || !script->IsRunning()) {
    args.rval().setNull();
    return true;
  }

  // documentation says to not use args.array but we do anyway because we dont give a fuck
  if (!ScriptMessageEvent(ctx, script, args.length(), args.array())) {
    THROW_ERROR(ctx, "failed to send message");
  }

  args.rval().setNull();
  return true;
}

bool ScriptWrap::GetScript(JSContext* ctx, JS::CallArgs& args) {
  Script* iterp = NULL;
  if (args.length() == 1 && args[0].isBoolean() && args[0].toBoolean() == TRUE)
    iterp = ((ThreadState*)JS_GetContextPrivate(ctx))->script;
  else if (args.length() == 1 && args[0].isNumber()) {
    // loop over the Scripts in ScriptEngine and find the one with the right threadid
    uint32_t tid;
    if (!JS::ToUint32(ctx, args[0], &tid)) {
      THROW_ERROR(ctx, "failed to convert argument");
    }
    FindHelper helper = {tid, NULL, NULL};
    sScriptEngine->ForEachScript(FindScriptByTid, &helper);
    if (helper.script != NULL)
      iterp = helper.script;
    else {
      args.rval().setNull();
      return true;
    }
  } else if (args.length() == 1 && args[0].isString()) {
    StringWrap name(ctx, args[0]);
    if (!name) {
      THROW_ERROR(ctx, "failed to encode string");
    }
    char* fname = _strdup(name);
    StringReplace(fname, '/', '\\', strlen(fname));
    FindHelper helper = {0, fname, NULL};
    sScriptEngine->ForEachScript(FindScriptByName, &helper);
    free(fname);
    if (helper.script != NULL)
      iterp = helper.script;
    else {
      args.rval().setNull();
      return true;
    }
  } else {
    if (sScriptEngine->scripts().size() > 0) {
      //	EnterCriticalSection(&sScriptEngine->lock);
      sScriptEngine->LockScriptList("getScript");
      iterp = sScriptEngine->scripts().begin()->second;
      sScriptEngine->UnLockScriptList("getScript");
      //	LeaveCriticalSection(&sScriptEngine->lock);
    }

    if (iterp == NULL) {
      args.rval().setNull();
      return true;
    }
  }

  JS::RootedObject obj(ctx, ScriptWrap::Instantiate(ctx, iterp));
  if (!obj) {
    THROW_ERROR(ctx, "failed to instantiate script");
  }
  args.rval().setObjectOrNull(obj);
  return true;
}

bool ScriptWrap::GetScripts(JSContext* ctx, JS::CallArgs& args) {
  DWORD dwArrayCount = NULL;

  JS::RootedObject pReturnArray(ctx, JS_NewArrayObject(ctx, 0));
  sScriptEngine->LockScriptList("getScripts");

  for (ScriptMap::iterator it = sScriptEngine->scripts().begin(); it != sScriptEngine->scripts().end(); it++) {
    JS::RootedObject res(ctx, ScriptWrap::Instantiate(ctx, it->second));
    if (!res) {
      THROW_ERROR(ctx, "failed to instantiate script");
    }
    JS_SetElement(ctx, pReturnArray, dwArrayCount, res);
    dwArrayCount++;
  }

  sScriptEngine->UnLockScriptList("getScripts");
  args.rval().setObject(*pReturnArray);
  return true;
}

D2BS_BINDING_INTERNAL(ScriptWrap, ScriptWrap::Initialize)
