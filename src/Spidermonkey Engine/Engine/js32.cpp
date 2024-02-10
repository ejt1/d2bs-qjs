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

// TODO(ejt): this function is a mess after throwing the old 'reportError' in here
void JS_ReportPendingException(JSContext* ctx) {
  std::optional<std::string> what;
  std::optional<std::string> stackframe;
  JSValue ex;
  bool isError;


  ex = JS_GetException(ctx);
  // skip uncatchable errors to avoid logging "interrupted" exceptions
  if (JS_IsUncatchableError(ctx, ex)) {
    JS_FreeValue(ctx, ex);
    return;
  }

  isError = JS_IsError(ctx, ex);
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

  if (what) {
    if (stackframe) {
      Log("[%hs%hs] %s\n%s", strict, type, what->c_str(), stackframe->c_str());
      Print("[ÿc%d%hs%hsÿc0] %s\n%s", (warn ? 9 : 1), strict, type, what->c_str(), stackframe->c_str());
    } else {
      Log("[%hs%hs] %s", strict, type, what->c_str());
      Print("[ÿc%d%hs%hsÿc0] %s", (warn ? 9 : 1), strict, type, what->c_str());
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