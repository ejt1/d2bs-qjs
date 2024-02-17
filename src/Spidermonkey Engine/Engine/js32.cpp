#include "js32.h"
//
// #include "Helpers.h"
#include "D2Helpers.h"
#include "Core.h"
#include "Globals.h"
#include "Console.h"
//
// JSValue JS_NewString(JSContext* ctx, const wchar_t* str) {
//  std::string utf8 = WideToAnsi(str);
//  return JS_NewString(ctx, utf8.c_str());
//}

JSScript* JS_CompileFile(JSContext* ctx, JS::HandleObject /*globalObject*/, std::string fileName) {
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

  JS::CompileOptions opts(ctx);
  opts.setFileAndLine(fileName.c_str(), 1);
  JS::RootedScript script(ctx);
  if (!JS_CompileScript(ctx, str.c_str(), str.length(), opts, &script)) {
    t.close();
    return nullptr;
  }
  t.close();

  return script;
}

// std::optional<std::string> JS_CStringToStd(JSContext* ctx, JSValue val) {
//   size_t len;
//   const char* ptr = JS_ToCStringLen(ctx, &len, val);
//   if (!ptr) {
//     return std::nullopt;
//   }
//   std::string s(ptr, len);
//   JS_FreeCString(ctx, ptr);
//   return s;
// }

void JS_LogReport(JSContext* ctx, JSErrorReport* report) {
  bool warn = JSREPORT_IS_WARNING(report->flags);
  bool isStrict = JSREPORT_IS_STRICT(report->flags);
  const char* type = (warn ? "Warning" : "Error");
  const char* strict = (isStrict ? "Strict " : "");
  const char* filename = report->filename ? report->filename : "<unknown>";
  const char* displayName = filename;
  if (_stricmp("Command Line", filename) != 0 && _stricmp("<unknown>", filename) != 0)
    displayName = filename + strlen(Vars.szPath);

  Log("[%hs%hs] Code(%d) File(%s:%d) %s", strict, type, report->errorNumber, filename, report->lineno, report->message().c_str());
  Print("[ÿc%d%hs%hsÿc0 (%d)] File(%s:%d) %s", (warn ? 9 : 1), strict, type, report->errorNumber, displayName, report->lineno, report->message().c_str());

  if (Vars.bQuitOnError && !JSREPORT_IS_WARNING(report->flags) && ClientState() == ClientStateInGame)
    D2CLIENT_ExitGame();
  else
    Console::ShowBuffer();
}

//  TODO(ejt): this function is a mess after throwing the old 'reportError' in here
void JS_ReportPendingException(JSContext* ctx) {
  JS::RootedValue ex(ctx);

  if (!JS_GetPendingException(ctx, &ex)) {
    return;
  }
  JS_ClearPendingException(ctx);
  JS::RootedObject error(ctx, ex.toObjectOrNull());
  JSErrorReport* report = JS_ErrorFromException(ctx, error);
  JS_LogReport(ctx, report);
}
