#pragma comment(lib, "psapi.lib")  // Added to support GetProcessMemoryInfo()
#pragma once

// TODO(ejt): most of the content in here is only for migration purposes
// to minimize the work to get from mozjs to quickjs.

#pragma warning(push, 0)
#include <quickjs.h>
#pragma warning(pop)

#include <vector>
#include <io.h>
#include <string>
#include <fstream>
#include <optional>
#include <streambuf>

JSValue JS_NewString(JSContext* ctx, const wchar_t* str);

struct JSClassSpec {
  JSClassID* pclass_id;
  JSClassDef* classp;
  JSClass* proto;
  JSValue (*ctor)(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst* argv);
  uint32_t argc;
  JSCFunctionListEntry* methods;
  int num_methods;
  JSCFunctionListEntry* properties;
  int num_properties;
  JSCFunctionListEntry* static_methods;
  int num_s_methods;
  JSCFunctionListEntry* static_properties;
  int num_s_properties;
};

#define CLASS_CTOR(name) \
  JSValue name##_ctor([[maybe_unused]] JSContext* ctx, [[maybe_unused]] JSValueConst new_target, [[maybe_unused]] int argc, [[maybe_unused]] JSValueConst* argv)

#define CLASS_FINALIZER(name) void name##_finalizer([[maybe_unused]] JSRuntime* rt, [[maybe_unused]] JSValue val)

#define EMPTY_CTOR(name)                                     \
  CLASS_CTOR(name) {                                         \
    return JS_ThrowReferenceError(ctx, "Invalid Operation"); \
  }

JSValue BuildObject(JSContext* ctx, JSClassID class_id, JSCFunctionListEntry* funcs = NULL, size_t num_funcs = 0, JSCFunctionListEntry* props = NULL,
                    size_t num_props = 0,
                    void* priv = NULL, JSValue new_target = JS_UNDEFINED);

JSValue JS_CompileFile(JSContext* ctx, JSValue globalObject, std::string fileName);

template <typename... Args>
JSValue JS_ReportError(JSContext* ctx, const char* fmt, Args&&... args) {
  return JS_ThrowSyntaxError(ctx, fmt, std::forward<Args>(args)...);
}

void JS_ReportPendingException(JSContext* ctx);

#define THROW_ERROR(cx, msg) \
  {                          \
    JS_ReportError(cx, msg); \
    return JS_EXCEPTION;     \
  }

#define THROW_WARNING(cx, msg) \
  {                            \
    JS_ReportError(cx, msg);   \
    return JS_EXCEPTION;       \
  }

#define JSAPI_FUNC(name) \
  JSValue name##([[maybe_unused]] JSContext * ctx, [[maybe_unused]] JSValueConst this_val, [[maybe_unused]] int argc, [[maybe_unused]] JSValueConst* argv)

#define JSAPI_PROP(name) JSValue name##([[maybe_unused]] JSContext * ctx, [[maybe_unused]] JSValueConst this_val, [[maybe_unused]] int magic)
#define JSAPI_STRICT_PROP(name) \
  JSValue name##([[maybe_unused]] JSContext * ctx, [[maybe_unused]] JSValueConst this_val, [[maybe_unused]] JSValueConst val, [[maybe_unused]] int magic)
//
#define JS_FN(name, func, length, flags) JS_CFUNC_DEF(name, length, func)
#define JS_FS(name, func, length, flags) JS_CFUNC_DEF(name, length, func)