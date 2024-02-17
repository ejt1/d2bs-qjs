#pragma once

// TODO(ejt): most of the content in here is only for migration purposes
// to minimize the work to get from mozjs to quickjs.

#pragma warning(push, 0)
#include <jsapi.h>
#include <jsfriendapi.h>
#include <js/Conversions.h>
#pragma warning(pop)

#include <atomic>
#include <vector>
#include <io.h>
#include <string>
#include <fstream>
#include <optional>
#include <streambuf>
#include <cassert>

// typedef struct {
//   std::atomic<int> ref_count;
//   uint64_t buf[1];
// } JSSABHeader;
//
///* shared array buffer allocator */
// inline static void* js_sab_alloc(void* /*opaque*/, size_t size) {
//   JSSABHeader* sab;
//   sab = static_cast<JSSABHeader*>(malloc(sizeof(JSSABHeader) + size));
//   if (!sab)
//     return NULL;
//   sab->ref_count = 1;
//   return sab->buf;
// }
//
// inline static void js_sab_free(void* /*opaque*/, void* ptr) {
//   JSSABHeader* sab;
//   int ref_count;
//   sab = (JSSABHeader*)((uint8_t*)ptr - sizeof(JSSABHeader));
//   ref_count = std::atomic_fetch_add(&sab->ref_count, -1);
//   assert(ref_count >= 0);
//   if (ref_count == 0) {
//     free(sab);
//   }
// }
//
// inline static void js_sab_dup(void* /*opaque*/, void* ptr) {
//   JSSABHeader* sab;
//   sab = (JSSABHeader*)((uint8_t*)ptr - sizeof(JSSABHeader));
//   std::atomic_fetch_add(&sab->ref_count, 1);
// }
//
// JSValue JS_NewString(JSContext* ctx, const wchar_t* str);

JSScript* JS_CompileFile(JSContext* ctx, JS::HandleObject globalObject, std::string fileName);

// template <typename... Args>
// JSValue JS_ReportError(JSContext* ctx, const char* fmt, Args&&... args) {
//   return JS_ThrowSyntaxError(ctx, fmt, std::forward<Args>(args)...);
// }

void JS_LogReport(JSContext* ctx, JSErrorReport* report);

// TODO(ejt): rename JS_ReportAndClearPendingException to make it clear
// that this function both reports and clears the exception!
void JS_ReportPendingException(JSContext* ctx);

#define THROW_ERROR(cx, msg)      \
  {                               \
    Log(msg);                     \
    JS_ReportErrorASCII(cx, msg); \
    return false;                 \
  }

#define THROW_WARNING(cx, msg)       \
  {                                  \
    Log(msg);                        \
    JS_ReportWarningASCII(ctx, msg); \
    args.rval().setBoolean(false);   \
    return true;                     \
  }

// #define JSAPI_FUNC(name) \
//  JSValue name##([[maybe_unused]] JSContext * ctx, [[maybe_unused]] JSValueConst this_val, [[maybe_unused]] int argc, [[maybe_unused]] JSValueConst* argv)
