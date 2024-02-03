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

typedef unsigned int uint;
typedef uint32_t uint32;
typedef int32_t int32;
typedef unsigned int jsrefcount;
typedef double jsdouble;
typedef int32_t jsint;
typedef uint32_t jsuint;
typedef uint16_t uint16;

// #define NUM(x) #x
// #define NAME(line, v) (__FILE__ ":" NUM(line) " -> " #v)
//
void* JS_GetPrivate(JSContext* ctx, JSValue obj);
void JS_SetPrivate(JSValue obj, void* data);

void* JS_GetPrivate(JSValue obj);
void JS_SetPrivate(JSContext* ctx, JSValue obj, void* data);

void* JS_GetContextPrivate(JSRuntime* rt);
void* JS_GetContextPrivate(JSContext* ctx);

void* JS_GetInstancePrivate(JSContext* ctx, JSValue val, JSClassID class_id, JSValue* argv);

JSValue JS_NewString(JSContext* ctx, const wchar_t* str);

std::optional<std::wstring> JS_ToWString(JSContext* ctx, JSValue val);

// JSBool JSVAL_IS_OBJECT(JSValue v);

struct JSClass2 {
  JSClassID* classpid;
  JSClassDef* classp;
};

struct JSClassSpec {
  JSClassID* pclass_id;
  JSClassDef* classp;
  JSClass* proto;
  JSValue (*ctor)(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst* argv);
  uint argc;
  JSCFunctionListEntry* methods;
  int num_methods;
  JSCFunctionListEntry* properties;
  int num_properties;
  JSCFunctionListEntry* static_methods;
  int num_s_methods;
  JSCFunctionListEntry* static_properties;
  int num_s_properties;
};

// #define JSVAL_IS_FUNCTION(cx, var) (!JSVAL_IS_PRIMITIVE(var) && JS_ObjectIsFunction(cx, JSVAL_TO_OBJECT(var)))
//
// #define JSPROP_PERMANENT_VAR (JSPROP_READONLY | JSPROP_ENUMERATE | JSPROP_PERMANENT)
// #define JSPROP_STATIC_VAR (JSPROP_ENUMERATE | JSPROP_PERMANENT)
//
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

//
// #define JSPROP_DEFAULT JSPROP_ENUMERATE | JSPROP_PERMANENT
// #define JSPROP_STATIC JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY
//
#define JSAPI_FUNC(name) \
  JSValue name##([[maybe_unused]] JSContext * ctx, [[maybe_unused]] JSValueConst this_val, [[maybe_unused]] int argc, [[maybe_unused]] JSValueConst* argv)

#define JSAPI_PROP(name) JSValue name##([[maybe_unused]] JSContext * ctx, [[maybe_unused]] JSValueConst this_val, [[maybe_unused]] int magic)
#define JSAPI_STRICT_PROP(name) \
  JSValue name##([[maybe_unused]] JSContext * ctx, [[maybe_unused]] JSValueConst this_val, [[maybe_unused]] JSValueConst val, [[maybe_unused]] int magic)
//
#define JS_FN(name, func, length, flags) JS_CFUNC_DEF(name, length, func)
#define JS_FS(name, func, length, flags) JS_CFUNC_DEF(name, length, func)
//
// #define JS_PS(name, id, flags, getter, setter) \
//  { name, id, flags, getter, setter }
//
// #define JS_PS_END JS_PS(0, 0, 0, 0, 0)
//
// #define JS_CS(classp, proto, ctor, argc, methods, props, static_methods, static_props) \
//  { classp, proto, ctor, argc, methods, props, static_methods, static_props }
//
// #define JS_CS_END \
//  { 0 }
//
// #define JS_MS(name, classes, static_methods, static_properties) \
//  { L##name, classes, static_methods, static_properties }
//
// #define JS_MS_END \
//  { 0, 0, 0, 0 }
//
// #define JSCLASS_SPEC(add, del, get, set, enumerate, resolve, convert, finalize, ctor) \
//  add, del, get, set, enumerate, resolve, convert, finalize, NULL, NULL, NULL, ctor, NULL
//
// #define JSCLASS_DEFAULT_WITH_CTOR(ctor) \
//  JSCLASS_SPEC(JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub, JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub, ctor)
//
// #define JSCLASS_DEFAULT_STANDARD_MEMBERS \
//  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub, JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
//  JSCLASS_NO_OPTIONAL_MEMBERS
