#include "js32.h"

#include "Helpers.h"
#include "D2Helpers.h"
#include "Core.h"
#include "Globals.h"
#include "Console.h"

JSValue JS_NewString(JSContext* ctx, const wchar_t* str) {
  std::string utf8 = WideToAnsi(str);
  return JS_NewString(ctx, utf8.c_str());
}

JSValue BuildObject(JSContext* ctx, JSClassID class_id, JSCFunctionListEntry* funcs, size_t num_funcs, JSCFunctionListEntry* props, size_t num_props, void* opaque,
                    JSValue new_target) {
  JSValue proto;
  if (JS_IsUndefined(new_target)) {
    proto = JS_GetClassProto(ctx, class_id);
  } else {
    proto = JS_GetPropertyStr(ctx, new_target, "prototype");
    if (JS_IsException(proto)) {
      return JS_EXCEPTION;
    }
  }
  JSValue obj = JS_NewObjectProtoClass(ctx, proto, class_id);
  JS_FreeValue(ctx, proto);
  if (JS_IsException(obj)) {
    return obj;
  }

  JS_SetPropertyFunctionList(ctx, obj, funcs, num_funcs);
  JS_SetPropertyFunctionList(ctx, obj, props, num_props);
  if (opaque) {
    JS_SetOpaque(obj, opaque);
  }

  return obj;
}

JSValue JS_CompileFile(JSContext* ctx, JSValue /*globalObject*/, std::string fileName) {
  std::ifstream t(fileName.c_str(), std::ios::binary);
  std::string str;

  int ch1 = t.get();
  int ch2 = t.get();
  int ch3 = t.get();

  t.seekg(0, std::ios::end);
  str.resize(static_cast<size_t>(t.tellg()));
  t.seekg(0, std::ios::beg);

  str.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

  if (ch1 == 0xEF && ch2 == 0xBB && ch3 == 0xBF) {  // replace utf8-bom with space
    str[0] = ' ';
    str[1] = ' ';
    str[2] = ' ';
  }

  JSValue rval = JS_Eval(ctx, str.c_str(), str.size(), fileName.c_str(), JS_EVAL_TYPE_GLOBAL | JS_EVAL_FLAG_COMPILE_ONLY);
  t.close();

  return rval;
}

std::optional<std::string> JS_CStringToStd(JSContext* ctx, JSValue val) {
  size_t len;
  const char* ptr = JS_ToCStringLen(ctx, &len, val);
  if (!ptr) {
    return std::nullopt;
  }
  std::string s(ptr, len);
  JS_FreeCString(ctx, ptr);
  return s;
}

std::optional<std::wstring> JS_ToWString(JSContext* ctx, JSValue val) {
  const char* str = JS_ToCString(ctx, val);
  if (!str) {
    return std::nullopt;
  }
  std::wstring ret = AnsiToWide(str);
  JS_FreeCString(ctx, str);
  return ret;
}

// TODO(ejt): this function is a mess after throwing the old 'reportError' in here
void JS_ReportPendingException(JSContext* ctx) {
  std::optional<std::string> what;
  std::optional<std::string> stackframe;
  JSValue ex = JS_GetException(ctx);
  bool isError = JS_IsError(ctx, ex);
  if (isError) {
    what = JS_CStringToStd(ctx, ex);
    JSValue stack = JS_GetPropertyStr(ctx, ex, "stack");
    if (!JS_IsNull(stack) && !JS_IsUndefined(stack)) {
      stackframe = JS_CStringToStd(ctx, stack);
    }
    JS_FreeValue(ctx, stack);
  }
  JS_FreeValue(ctx, ex);

  bool warn = false;      // JSREPORT_IS_WARNING(report->flags);
  bool isStrict = false;  // JSREPORT_IS_STRICT(report->flags);
  const char* type = (warn ? "Warning" : "Error");
  const char* strict = (isStrict ? "Strict " : "");
  // wchar_t* filename = report->filename ? AnsiToUnicode(report->filename) : _wcsdup(L"<unknown>");
  // wchar_t* displayName = filename;
  // if (_wcsicmp(L"Command Line", filename) != 0 && _wcsicmp(L"<unknown>", filename) != 0)
  //   displayName = filename + wcslen(Vars.szPath);

  // Log(L"[%hs%hs] Code(%d) File(%s:%d) %hs\nLine: %hs", strict, type, report->errorNumber, filename, report->lineno, message, report->linebuf);
  // Print(L"[\u00FFc%d%hs%hs\u00FFc0 (%d)] File(%s:%d) %hs", (warn ? 9 : 1), strict, type, report->errorNumber, displayName, report->lineno, message);

  if (what) {
    if (stackframe) {
      Log(L"[%hs%hs] %S\n%S", strict, type, what->c_str(), stackframe->c_str());
      Print(L"[\u00FFc%d%hs%hs\u00FFc0] %S\n%S", (warn ? 9 : 1), strict, type, what->c_str(), stackframe->c_str());
    } else {
      Log(L"[%hs%hs] %S", strict, type, what->c_str());
      Print(L"[\u00FFc%d%hs%hs\u00FFc0] %S", (warn ? 9 : 1), strict, type, what->c_str());
    }
  }

  // if (filename[0] == L'<')
  //   free(filename);
  // else
  //   delete[] filename;

  if (Vars.bQuitOnError && /*!JSREPORT_IS_WARNING(report->flags) && */ ClientState() == ClientStateInGame)
    D2CLIENT_ExitGame();
  else
    Console::ShowBuffer();
}

// JSBool JSVAL_IS_OBJECT(JSValue v) {
//   return !JSVAL_IS_PRIMITIVE(v);
// }