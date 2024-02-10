#pragma once

#include "js32.h"
#include "Script.h"

class ScriptWrap {
 public:
  static JSValue Instantiate(JSContext* ctx, JSValue new_target, Script* script);
  static void Initialize(JSContext* ctx, JSValue target);

 private:
  ScriptWrap(JSContext* ctx, Script* script);

  // properties
  static JSValue GetName(JSContext* ctx, JSValue this_val);
  static JSValue GetType(JSContext* ctx, JSValue this_val);
  static JSValue GetRunning(JSContext* ctx, JSValue this_val);
  static JSValue GetThreadId(JSContext* ctx, JSValue this_val);
  static JSValue GetMemory(JSContext* ctx, JSValue this_val);

  // functions
  static JSValue GetNext(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue Pause(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue Resume(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue Stop(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue Join(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue Send(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  // global functions
  static JSValue GetScript(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue GetScripts(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  static inline JSClassID m_class_id = 0;
  static inline JSCFunctionListEntry m_proto_funcs[] = {
      JS_CGETSET_DEF("name", GetName, nullptr),          //
      JS_CGETSET_DEF("type", GetType, nullptr),          //
      JS_CGETSET_DEF("running", GetRunning, nullptr),    //
      JS_CGETSET_DEF("threadid", GetThreadId, nullptr),  //
      JS_CGETSET_DEF("memory", GetMemory, nullptr),      //

      JS_FS("getNext", GetNext, 0, FUNCTION_FLAGS),  //
      JS_FS("pause", Pause, 0, FUNCTION_FLAGS),      //
      JS_FS("resume", Resume, 0, FUNCTION_FLAGS),    //
      JS_FS("stop", Stop, 0, FUNCTION_FLAGS),        //
      JS_FS("join", Join, 0, FUNCTION_FLAGS),        //
      JS_FS("send", Send, 1, FUNCTION_FLAGS),
  };

  Script* m_script;
};