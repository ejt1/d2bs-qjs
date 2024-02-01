#include "js32.h"

#include "Helpers.h"
#include "D2Helpers.h"
#include "Core.h"
#include "Globals.h"
#include "Console.h"

void* JS_GetPrivate(JSValue obj) {
  return JS_GetOpaque3(obj);
}

void JS_SetPrivate(JSValue obj, void* data) {
  return JS_SetOpaque(obj, data);
}

void* JS_GetPrivate(JSContext* /*ctx*/, JSValue obj) {
  return JS_GetOpaque3(obj);
}

void JS_SetPrivate(JSContext* /*ctx*/, JSValue obj, void* data) {
  return JS_SetOpaque(obj, data);
}

void* JS_GetContextPrivate(JSRuntime* rt) {
  return JS_GetRuntimeOpaque(rt);
}

void* JS_GetContextPrivate(JSContext* ctx) {
  return JS_GetContextOpaque(ctx);
}

void* JS_GetInstancePrivate(JSContext* /*ctx*/, JSValue val, JSClassID /*class_id*/, JSValue* /*argv*/) {
  return JS_GetOpaque3(val);
}

JSValue JS_NewString(JSContext* ctx, const wchar_t* str) {
  char* ansi_str = UnicodeToAnsi(str);
  JSValue rval = JS_NewString(ctx, ansi_str);
  delete[] ansi_str;
  return rval;
}

JSValue BuildObject(JSContext* ctx, JSClassID class_id, JSFunctionSpec* funcs, size_t num_funcs, JSPropertySpec* props, size_t num_props, void* opaque,
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
    JS_SetPrivate(ctx, obj, opaque);
  }

  return obj;
}

JSValue JS_CompileFile(JSContext* ctx, JSValue /*globalObject*/, std::wstring fileName) {
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

  char* nFileName = UnicodeToAnsi(fileName.c_str());
  JSValue rval = JS_Eval(ctx, str.c_str(), str.size(), nFileName, JS_EVAL_TYPE_GLOBAL | JS_EVAL_FLAG_COMPILE_ONLY);
  t.close();
  delete[] nFileName;

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
  const wchar_t* tmp = AnsiToUnicode(str);
  JS_FreeCString(ctx, str);
  std::wstring ret = tmp;
  delete[] tmp;
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
    wchar_t* unicode_what = AnsiToUnicode(what->c_str());
    if (stackframe) {
      wchar_t* unicode_stackframe = AnsiToUnicode(stackframe->c_str());
      Log(L"[%hs%hs] %s\n%s", strict, type, unicode_what, unicode_stackframe);
      Print(L"[\u00FFc%d%hs%hs\u00FFc0] %s\n%s", (warn ? 9 : 1), strict, type, unicode_what, unicode_stackframe);
      delete[] unicode_stackframe;
    } else {
      Log(L"[%hs%hs] %s", strict, type, unicode_what);
      Print(L"[\u00FFc%d%hs%hs\u00FFc0] %s", (warn ? 9 : 1), strict, type, unicode_what);
    }
    delete[] unicode_what;
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

// JSBool JSVAL_IS_OBJECT(jsval v) {
//   return !JSVAL_IS_PRIMITIVE(v);
// }