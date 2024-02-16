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

  static bool New(JSContext* ctx, unsigned argc, JS::Value* vp);

  // properties
  static bool GetName(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetType(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetRunning(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetThreadId(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetMemory(JSContext* ctx, unsigned argc, JS::Value* vp);

  // functions
  static bool GetNext(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool Pause(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool Resume(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool Stop(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool Join(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool Send(JSContext* ctx, unsigned argc, JS::Value* vp);

  // global functions
  static bool GetScript(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetScripts(JSContext* ctx, unsigned argc, JS::Value* vp);

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
