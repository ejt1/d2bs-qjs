#include "JSTimer.h"

#include "Bindings.h"
#include "Script.h"  // ThreadState

#include "D2Helpers.h"

JSObject* TimerWrap::Instantiate(JSContext* ctx, int64_t delay, JS::HandleValue func, unsigned argc, JS::Value* argv, bool interval) {
  JS::RootedObject obj(ctx, JS_NewObject(ctx, &m_class));
  if (!obj) {
    JS_ReportErrorASCII(ctx, "Calling D2BSScript constructor failed");
    return nullptr;
  }
  TimerWrap* wrap = new TimerWrap(ctx, obj);
  if (!wrap) {
    JS_ReportOutOfMemory(ctx);
    return nullptr;
  }
  wrap->ref.init(ctx, obj);
  wrap->callback.init(ctx, func.toObjectOrNull());
  for (uint32_t i = 0; i < argc; ++i) {
    wrap->args.append(argv[i]);
  }

  ThreadState* ts = static_cast<ThreadState*>(JS_GetContextPrivate(ctx));
  uv_timer_init(ts->loop, &wrap->timer_handle);
  // associate timer with TimerWrap so we can
  // access it inside the callback later
  wrap->timer_handle.data = wrap;
  // start timer
  uint64_t repeat = interval ? delay : 0;
  uv_timer_start(&wrap->timer_handle, TimerCallback, delay, repeat);
  return obj;
}

void TimerWrap::Initialize(JSContext* ctx, JS::HandleObject target) {
  JS::RootedObject proto(ctx, JS_InitClass(ctx, target, nullptr, &m_class, nullptr, 0, nullptr, nullptr, nullptr, nullptr));
  if (!proto) {
    Log("failed to initialize class Timeout");
    return;
  }

  JS_DefineFunctions(ctx, target, m_methods);
}

void TimerWrap::Unref() {
  ref.reset();
}

TimerWrap::TimerWrap(JSContext* ctx, JS::HandleObject obj)
    : BaseObject(ctx, obj), context(ctx), ref(ctx), timer_handle(), callback(ctx), args(ctx), inside_callback(false), clear_after_callback(false) {
}

TimerWrap::~TimerWrap() {
  uv_close((uv_handle_t*)(&timer_handle), nullptr);
}

void TimerWrap::finalize(JSFreeOp* fop, JSObject* obj) {
  BaseObject* wrap = BaseObject::FromJSObject(obj);
  if (wrap) {
    delete wrap;
  }
}

// void TimerWrap::MarkGC(JSRuntime* rt, JSValue val, JS_MarkFunc* mark_func) {
//   TimerWrap* wrap = static_cast<TimerWrap*>(JS_GetOpaque(val, m_class_id));
//   if (wrap && !JS_IsUndefined(wrap->callback)) {
//     JS_MarkValue(rt, wrap->callback, mark_func);
//   }
// }

bool TimerWrap::setImmediate(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "setImmediate", 1)) {
    return false;
  }

  JS::RootedValue func(ctx, args[0]);
  if (!func.isObject() || !JS::IsCallable(&func.toObject())) {
    JS_ReportErrorASCII(ctx, "handler must be a function");
    return false;
  }

  args.rval().setObjectOrNull(Instantiate(ctx, 1, func, argc - 1, &vp[1]));
  return true;
}

bool TimerWrap::clearImmediate(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "clearImmediate", 1)) {
    return false;
  }
  TimerWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args[0]);
  wrap->Clear();
  args.rval().setUndefined();
  return true;
}

bool TimerWrap::setTimeout(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "setTimeout", 2)) {
    return false;
  }

  JS::RootedValue func(ctx, args[0]);
  if (!func.isObject() || !JS::IsCallable(&func.toObject())) {
    JS_ReportErrorASCII(ctx, "handler must be a function");
    return false;
  }

  int64_t delay = static_cast<int64_t>(args[1].toNumber());

  args.rval().setObjectOrNull(Instantiate(ctx, delay, func, argc - 2, &vp[2]));
  return true;
}

bool TimerWrap::clearTimeout(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "clearTimeout", 1)) {
    return false;
  }

  TimerWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args[0]);
  wrap->Clear();
  args.rval().setUndefined();
  return true;
}

bool TimerWrap::setInterval(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "setInterval", 2)) {
    return false;
  }

  JS::RootedValue func(ctx, args[0]);
  if (!func.isObject() || !JS::IsCallable(&func.toObject())) {
    JS_ReportErrorASCII(ctx, "handler must be a function");
    return false;
  }

  int64_t delay = static_cast<int64_t>(args[1].toNumber());

  args.rval().setObjectOrNull(Instantiate(ctx, delay, func, argc - 2, &vp[2], true));
  return true;
}

bool TimerWrap::clearInterval(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "clearInterval", 1)) {
    return false;
  }

  TimerWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args[0]);
  wrap->Clear();
  args.rval().setUndefined();
  return true;
}

void TimerWrap::TimerCallback(uv_timer_t* handle) {
  TimerWrap* wrap = static_cast<TimerWrap*>(handle->data);
  wrap->inside_callback = true;
  JS::RootedValue rval(wrap->context);
  JS::RootedObject global(wrap->context, JS_GetGlobalForObject(wrap->context, wrap->ref));
  JS::RootedFunction fun(wrap->context, JS_GetObjectFunction(wrap->callback.get()));
  if (!JS_CallFunction(wrap->context, global, fun, wrap->args, &rval)) {
    JS_ReportPendingException(wrap->context);
    wrap->Clear();
  }
  wrap->inside_callback = false;
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
