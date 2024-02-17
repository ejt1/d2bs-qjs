#pragma once

#include "JSBaseObject.h"
#include "Script.h"

class ScriptWrap : public BaseObject {
 public:
  static JSObject* Instantiate(JSContext* ctx, Script* script);
  static void Initialize(JSContext* ctx, JS::HandleObject target);

 private:
  ScriptWrap(JSContext* ctx, JS::HandleObject obj, Script* script);

  static void finalize(JSFreeOp* fop, JSObject* obj);

  static bool New(JSContext* ctx, JS::CallArgs& args);

  // properties
  static bool GetName(JSContext* ctx, JS::CallArgs& args);
  static bool GetType(JSContext* ctx, JS::CallArgs& args);
  static bool GetRunning(JSContext* ctx, JS::CallArgs& args);
  static bool GetThreadId(JSContext* ctx, JS::CallArgs& args);
  static bool GetMemory(JSContext* ctx, JS::CallArgs& args);

  // functions
  static bool GetNext(JSContext* ctx, JS::CallArgs& args);
  static bool Pause(JSContext* ctx, JS::CallArgs& args);
  static bool Resume(JSContext* ctx, JS::CallArgs& args);
  static bool Stop(JSContext* ctx, JS::CallArgs& args);
  static bool Join(JSContext* ctx, JS::CallArgs& args);
  static bool Send(JSContext* ctx, JS::CallArgs& args);

  // global functions
  static bool GetScript(JSContext* ctx, JS::CallArgs& args);
  static bool GetScripts(JSContext* ctx, JS::CallArgs& args);

  static inline JSClassOps m_ops = {
      .addProperty = nullptr,
      .delProperty = nullptr,
      .enumerate = nullptr,
      .newEnumerate = nullptr,
      .resolve = nullptr,
      .mayResolve = nullptr,
      .finalize = finalize,
      .call = nullptr,
      .hasInstance = nullptr,
      .construct = nullptr,
      .trace = nullptr,
  };
  static inline JSClass m_class = {
      "D2BSScript",
      JSCLASS_HAS_RESERVED_SLOTS(kInternalFieldCount) | JSCLASS_FOREGROUND_FINALIZE,
      &m_ops,
  };

  Script* m_script;
};
