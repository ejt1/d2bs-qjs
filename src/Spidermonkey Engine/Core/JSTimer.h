#pragma once

#include "JSBaseObject.h"

#include <uv.h>

class TimerWrap : public BaseObject {
 public:
  static JSObject* Instantiate(JSContext* ctx, int64_t delay, JS::HandleValue func, unsigned argc, JS::Value* argv, bool interval = false);
  static void Initialize(JSContext* ctx, JS::HandleObject target);

  void Unref();

 private:
  TimerWrap(JSContext* ctx, JS::HandleObject obj);
  virtual ~TimerWrap();

  static void finalize(JSFreeOp* fop, JSObject* obj);
  // static void MarkGC(JSRuntime* rt, JSValue val, JS_MarkFunc* mark_func);

  // globals
  static bool setImmediate(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool clearImmediate(JSContext* ctx, unsigned argc, JS::Value* vp);

  static bool setTimeout(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool clearTimeout(JSContext* ctx, unsigned argc, JS::Value* vp);

  static bool setInterval(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool clearInterval(JSContext* ctx, unsigned argc, JS::Value* vp);

  // implementation detail
  static void TimerCallback(uv_timer_t* handle);
  void Clear();

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
      "Timeout",
      JSCLASS_HAS_RESERVED_SLOTS(kInternalFieldCount) | JSCLASS_FOREGROUND_FINALIZE,
      &m_ops,
  };
  static inline JSFunctionSpec m_methods[] = {
      JS_FN("setImmediate", setImmediate, 1, JSPROP_ENUMERATE),      //
      JS_FN("clearImmediate", clearImmediate, 1, JSPROP_ENUMERATE),  //
      JS_FN("setTimeout", setTimeout, 2, JSPROP_ENUMERATE),          //
      JS_FN("clearTimeout", clearTimeout, 1, JSPROP_ENUMERATE),      //
      JS_FN("setInterval", setInterval, 2, JSPROP_ENUMERATE),        //
      JS_FN("clearInterval", clearInterval, 1, JSPROP_ENUMERATE),    //
      JS_FS_END,                                                     //
  };

  JSContext* context;
  uv_timer_t timer_handle;
  JS::PersistentRootedObject ref;
  JS::PersistentRootedObject callback;
  JS::AutoValueVector args;
  bool inside_callback;
  bool clear_after_callback;
};
