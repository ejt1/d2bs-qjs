#pragma once

#include "js32.h"

#include <uv.h>

class TimerWrap {
 public:
  static JSValue Instantiate(JSContext* ctx, int64_t delay, JSValue func, int argc, JSValue* argv, bool interval = false);
  static void Initialize(JSContext* ctx, JSValue target);

  void Unref();

 private:
  TimerWrap(JSContext* ctx);
  virtual ~TimerWrap();

  static void MarkGC(JSRuntime* rt, JSValue val, JS_MarkFunc* mark_func);

  // globals
  static JSValue setImmediate(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue clearImmediate(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  static JSValue setTimeout(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue clearTimeout(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  static JSValue setInterval(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue clearInterval(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  static inline JSClassID m_class_id = 0;
  static inline JSCFunctionListEntry m_funcs[] = {
      JS_CFUNC_DEF("setImmediate", 1, setImmediate),
      JS_CFUNC_DEF("clearImmediate", 1, clearImmediate),
      JS_CFUNC_DEF("setTimeout", 2, setTimeout),
      JS_CFUNC_DEF("clearTimeout", 1, clearTimeout),
      JS_CFUNC_DEF("setInterval", 2, setInterval),
      JS_CFUNC_DEF("clearInterval", 1, clearInterval),
  };

  // implementation detail
  static void TimerCallback(uv_timer_t* handle);
  void Clear();

  JSContext* context;
  JSValue ref;
  uv_timer_t timer_handle;
  JSValue callback;
  std::vector<JSValue> args;
  bool inside_callback;
  bool clear_after_callback;
};