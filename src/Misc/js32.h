#pragma comment(lib, "psapi.lib")  // Added to support GetProcessMemoryInfo()
#pragma once

#pragma warning(push, 0)
#include "jsapi.h"
#include "jsfriendapi.h"
#pragma warning(pop)

#include <vector>
#include <io.h>
#include <string>
#include <fstream>
#include <streambuf>

typedef unsigned int uint;
typedef uint32_t uint32;
typedef int32_t int32;
typedef unsigned int jsrefcount;
typedef double jsdouble;
typedef int32_t jsint;
typedef uint32_t jsuint;
typedef uint16_t uint16;

// #if defined(EXPORTING)
// #define EXPORT __declspec(dllexport)
// #else
// #define EXPORT
// #endif

#define NUM(x) #x
#define NAME(line, v) (__FILE__ ":" NUM(line) " -> " #v)
#define JS_AddValueRoot(cx, vp) JS_AddNamedValueRoot((cx), (vp), NAME(__LINE__, vp))
#define JS_AddStringRoot(cx, vp) JS_AddNamedStringRoot((cx), (vp), NAME(__LINE__, vp))
#define JS_AddObjectRoot(cx, vp) JS_AddNamedObjectRoot((cx), (vp), NAME(__LINE__, vp))

void* JS_GetPrivate(JSContext* cx, JSObject* obj);
void JS_SetPrivate(JSContext* cx, JSObject* obj, void* data);
JSBool JSVAL_IS_OBJECT(jsval v);
// IMPORTANT: Ordering is critical here! If your object has a
// defined prototype, _THAT PROTOTYPE MUST BE LISTED ABOVE IT_
struct JSClassSpec {
  JSClass* classp;
  JSClass* proto;
  JSNative ctor;
  uint argc;
  JSFunctionSpec* methods;
  JSPropertySpec* properties;
  JSFunctionSpec* static_methods;
  JSPropertySpec* static_properties;
};

#define JS_AddRoot(cx, vp) JS_AddObjectRoot(cx, (JSObject**)(vp))
#define JS_RemoveRoot(cx, vp) JS_RemoveObjectRoot(cx, (JSObject**)(vp));
#define JSVAL_IS_FUNCTION(cx, var) (!JSVAL_IS_PRIMITIVE(var) && JS_ObjectIsFunction(cx, JSVAL_TO_OBJECT(var)))

#define JSPROP_PERMANENT_VAR (JSPROP_READONLY | JSPROP_ENUMERATE | JSPROP_PERMANENT)
#define JSPROP_STATIC_VAR (JSPROP_ENUMERATE | JSPROP_PERMANENT)

#define CLASS_CTOR(name) JSBool name##_ctor(JSContext* cx, uint argc, jsval* vp)

// #define EMPTY_CTOR(name) JSBool name##_ctor (JSContext* cx, uint argc, jsval* vp) { return THROW_ERROR(cx, #name " is not constructable."); }
//           #define EMPTY_CTOR(name) \
//JSBool name##_ctor (JSContext *cx, JSObject* obj, uint argc, jsval *argv, jsval *rval) { \
//	THROW_ERROR(cx, "Invalid Operation"); }

JSObject* BuildObject(JSContext* cx, JSClass* classp = NULL, JSFunctionSpec* funcs = NULL, JSPropertySpec* props = NULL, void* priv = NULL, JSObject* proto = NULL,
                      JSObject* parent = NULL);
JSScript* JS_CompileFile(JSContext* cx, JSObject* globalObject, std::wstring fileName);
#define THROW_ERROR(cx, msg) \
  {                          \
    JS_ReportError(cx, msg); \
    return JS_FALSE;         \
  }
#define THROW_WARNING(cx, vp, msg)    \
  {                                   \
    JS_ReportWarning(cx, msg);        \
    JS_SET_RVAL(cx, vp, JSVAL_FALSE); \
    return JS_TRUE;                   \
  }

#define JSPROP_DEFAULT JSPROP_ENUMERATE | JSPROP_PERMANENT
#define JSPROP_STATIC JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY

#define JSAPI_FUNC(name) JSBool name##(JSContext * cx, uint argc, jsval * vp)
#define FUNCTION_FLAGS JSFUN_STUB_GSOPS

#define EMPTY_CTOR(name)                            \
  JSBool name##_ctor(JSContext* cx, uint, jsval*) { \
    THROW_ERROR(cx, "Invalid Operation");           \
  }

#define JSAPI_PROP(name) JSBool name##(JSContext * cx, JSHandleObject obj, JSHandleId id, JSMutableHandleValue vp)
#define JSAPI_STRICT_PROP(name) JSBool name##(JSContext * cx, JSHandleObject obj, JSHandleId id, JSBool strict, JSMutableHandleValue vp)

#define JS_PS(name, id, flags, getter, setter) \
  { name, id, flags, getter, setter }

#define JS_PS_END JS_PS(0, 0, 0, 0, 0)

#define JS_CS(classp, proto, ctor, argc, methods, props, static_methods, static_props) \
  { classp, proto, ctor, argc, methods, props, static_methods, static_props }

#define JS_CS_END \
  { 0 }

#define JS_MS(name, classes, static_methods, static_properties) \
  { L##name, classes, static_methods, static_properties }

#define JS_MS_END \
  { 0, 0, 0, 0 }

#define JSCLASS_SPEC(add, del, get, set, enumerate, resolve, convert, finalize, ctor) \
  add, del, get, set, enumerate, resolve, convert, finalize, NULL, NULL, NULL, ctor, NULL

#define JSCLASS_DEFAULT_WITH_CTOR(ctor) \
  JSCLASS_SPEC(JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub, JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub, ctor)

#define JSCLASS_DEFAULT_STANDARD_MEMBERS \
  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub, JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub, JSCLASS_NO_OPTIONAL_MEMBERS
