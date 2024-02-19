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

std::optional<std::string> ReadFileUTF8(const std::filesystem::path& filename);
std::optional<std::u16string> ReadFileUTF16(const std::filesystem::path& filename);

JSScript* JS_CompileFile(JSContext* ctx, JS::HandleObject globalObject, std::string fileName);

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
