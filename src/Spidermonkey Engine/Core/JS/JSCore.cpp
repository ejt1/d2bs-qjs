#include "JSCore.h"

#include <sstream>

//
// #include <io.h>
#include <windows.h>
#include <wininet.h>
// #include <iostream>
//
// #include "JSCore.h"
// #include "Events.h"
//
// #include "JSScript.h"

#include "Console.h"
#include "Core.h"
#include "D2Helpers.h"
#include "File.h"
#include "Globals.h"
#include "Helpers.h"
#include "Localization.h"
#include "ScriptEngine.h"
#include "StringWrap.h"

// TODO(ejt): is this function necessary, and does it even work?!
bool my_stringToEUC(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "stringToEUC", 1)) {
    return false;
  }
  if (args[0].isNullOrUndefined()) {
    args.rval().setUndefined();
    return true;
  }

  char* szText = JS_EncodeString(ctx, args[0].toString());
  if (!szText) {
    THROW_ERROR(ctx, "failed to encode string");
  }
  std::string ansi = UTF8ToANSI(szText);
  JS_free(ctx, szText);
  args.rval().setString(JS_NewStringCopyN(ctx, ansi.c_str(), ansi.length()));
  return true;
}

bool my_print(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  std::stringstream ss;
  for (uint32_t i = 0; i < argc; i++) {
    if (i != 0) {
      ss << " ";
    }
    char* str = JS_EncodeString(ctx, JS::ToString(ctx, args[i]));
    if (!str) {
      JS_ReportErrorASCII(ctx, "failed to encode string");
      return false;
    }
    ss << str;
    JS_free(ctx, str);
  }
  std::string finalstr = ss.str();
  Print(finalstr.c_str());
  args.rval().setUndefined();
  return true;
}

bool my_delay(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "delay", 1)) {
    return false;
  }
  uint32_t nDelay = args[0].isInt32() ? args[0].toInt32() : 1;
  ThreadState* ts = static_cast<ThreadState*>(JS_GetContextPrivate(ctx));

  if (!ts->script->BlockThread(nDelay)) {
    return false;
  }
  args.rval().setUndefined();
  return true;
}

bool my_load(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "load", 1)) {
    return false;
  }
  ThreadState* ts = static_cast<ThreadState*>(JS_GetContextPrivate(ctx));
  char* file = JS_EncodeString(ctx, args[0].toString());
  if (!file) {
    THROW_ERROR(ctx, "failed to encode string");
  }

  if (strlen(file) > (_MAX_FNAME + _MAX_PATH - strlen(Vars.szScriptPath))) {
    JS_free(ctx, file);
    JS_ReportErrorASCII(ctx, "File name too long!");
    return false;
  }

  ScriptMode scriptState = ts->script->GetMode();
  if (scriptState == kScriptModeCommand)
    scriptState = (ClientState() == ClientStateInGame ? kScriptModeGame : kScriptModeMenu);

  char buf[_MAX_PATH + _MAX_FNAME];
  sprintf_s(buf, _countof(buf), "%s\\%s", Vars.szScriptPath, file);
  StringReplace(buf, '/', '\\', _countof(buf));

  // JSAutoStructuredCloneBuffer** autoBuffer = new JSAutoStructuredCloneBuffer*[argc - 1];
  // for (uint32_t i = 1; i < argc; i++) {
  //   autoBuffer[i - 1] = new JSAutoStructuredCloneBuffer;
  //   autoBuffer[i - 1]->write(cx, JS_ARGV(cx, vp)[i]);
  // }

  Script* newScript = sScriptEngine->NewScript(buf, scriptState /*, argc - 1, autoBuffer*/);
  if (newScript) {
    newScript->Start();
  } else {
    // TODO: Should this actually be there? No notification is bad, but do we want this? maybe throw an exception?
    Print("File \"%s\" not found.", file);
  }
  JS_free(ctx, file);
  args.rval().setUndefined();
  return true;
}

bool my_include(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "include", 1)) {
    return false;
  }
  ThreadState* ts = static_cast<ThreadState*>(JS_GetContextPrivate(ctx));
  char* file = JS_EncodeString(ctx, args[0].toString());
  if (!file) {
    THROW_ERROR(ctx, "failed to encode string");
  }

  if (strlen(file) > (_MAX_FNAME + _MAX_PATH - strlen(Vars.szScriptPath) - 6)) {
    JS_free(ctx, file);
    JS_ReportErrorASCII(ctx, "File name too long!");
    return false;
  }

  char buf[_MAX_PATH + _MAX_FNAME];
  sprintf_s(buf, _countof(buf), "%s\\libs\\%s", Vars.szScriptPath, file);
  JS_free(ctx, file);

  args.rval().setBoolean(false);
  if (_access(buf, 0) == 0) {
    args.rval().setBoolean(ts->script->Include(buf));
  }

  return true;
}

bool my_stop(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

  if (args.length() > 0 && (args[0].isInt32() && args[0].toInt32() == 1) || (args[0].isBoolean() && args[0].toBoolean())) {
    ThreadState* ts = static_cast<ThreadState*>(JS_GetContextPrivate(ctx));
    ts->script->Stop();
  } else {
    sScriptEngine->StopAll();
  }

  args.rval().setUndefined();
  return true;
}

bool my_stacktrace(JSContext* /*ctx*/, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  Log("stacktrace is deprecated");
  args.rval().setUndefined();
  return true;
}

bool my_beep(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "beep", 1)) {
    return false;
  }

  if (!args[0].isInt32()) {
    THROW_ERROR(ctx, "invalid argument");
  }
  MessageBeep(args[0].toInt32());
  args.rval().setUndefined();
  return true;
}

bool my_getTickCount(JSContext* /*ctx*/, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  args.rval().setNumber(static_cast<double>(GetTickCount()));
  return true;
}

bool my_getThreadPriority(JSContext* /*ctx*/, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  args.rval().setInt32(GetThreadPriority(GetCurrentThread()));
  return true;
}

bool my_isIncluded(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "isIncluded", 1)) {
    return false;
  }
  char* file = JS_EncodeString(ctx, args[0].toString());
  if (!file) {
    THROW_ERROR(ctx, "failed to encode string");
  }

  if (strlen(file) > (_MAX_FNAME + _MAX_PATH - strlen(Vars.szScriptPath) - 6)) {
    JS_free(ctx, file);
    JS_ReportErrorASCII(ctx, "File name too long");
    return false;
  }

  char path[_MAX_FNAME + _MAX_PATH];
  sprintf_s(path, _countof(path), "%s\\libs\\%s", Vars.szScriptPath, file);
  JS_free(ctx, file);
  ThreadState* ts = static_cast<ThreadState*>(JS_GetContextPrivate(ctx));
  args.rval().setBoolean(ts->script->IsIncluded(path));
  return true;
}

bool my_version(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.hasDefined(0)) {
    args.rval().setString(JS_NewStringCopyN(ctx, D2BS_VERSION, strlen(D2BS_VERSION)));
    return true;
  }

  Print("ÿc4D2BSÿc1 ÿc3%s for Diablo II 1.14d.", D2BS_VERSION);
  args.rval().setUndefined();
  return true;
}

bool my_debugLog(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  std::stringstream ss;
  for (uint32_t i = 0; i < argc; i++) {
    if (i != 0) {
      ss << " ";
    }
    char* str = JS_EncodeString(ctx, args[i].toString());
    if (!str) {
      THROW_ERROR(ctx, "failed to encode string");
    }
    ss << str;
    JS_free(ctx, str);
  }
  std::string finalstr = ss.str();
  Log(finalstr.c_str());
  args.rval().setUndefined();
  return true;
}

bool my_copy(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "copy", 1)) {
    return false;
  }
  char* data = JS_EncodeString(ctx, args[0].toString());
  if (!data) {
    THROW_ERROR(ctx, "failed to encode string");
  }
  HGLOBAL hText;
  char* pText;
  hText = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, strlen(data) + 1);
  pText = (char*)GlobalLock(hText);

  strcpy_s(pText, strlen(data) + 1, data);
  GlobalUnlock(hText);

  OpenClipboard(NULL);
  EmptyClipboard();
  SetClipboardData(CF_TEXT, hText);
  CloseClipboard();
  JS_free(ctx, data);
  args.rval().setUndefined();
  return true;
}

bool my_paste(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  OpenClipboard(NULL);
  HANDLE foo = GetClipboardData(CF_TEXT);
  CloseClipboard();
  LPVOID lptstr = GlobalLock(foo);
  args.rval().setString(JS_NewStringCopyZ(ctx, static_cast<const char*>(lptstr)));
  return true;
}

bool my_sendCopyData(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "sendCopyData", 4)) {
    return false;
  }

  StringWrap windowClassName;
  StringWrap windowName;
  StringWrap data;
  int32_t nModeId = NULL;
  HWND hWnd = NULL;

  if (args[0].isString() && !args[0].isNullOrUndefined()) {
    windowClassName = std::move(StringWrap{ctx, args[0]});
  }

  if (!args[1].isNull()) {
    if (args[1].isInt32()) {
      JS::ToUint32(ctx, args[1], reinterpret_cast<uint32_t*>(&hWnd));
    } else if (args[1].isString()) {
      windowName = std::move(StringWrap{ctx, args[1]});
    }
  }

  if (args[2].isInt32() && !args[2].isNull()) {
    nModeId = args[2].toInt32();
  }

  if (args[3].isString() && !args[3].isNull()) {
    data = std::move(StringWrap{ctx, args[3]});
  }

  if (hWnd == NULL && windowName != NULL) {
    hWnd = FindWindowA(windowClassName, windowName);
    if (!hWnd) {
      THROW_ERROR(ctx, "failed to find window");
    }
  }

  // if data is NULL, strlen crashes
  COPYDATASTRUCT aCopy{};
  if (!data) {
    aCopy = {static_cast<ULONG_PTR>(nModeId), 0, nullptr};
  } else {
    aCopy = {static_cast<ULONG_PTR>(nModeId), data.length(), (LPVOID)data.c_str()};
  }

  args.rval().setNumber(static_cast<double>(SendMessage(hWnd, WM_COPYDATA, (WPARAM)D2GFX_GetHwnd(), (LPARAM)&aCopy)));
  return true;
}

bool my_keystate(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "keystate", 1)) {
    return false;
  }
  if (!args[0].isInt32()) {
    THROW_ERROR(ctx, "invalid argument");
  }

  int vKey = args[0].toInt32();
  args.rval().setBoolean(static_cast<bool>(GetAsyncKeyState(vKey)));
  return true;
}

bool my_addEventListener(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "addEventListener", 2)) {
    return false;
  }

  if (args[0].isString() && JS::IsCallable(args[1].toObjectOrNull())) {
    char* evtName = JS_EncodeString(ctx, args[0].toString());
    if (evtName && strlen(evtName)) {
      ThreadState* ts = static_cast<ThreadState*>(JS_GetContextPrivate(ctx));
      JS::RootedValue func(ctx, args[1]);
      ts->script->AddEventListener(evtName, func);
    } else {
      JS_ReportErrorASCII(ctx, "Event name is invalid!");
      return false;
    }
    JS_free(ctx, evtName);
  }
  args.rval().setUndefined();
  return true;
}

bool my_removeEventListener(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "removeEventListener", 2)) {
    return false;
  }
  if (args[0].isString() && JS::IsCallable(args[1].toObjectOrNull())) {
    char* evtName = JS_EncodeString(ctx, args[0].toString());
    if (evtName && strlen(evtName)) {
      ThreadState* ts = static_cast<ThreadState*>(JS_GetContextPrivate(ctx));
      JS::RootedValue func(ctx, args[1]);
      ts->script->RemoveEventListener(evtName, func);
    } else {
      JS_ReportErrorASCII(ctx, "Event name is invalid!");
      return false;
    }
    JS_free(ctx, evtName);
  }
  args.rval().setUndefined();
  return true;
}

bool my_clearEvent(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "clearEvent", 1)) {
    return false;
  }
  if (args[0].isString()) {
    ThreadState* ts = static_cast<ThreadState*>(JS_GetContextPrivate(ctx));
    char* evt = JS_EncodeString(ctx, args[0].toString());
    ts->script->RemoveAllListeners(evt);
    JS_free(ctx, evt);
  }
  args.rval().setUndefined();
  return true;
}

bool my_clearAllEvents(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  ThreadState* ts = static_cast<ThreadState*>(JS_GetContextPrivate(ctx));
  ts->script->RemoveAllEventListeners();
  args.rval().setUndefined();
  return true;
}

bool my_js_strict(JSContext* /*ctx*/, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (argc == NULL) {
    args.rval().setBoolean(true);
    return true;
  }

  if (argc == 1) {
    // NOTE(ejt): disabled setting strict this way for now
    //
    // bool bFlag = ((JS_GetOptions(cx) & JSOPTION_STRICT) == JSOPTION_STRICT);
    // if (JSVAL_TO_BOOLEAN(JS_ARGV(cx, vp)[0])) {
    //  if (!bFlag)
    //    JS_ToggleOptions(cx, JSOPTION_STRICT);
    //} else {
    //  if (bFlag)
    //    JS_ToggleOptions(cx, JSOPTION_STRICT);
    //}
  }

  args.rval().setUndefined();
  return true;
}

bool my_scriptBroadcast(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "scriptBroadcast", 1)) {
    return false;
  }

  // documentation says to not use args.array but we do anyway because we're badasses
  ScriptBroadcastEvent(ctx, argc, args.array());
  args.rval().setUndefined();
  return true;
}

bool my_showConsole(JSContext* /*ctx*/, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  Console::Show();
  args.rval().setUndefined();
  return true;
}

bool my_hideConsole(JSContext* /*ctx*/, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  Console::Hide();
  args.rval().setUndefined();
  return true;
}

bool my_handler(JSContext* /*ctx*/, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  args.rval().setInt32(reinterpret_cast<int32_t>(Vars.hHandle));
  return true;
}

bool my_loadMpq(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "loadMpq", 1)) {
    return false;
  }
  char* path = JS_EncodeString(ctx, args[0].toString());
  if (!path) {
    THROW_ERROR(ctx, "failed to encode string");
  }
  if (isValidPath(path)) {
    LoadMPQ(path);
  }
  JS_free(ctx, path);
  args.rval().setUndefined();
  return true;
}

bool my_sendPacket(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "sendPacket", 1)) {
    return false;
  }
  if (!Vars.bEnableUnsupported) {
    JS_ReportWarningASCII(ctx, "sendPacket requires EnableUnsupported = true in d2bs.ini");
    args.rval().setBoolean(false);
    return true;
  }

  BYTE* aPacket;
  bool del = false;
  uint32_t len = 0;

  if (args[0].isObject()) {
    JS::RootedObject obj(ctx, args[0].toObjectOrNull());
    if (!JS_IsArrayBufferObject(obj)) {
      THROW_WARNING(ctx, "invalid ArrayBuffer parameter");
    }
    JS::AutoAssertNoGC hazard(ctx);
    bool isSharedMemory;
    len = JS_GetArrayBufferByteLength(obj);
    aPacket = JS_GetArrayBufferData(obj, &isSharedMemory, hazard);
    if (!aPacket) {
      JS_ReportErrorASCII(ctx, "getPacket: invalid ArrayBuffer parameter");
      return false;
    }
  } else {
    if (argc % 2 != 0) {
      JS_ReportWarningASCII(ctx, "invalid packet format");
      args.rval().setBoolean(false);
      return true;
    }

    aPacket = new BYTE[2 * argc];
    del = true;
    uint32_t size = 0;

    for (uint32_t i = 0; i < argc; i += 2, len += size) {
      JS::ToUint32(ctx, args[i], &size);
      JS::ToUint32(ctx, args[i + 1], (uint32_t*)&aPacket[len]);
      size = args[i].toInt32();
      aPacket[len] = static_cast<BYTE>(args[i + 1].toInt32());
    }
  }

  D2NET_SendPacket(len, 1, aPacket);
  if (del) {
    delete[] aPacket;
  }
  args.rval().setBoolean(true);
  return true;
}

bool my_getPacket(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "getPacket", 1)) {
    return false;
  }
  if (!Vars.bEnableUnsupported) {
    JS_ReportWarningASCII(ctx, "sendPacket requires EnableUnsupported = true in d2bs.ini");
    args.rval().setBoolean(false);
    return true;
  }

  BYTE* aPacket;
  bool del = false;
  uint32_t len = 0;

  bool isArray;
  if (JS_IsArrayObject(ctx, args[0], &isArray) && isArray) {
    JS::AutoAssertNoGC hazard(ctx);
    bool isSharedMemory;
    len = JS_GetArrayBufferByteLength(args[0].toObjectOrNull());
    aPacket = JS_GetArrayBufferData(args[0].toObjectOrNull(), &isSharedMemory, hazard);
    if (!aPacket) {
      JS_ReportErrorASCII(ctx, "getPacket: invalid ArrayBuffer parameter");
      return false;
    }
  } else {
    if (argc % 2 != 0) {
      JS_ReportWarningASCII(ctx, "invalid packet format");
      args.rval().setBoolean(false);
      return true;
    }

    aPacket = new BYTE[2 * argc];
    del = true;
    uint32_t size = 0;

    for (uint32_t i = 0; i < argc; i += 2, len += size) {
      size = args[i].toInt32();
      aPacket[len] = static_cast<BYTE>(args[i + 1].toInt32());
    }
  }

  D2NET_ReceivePacket(aPacket, len);
  if (del) {
    delete[] aPacket;
  }
  args.rval().setBoolean(true);
  return true;
}

#pragma comment(lib, "wininet")
bool my_getIP(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  HINTERNET hInternet, hFile;
  DWORD rSize;
  char buffer[32];

  hInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
  hFile = InternetOpenUrl(hInternet, "http://ipv4bot.whatismyipaddress.com", NULL, 0, INTERNET_FLAG_RELOAD, 0);
  InternetReadFile(hFile, &buffer, sizeof(buffer), &rSize);
  buffer[std::min(static_cast<uint32_t>(rSize), 31u)] = '\0';
  InternetCloseHandle(hFile);
  InternetCloseHandle(hInternet);
  args.rval().setString(JS_NewStringCopyZ(ctx, buffer));
  return true;
}

bool my_sendClick(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "sendClick", 2)) {
    return false;
  }
  uint32_t x = args[0].isInt32() ? args[0].toInt32() : 0;
  uint32_t y = args[1].isInt32() ? args[1].toInt32() : 0;

  if (!JS::ToUint32(ctx, args[0], &x) || !JS::ToUint32(ctx, args[1], &y)) {
    JS_ReportErrorASCII(ctx, "failed to convert values to uint32");
    return false;
  }
  Vars.bIgnoreMouse = TRUE;
  Sleep(100);
  SendMouseClick(x, y, 0);
  Sleep(100);
  SendMouseClick(x, y, 1);
  Sleep(100);
  Vars.bIgnoreMouse = FALSE;
  args.rval().setNull();
  return true;
}

bool my_sendKey(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  if (!args.requireAtLeast(ctx, "sendKey", 1)) {
    return false;
  }
  uint32_t key;
  if (!JS::ToUint32(ctx, args[0], &key)) {
    JS_ReportErrorASCII(ctx, "failed to convert values to uint32");
    return false;
  }
  BOOL prompt = Console::IsEnabled();
  if (prompt) {
    Console::HidePrompt();
  }
  Vars.bIgnoreKeys = TRUE;
  Sleep(100);
  SendKeyPress(WM_KEYDOWN, key, 0);
  Sleep(100);
  SendKeyPress(WM_KEYUP, key, 0);
  Sleep(100);
  Vars.bIgnoreKeys = FALSE;
  if (prompt) {
    Console::ShowPrompt();
  }
  args.rval().setNull();
  return true;
}
