#pragma comment(lib, "psapi.lib")  // Added to support GetProcessMemoryInfo()
#pragma once

// TODO(ejt): most of the content in here is only for migration purposes
// to minimize the work to get from mozjs to quickjs.

#pragma warning(push, 0)
#include <quickjs.h>
#pragma warning(pop)

#include <atomic>
#include <vector>
#include <io.h>
#include <string>
#include <fstream>
#include <optional>
#include <streambuf>
#include <cassert>

typedef struct {
  std::atomic<int> ref_count;
  uint64_t buf[1];
} JSSABHeader;

/* shared array buffer allocator */
inline static void* js_sab_alloc(void* /*opaque*/, size_t size) {
  JSSABHeader* sab;
  sab = static_cast<JSSABHeader*>(malloc(sizeof(JSSABHeader) + size));
  if (!sab)
    return NULL;
  sab->ref_count = 1;
  return sab->buf;
}

inline static void js_sab_free(void* /*opaque*/, void* ptr) {
  JSSABHeader* sab;
  int ref_count;
  sab = (JSSABHeader*)((uint8_t*)ptr - sizeof(JSSABHeader));
  ref_count = std::atomic_fetch_add(&sab->ref_count, -1);
  assert(ref_count >= 0);
  if (ref_count == 0) {
    free(sab);
  }
}

inline static void js_sab_dup(void* /*opaque*/, void* ptr) {
  JSSABHeader* sab;
  sab = (JSSABHeader*)((uint8_t*)ptr - sizeof(JSSABHeader));
  std::atomic_fetch_add(&sab->ref_count, 1);
}

JSValue JS_NewString(JSContext* ctx, const wchar_t* str);

struct JSClassSpec {
  const char* name;
  JSClassID* pclass_id;
  JSValue (*ctor)(JSContext* ctx, JSValueConst new_target, int argc, JSValueConst* argv);
  JSClassFinalizer* finalizer;
  uint32_t argc;
  JSCFunctionListEntry* proto_funcs;
  int num_proto_funcs;
  JSCFunctionListEntry* static_funcs;
  int num_static_funcs;
};

#define FUNCLIST(list) list, _countof(list)
#define EMPTY_FUNCLIST nullptr, 0

#define CLASS_CTOR(name) \
  JSValue name##_ctor([[maybe_unused]] JSContext* ctx, [[maybe_unused]] JSValueConst new_target, [[maybe_unused]] int argc, [[maybe_unused]] JSValueConst* argv)

#define CLASS_FINALIZER(name) void name##_finalizer([[maybe_unused]] JSRuntime* rt, [[maybe_unused]] JSValue val)

#define EMPTY_CTOR(name)                                     \
  CLASS_CTOR(name) {                                         \
    return JS_ThrowReferenceError(ctx, "Invalid Operation"); \
  }

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
    return JS_UNDEFINED;       \
  }

#define JSAPI_FUNC(name) \
  JSValue name##([[maybe_unused]] JSContext * ctx, [[maybe_unused]] JSValueConst this_val, [[maybe_unused]] int argc, [[maybe_unused]] JSValueConst* argv)

#define JSAPI_PROP(name) JSValue name##([[maybe_unused]] JSContext * ctx, [[maybe_unused]] JSValueConst this_val, [[maybe_unused]] int magic)
#define JSAPI_STRICT_PROP(name) \
  JSValue name##([[maybe_unused]] JSContext * ctx, [[maybe_unused]] JSValueConst this_val, [[maybe_unused]] JSValueConst val, [[maybe_unused]] int magic)
//
#define JS_FN(name, func, length, flags) JS_CFUNC_DEF(name, length, func)
#define JS_FS(name, func, length, flags) JS_CFUNC_DEF(name, length, func)