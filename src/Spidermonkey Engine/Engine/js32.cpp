#include "js32.h"

#include "D2Helpers.h"
#include "Core.h"
#include "Globals.h"
#include "Console.h"

#include <codecvt>

std::optional<std::string> ReadFileUTF8(const std::filesystem::path& filename) {
  if (!std::filesystem::exists(filename)) {
    return std::nullopt;
  }

  FILE* fp;
  if (fopen_s(&fp, filename.string().c_str(), "rb")) {
    return std::nullopt;
  }
  fseek(fp, 0, SEEK_END);
  size_t size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  std::string code(size, 0);
  fread(code.data(), 1, code.length(), fp);
  fclose(fp);
  return code;
}

std::optional<std::u16string> ReadFileUTF16(const std::filesystem::path& filename) {
  std::optional<std::string> utf8_code = ReadFileUTF8(filename);
  if (!utf8_code) {
    return std::nullopt;
  }

  // convert utf8 -> utf16
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
  return convert.from_bytes(utf8_code.value());
}

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
  JS::SourceText<mozilla::Utf8Unit> source;
  if (!source.init(ctx, str.c_str(), str.length(), JS::SourceOwnership::Borrowed)) {
    return nullptr;
  }

  return JS::Compile(ctx, opts, source);
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
  bool warn = report->isWarning();
  bool isStrict = false; //JSREPORT_IS_STRICT(report->flags);
  const char* type = (warn ? "Warning" : "Error");
  const char* strict = (isStrict ? "Strict " : "");
  const char* filename = report->filename ? report->filename : "<unknown>";
  const char* displayName = filename;
  if (_stricmp("Command Line", filename) != 0 && _stricmp("<unknown>", filename) != 0)
    displayName = filename + strlen(Vars.szPath);

  Log("[%hs%hs] Code(%d) File(%s:%d) %s", strict, type, report->errorNumber, filename, report->lineno, report->message().c_str());
  Print("[ÿc%d%hs%hsÿc0 (%d)] File(%s:%d) %s", (warn ? 9 : 1), strict, type, report->errorNumber, displayName, report->lineno, report->message().c_str());

  if (Vars.bQuitOnError && !report->isWarning() && ClientState() == ClientStateInGame)
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
