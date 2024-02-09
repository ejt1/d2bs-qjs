#include "JSTimer.h"

#include "Bindings.h"
#include "Script.h"  // ThreadState

#include "D2Helpers.h"

JSValue TimerWrap::Instantiate(JSContext* ctx, int64_t delay, JSValue func, int argc, JSValue* argv, bool interval) {
  JSRuntime* rt = JS_GetRuntime(ctx);
  ThreadState* ts = static_cast<ThreadState*>(JS_GetRuntimeOpaque(rt));
  JSValue obj = JS_NewObjectClass(ctx, m_class_id);
  if (JS_IsException(obj)) {
    return JS_EXCEPTION;
  }

  TimerWrap* wrap = new TimerWrap(ctx);
  if (!wrap) {
    JS_FreeValue(ctx, obj);
    return JS_ThrowOutOfMemory(ctx);
  }
  wrap->callback = JS_DupValue(ctx, func);
  for (int i = 0; i < argc; ++i) {
    wrap->args.push_back(JS_DupValue(ctx, argv[i]));
  }

  uv_timer_init(ts->loop, &wrap->timer_handle);
  // associate timer with TimerWrap so we can
  // access it inside the callback later
  wrap->timer_handle.data = wrap;
  // start timer
  uint64_t repeat = interval ? delay : 0;
  uv_timer_start(&wrap->timer_handle, TimerCallback, delay, repeat);
  wrap->ref = JS_DupValue(ctx, obj);
  JS_SetOpaque(obj, wrap);
  return obj;
}

void TimerWrap::Initialize(JSContext* ctx, JSValue target) {
  JSClassDef def{};
  def.class_name = "Timeout";
  def.finalizer = [](JSRuntime* /*rt*/, JSValue val) {
    TimerWrap* wrap = static_cast<TimerWrap*>(JS_GetOpaque(val, m_class_id));
    if (wrap) {
      delete wrap;
    }
  };
  def.gc_mark = MarkGC;

  if (m_class_id == 0) {
    JS_NewClassID(&m_class_id);
  }
  JS_NewClass(JS_GetRuntime(ctx), m_class_id, &def);
  JSValue proto = JS_NewObject(ctx);
  JSValue obj = JS_NewObjectProtoClass(ctx, proto, m_class_id);
  JS_SetClassProto(ctx, m_class_id, proto);
  JS_SetPropertyStr(ctx, target, "Timeout", obj);

  // globals
  JS_SetPropertyFunctionList(ctx, target, m_funcs, _countof(m_funcs));
}

void TimerWrap::Unref() {
  if (context && !JS_IsUndefined(ref)) {
    JS_FreeValue(context, ref);
  }
}

TimerWrap::TimerWrap(JSContext* ctx) : context(ctx), ref(JS_UNDEFINED), timer_handle(), callback(JS_UNDEFINED), inside_callback(false), clear_after_callback(false) {
}

TimerWrap::~TimerWrap() {
  uv_close((uv_handle_t*)(&timer_handle), nullptr);
  if (context && !JS_IsUndefined(callback)) {
    for (const auto& arg : args) {
      JS_FreeValue(context, arg);
    }
    args.clear();
    JS_FreeValue(context, callback);
  }
}

void TimerWrap::MarkGC(JSRuntime* rt, JSValue val, JS_MarkFunc* mark_func) {
  TimerWrap* wrap = static_cast<TimerWrap*>(JS_GetOpaque(val, m_class_id));
  if (wrap && !JS_IsUndefined(wrap->callback)) {
    JS_MarkValue(rt, wrap->callback, mark_func);
  }
}

JSValue TimerWrap::setImmediate(JSContext* ctx, JSValue /*this_val*/, int argc, JSValue* argv) {
  if (argc < 1) {
    return JS_ThrowSyntaxError(ctx, "not enough arguments");
  }

  JSValue func = argv[0];
  if (!JS_IsFunction(ctx, func)) {
    return JS_ThrowTypeError(ctx, "handler must be a function");
  }

  return Instantiate(ctx, 1, func, argc - 1, &argv[1]);
}

JSValue TimerWrap::clearImmediate(JSContext* ctx, JSValue /*this_val*/, int argc, JSValue* argv) {
  if (argc < 1) {
    return JS_ThrowSyntaxError(ctx, "not enough arguments");
  }

  if (!JS_IsObject(argv[0])) {
    return JS_ThrowTypeError(ctx, "not an object");
  }

  TimerWrap* wrap = static_cast<TimerWrap*>(JS_GetOpaque2(ctx, argv[0], m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  wrap->Clear();
  return JS_UNDEFINED;
}

JSValue TimerWrap::setTimeout(JSContext* ctx, JSValue /*this_val*/, int argc, JSValue* argv) {
  if (argc < 2) {
    return JS_ThrowSyntaxError(ctx, "not enough arguments");
  }

  JSValue func = argv[0];
  if (!JS_IsFunction(ctx, func)) {
    return JS_ThrowTypeError(ctx, "handler must be a function");
  }

  int64_t delay;
  if (JS_ToInt64(ctx, &delay, argv[1])) {
    return JS_EXCEPTION;
  }

  return Instantiate(ctx, delay, func, argc - 2, &argv[2]);
}

JSValue TimerWrap::clearTimeout(JSContext* ctx, JSValue /*this_val*/, int argc, JSValue* argv) {
  if (argc < 1) {
    return JS_ThrowSyntaxError(ctx, "not enough arguments");
  }

  if (!JS_IsObject(argv[0])) {
    return JS_ThrowTypeError(ctx, "not an object");
  }

  TimerWrap* wrap = static_cast<TimerWrap*>(JS_GetOpaque2(ctx, argv[0], m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  wrap->Clear();
  return JS_UNDEFINED;
}

JSValue TimerWrap::setInterval(JSContext* ctx, JSValue /*this_val*/, int argc, JSValue* argv) {
  if (argc < 2) {
    return JS_ThrowSyntaxError(ctx, "not enough arguments");
  }

  JSValue func = argv[0];
  if (!JS_IsFunction(ctx, func)) {
    return JS_ThrowTypeError(ctx, "handler must be a function");
  }

  int64_t delay;
  if (JS_ToInt64(ctx, &delay, argv[1])) {
    return JS_EXCEPTION;
  }

  return Instantiate(ctx, delay, func, argc - 2, &argv[2], true);
}

JSValue TimerWrap::clearInterval(JSContext* ctx, JSValue /*this_val*/, int argc, JSValue* argv) {
  if (argc < 1) {
    return JS_ThrowSyntaxError(ctx, "not enough arguments");
  }

  if (!JS_IsObject(argv[0])) {
    return JS_ThrowTypeError(ctx, "not an object");
  }

  TimerWrap* wrap = static_cast<TimerWrap*>(JS_GetOpaque2(ctx, argv[0], m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  wrap->Clear();
  return JS_UNDEFINED;
}

void TimerWrap::TimerCallback(uv_timer_t* handle) {
  TimerWrap* wrap = static_cast<TimerWrap*>(handle->data);
  // BUG(ejt): Timers CANNOT be canceled inside its own callback!
  wrap->inside_callback = true;
  JSValue rval = JS_Call(wrap->context, wrap->callback, JS_UNDEFINED, wrap->args.size(), wrap->args.data());
  wrap->inside_callback = false;
  if (JS_IsException(rval)) {
    JS_ReportPendingException(wrap->context);
    wrap->Clear();
  }
  JS_FreeValue(wrap->context, rval);
  if (wrap->clear_after_callback) {
    wrap->Clear();
    wrap->clear_after_callback = false;
  }
}

void TimerWrap::Clear() {
  // TODO(ejt): ugly little hack to make it so you can cancel a timer inside its own callback
  if (inside_callback) {
    clear_after_callback = true;
    return;
  }
  uv_timer_stop(&timer_handle);
}

D2BS_BINDING_INTERNAL(TimerWrap, TimerWrap::Initialize)