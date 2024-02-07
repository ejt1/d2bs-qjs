#include "JSCore.h"

#include <sstream>

#include "D2Helpers.h"

#include <io.h>
#include <windows.h>
#include <wininet.h>
#include <iostream>

#include "JSCore.h"
#include "Core.h"
#include "Helpers.h"
#include "Events.h"
#include "Console.h"
#include "File.h"

#include "JSScript.h"

// TODO(ejt): is this function necessary, and does it even work?!
JSAPI_FUNC(my_stringToEUC) {
  if (argc == 0 || JS_IsNull(argv[0])) {
    return JS_NULL;
  }

  const char* szText = JS_ToCString(ctx, argv[0]);
  if (!szText) {
    return JS_EXCEPTION;
  }
  std::string ansi = UTF8ToANSI(szText);
  JS_FreeCString(ctx, szText);
  return JS_NewString(ctx, ansi.c_str());
}

JSAPI_FUNC(my_print) {
  std::stringstream ss;
  for (int32_t i = 0; i < argc; i++) {
    if (i != 0) {
      ss << " ";
    }
    size_t len;
    const char* str = JS_ToCStringLen(ctx, &len, argv[i]);
    if (!str) {
      return JS_EXCEPTION;
    }
    ss << str;
    JS_FreeCString(ctx, str);
  }
  std::string finalstr = ss.str();
  Print(finalstr.c_str());
  return JS_NULL;
}

// TODO(ejt): setTimeout, setInterval and clearInterval is not currently used by kolbot (it overrides it).
// The plan is to use libuv timers once we get to adding libuv, for now just throw an error.
JSAPI_FUNC(my_setTimeout) {
  JS_ThrowInternalError(ctx, "setTimeout temporarily disabled during development");
  return JS_EXCEPTION;

  // JS_SET_RVAL(cx, vp, JSVAL_NULL);

  // if (argc < 2 || !JSVAL_IS_NUMBER(JS_ARGV(cx, vp)[1]))
  //   JS_ReportError(cx, "invalid params passed to setTimeout");

  // if (JSVAL_IS_FUNCTION(cx, JS_ARGV(cx, vp)[0]) && JSVAL_IS_NUMBER(JS_ARGV(cx, vp)[1])) {
  //   Script* self = (Script*)JS_GetContextOpaque(cx);
  //   int freq = JSVAL_TO_INT(JS_ARGV(cx, vp)[1]);
  //   self->AddEventListener("setTimeout", JS_ARGV(cx, vp)[0]);
  //   Event* evt = new Event;
  //   evt->owner = self;
  //   evt->name = "setTimeout";
  //   evt->arg3 = new JSValue(JS_ARGV(cx, vp)[0]);
  //   JS_SET_RVAL(cx, vp, INT_TO_JSVAL(sScriptEngine->AddDelayedEvent(evt, freq)));
  // }

  // return JS_TRUE;
}

JSAPI_FUNC(my_setInterval) {
  JS_ThrowInternalError(ctx, "setInterval temporarily disabled during development");
  return JS_EXCEPTION;

  // JS_SET_RVAL(cx, vp, JSVAL_NULL);

  // if (argc < 2 || !JSVAL_IS_NUMBER(JS_ARGV(cx, vp)[1]))
  //   JS_ReportError(cx, "invalid params passed to setInterval");

  // if (JSVAL_IS_FUNCTION(cx, JS_ARGV(cx, vp)[0]) && JSVAL_IS_NUMBER(JS_ARGV(cx, vp)[1])) {
  //   Script* self = (Script*)JS_GetContextOpaque(cx);
  //   int freq = JSVAL_TO_INT(JS_ARGV(cx, vp)[1]);
  //   self->AddEventListener("setInterval", JS_ARGV(cx, vp)[0]);
  //   Event* evt = new Event;
  //   evt->owner = self;
  //   evt->name = "setInterval";
  //   evt->arg3 = new JSValue(JS_ARGV(cx, vp)[0]);
  //   JS_SET_RVAL(cx, vp, INT_TO_JSVAL(sScriptEngine->AddDelayedEvent(evt, freq)));
  // }

  // return JS_TRUE;
}
JSAPI_FUNC(my_clearInterval) {
  JS_ThrowInternalError(ctx, "clearInterval temporarily disabled during development");
  return JS_EXCEPTION;

  // JS_SET_RVAL(cx, vp, JSVAL_NULL);
  // if (argc != 1 || !JSVAL_IS_NUMBER(JS_ARGV(cx, vp)[0]))
  //   JS_ReportError(cx, "invalid params passed to clearInterval");

  // sScriptEngine->RemoveDelayedEvent(JSVAL_TO_INT(JS_ARGV(cx, vp)[0]));
  // return JS_TRUE;
}

JSAPI_FUNC(my_delay) {
  uint32_t nDelay = 0;
  if (JS_ToUint32(ctx, &nDelay, argv[0])) {
    return JS_EXCEPTION;
  }

  Script* script = (Script*)JS_GetContextOpaque(ctx);

  if (nDelay) {  // loop so we can exec events while in delay
    script->BlockThread(nDelay);
  } else {
    THROW_ERROR(ctx, "delay(0) called, argument must be >= 1");
  }
  return JS_UNDEFINED;
}

JSAPI_FUNC(my_load) {
  Script* script = (Script*)JS_GetContextOpaque(ctx);
  if (!script) {
    JS_ReportError(ctx, "Failed to get script object");
    return JS_EXCEPTION;
  }

  const char* file = JS_ToCString(ctx, argv[0]);
  if (!file) {
    return JS_EXCEPTION;
  }

  if (strlen(file) > (_MAX_FNAME + _MAX_PATH - strlen(Vars.szScriptPath))) {
    JS_FreeCString(ctx, file);
    JS_ReportError(ctx, "File name too long!");
    return JS_EXCEPTION;
  }

  ScriptMode scriptState = script->GetMode();
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
  JS_FreeCString(ctx, file);
  return JS_NULL;
}

JSAPI_FUNC(my_include) {
  Script* script = (Script*)JS_GetContextOpaque(ctx);
  if (!script) {
    JS_ReportError(ctx, "Failed to get script object");
    return JS_EXCEPTION;
  }

  const char* file = JS_ToCString(ctx, argv[0]);
  if (!file) {
    return JS_EXCEPTION;
  }

  if (strlen(file) > (_MAX_FNAME + _MAX_PATH - strlen(Vars.szScriptPath) - 6)) {
    JS_FreeCString(ctx, file);
    JS_ReportError(ctx, "File name too long!");
    return JS_EXCEPTION;
  }

  char buf[_MAX_PATH + _MAX_FNAME];
  sprintf_s(buf, _countof(buf), "%s\\libs\\%s", Vars.szScriptPath, file);
  JS_FreeCString(ctx, file);

  if (_access(buf, 0) == 0)
    return JS_NewBool(ctx, script->Include(buf));

  return JS_FALSE;
}

JSAPI_FUNC(my_stop) {
  int ival;
  if (argc > 0 && (JS_IsNumber(argv[0]) && (!JS_ToInt32(ctx, &ival, argv[0]) && ival == 1)) || (JS_IsBool(argv[0]) && JS_ToBool(ctx, argv[0]) == TRUE)) {
    Script* script = (Script*)JS_GetContextOpaque(ctx);
    if (script)
      script->Stop();
  } else
    sScriptEngine->StopAll();
  return JS_EXCEPTION;
}

JSAPI_FUNC(my_stacktrace) {
  Log("stacktrace is deprecated");
  return JS_TRUE;
}

JSAPI_FUNC(my_beep) {
  int32_t nBeepId = NULL;

  if (argc > 0 && JS_IsNumber(argv[0])) {
    JS_ToInt32(ctx, &nBeepId, argv[0]);
  }

  MessageBeep(nBeepId);
  return JS_TRUE;
}

JSAPI_FUNC(my_getTickCount) {
  return JS_NewFloat64(ctx, GetTickCount());
}

JSAPI_FUNC(my_getThreadPriority) {
  return JS_NewUint32(ctx, (uint32_t)GetCurrentThread());
}

JSAPI_FUNC(my_isIncluded) {
  const char* file = JS_ToCString(ctx, argv[0]);
  if (!file) {
    return JS_EXCEPTION;
  }

  if (strlen(file) > (_MAX_FNAME + _MAX_PATH - strlen(Vars.szScriptPath) - 6)) {
    JS_FreeCString(ctx, file);
    JS_ReportError(ctx, "File name too long");
    return JS_EXCEPTION;
  }

  char path[_MAX_FNAME + _MAX_PATH];
  sprintf_s(path, _countof(path), "%s\\libs\\%s", Vars.szScriptPath, file);
  JS_FreeCString(ctx, file);
  Script* script = (Script*)JS_GetContextOpaque(ctx);
  return JS_NewBool(ctx, script->IsIncluded(path));
}

JSAPI_FUNC(my_version) {
  if (argc < 1) {
    return JS_NewString(ctx, D2BS_VERSION);
  }

  Print("ÿc4D2BSÿc1 ÿc3%s for Diablo II 1.14d.", D2BS_VERSION);
  return JS_TRUE;
}

JSAPI_FUNC(my_debugLog) {
  std::stringstream ss;
  for (int32_t i = 0; i < argc; i++) {
    if (i != 0) {
      ss << " ";
    }
    size_t len;
    const char* str = JS_ToCStringLen(ctx, &len, argv[i]);
    if (!str) {
      return JS_EXCEPTION;
    }
    ss << str;
    JS_FreeCString(ctx, str);
  }
  std::string finalstr = ss.str();
  Log(finalstr.c_str());
  return JS_UNDEFINED;
}

JSAPI_FUNC(my_copy) {
  const char* data = JS_ToCString(ctx, argv[0]);
  if (!data) {
    return JS_EXCEPTION;
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
  JS_FreeCString(ctx, data);
  return JS_UNDEFINED;
}

JSAPI_FUNC(my_paste) {
  OpenClipboard(NULL);
  HANDLE foo = GetClipboardData(CF_TEXT);
  CloseClipboard();
  LPVOID lptstr = GlobalLock(foo);
  return JS_NewString(ctx, (const char*)lptstr);
}

JSAPI_FUNC(my_sendCopyData) {
  if (argc < 4) {
    return JS_FALSE;
  }

  const char *windowClassName = NULL, *windowName = NULL;
  const char* data = NULL;
  int32_t nModeId = NULL;
  HWND hWnd = NULL;

  if (JS_IsString(argv[0]) && !JS_IsNull(argv[0])) {
    windowClassName = JS_ToCString(ctx, argv[0]);
  }

  if (!JS_IsNull(argv[1])) {
    if (JS_IsNumber(argv[1])) {
      JS_ToUint32(ctx, (uint32_t*)&hWnd, argv[1]);
    } else if (JS_IsString(argv[1])) {
      windowName = JS_ToCString(ctx, argv[1]);
    }
  }

  if (JS_IsNumber(argv[2]) && !JS_IsNull(argv[2])) {
    JS_ToUint32(ctx, (uint32_t*)&nModeId, argv[2]);
  }

  if (JS_IsString(argv[3]) && !JS_IsNull(argv[3])) {
    data = JS_ToCString(ctx, argv[3]);
  }

  if (hWnd == NULL && windowName != NULL) {
    hWnd = FindWindowA(windowClassName, windowName);
    if (!hWnd) {
      JS_FreeCString(ctx, windowName);
      JS_FreeCString(ctx, windowClassName);
      JS_FreeCString(ctx, data);
      return JS_FALSE;
    }
  }

  // if data is NULL, strlen crashes
  COPYDATASTRUCT aCopy{};
  if (data == NULL) {
    aCopy = {static_cast<ULONG_PTR>(nModeId), 0, nullptr};
  } else {
    aCopy = {static_cast<ULONG_PTR>(nModeId), strlen(data) + 1, (LPVOID)data};
  }

  JSValue rval = JS_NewInt64(ctx, SendMessage(hWnd, WM_COPYDATA, (WPARAM)D2GFX_GetHwnd(), (LPARAM)&aCopy));
  JS_FreeCString(ctx, windowName);
  JS_FreeCString(ctx, windowClassName);
  JS_FreeCString(ctx, data);
  return rval;
}

JSAPI_FUNC(my_keystate) {
  if (argc < 1 || !JS_IsNumber(argv[0]))
    return JS_FALSE;

  int vKey;
  JS_ToInt32(ctx, &vKey, argv[0]);
  return JS_NewBool(ctx, !!GetAsyncKeyState(vKey));
}

JSAPI_FUNC(my_addEventListener) {
  if (JS_IsString(argv[0]) && JS_IsFunction(ctx, argv[1])) {
    const char* evtName = JS_ToCString(ctx, argv[0]);
    if (evtName && strlen(evtName)) {
      Script* self = (Script*)JS_GetContextOpaque(ctx);
      self->AddEventListener(evtName, argv[1]);
    } else {
      THROW_ERROR(ctx, "Event name is invalid!");
    }
    JS_FreeCString(ctx, evtName);
  }
  return JS_UNDEFINED;
}

JSAPI_FUNC(my_removeEventListener) {
  if (JS_IsString(argv[0]) && JS_IsFunction(ctx, argv[1])) {
    const char* evtName = JS_ToCString(ctx, argv[0]);
    if (evtName && strlen(evtName)) {
      Script* self = (Script*)JS_GetContextOpaque(ctx);
      self->RemoveEventListener(evtName, argv[1]);
    } else {
      THROW_ERROR(ctx, "Event name is invalid!");
    }
    JS_FreeCString(ctx, evtName);
  }
  return JS_UNDEFINED;
}

JSAPI_FUNC(my_clearEvent) {
  if (JS_IsString(argv[0])) {
    Script* self = (Script*)JS_GetContextOpaque(ctx);
    const char* evt = JS_ToCString(ctx, argv[0]);
    self->RemoveAllListeners(evt);
    JS_FreeCString(ctx, evt);
  }
  return JS_UNDEFINED;
}

JSAPI_FUNC(my_clearAllEvents) {
  Script* self = (Script*)JS_GetContextOpaque(ctx);
  self->RemoveAllEventListeners();
  return JS_UNDEFINED;
}

JSAPI_FUNC(my_js_strict) {
  if (argc == NULL) {
    return JS_NewBool(ctx, true);
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

  return JS_UNDEFINED;
}

JSAPI_FUNC(my_scriptBroadcast) {
  if (argc < 1)
    THROW_ERROR(ctx, "You must specify something to broadcast");

  ScriptBroadcastEvent(ctx, argv);
  return JS_NULL;
}

JSAPI_FUNC(my_showConsole) {
  Console::Show();
  return JS_NULL;
}

JSAPI_FUNC(my_hideConsole) {
  Console::Hide();
  return JS_NULL;
}

JSAPI_FUNC(my_handler) {
  return JS_NewUint32(ctx, (uint32_t)Vars.hHandle);
}

JSAPI_FUNC(my_loadMpq) {
  const char* path = JS_ToCString(ctx, argv[0]);
  if (!path) {
    return JS_EXCEPTION;
  }
  if (isValidPath(path)) {
    LoadMPQ(path);
  }
  JS_FreeCString(ctx, path);
  return JS_NULL;
}

JSAPI_FUNC(my_sendPacket) {
  if (!Vars.bEnableUnsupported) {
    THROW_WARNING(ctx, "sendPacket requires EnableUnsupported = true in d2bs.ini");
  }

  BYTE* aPacket;
  bool del = false;
  uint32_t len = 0;

  if (JS_IsObject(argv[0])) {
    aPacket = JS_GetArrayBuffer(ctx, &len, argv[0]);
    if (!aPacket) {
      THROW_ERROR(ctx, "getPacket: invalid ArrayBuffer parameter");
    }
  } else {
    if (argc % 2 != 0) {
      THROW_WARNING(ctx, "invalid packet format");
    }

    aPacket = new BYTE[2 * argc];
    del = true;
    uint32_t size = 0;

    for (int32_t i = 0; i < argc; i += 2, len += size) {
      JS_ToUint32(ctx, &size, argv[i]);
      JS_ToUint32(ctx, (uint32_t*)&aPacket[len], argv[i + 1]);
    }
  }

  D2NET_SendPacket(len, 1, aPacket);
  if (del) {
    delete[] aPacket;
  }
  return JS_TRUE;
}

JSAPI_FUNC(my_getPacket) {
  if (!Vars.bEnableUnsupported) {
    THROW_WARNING(ctx, "getPacket requires EnableUnsupported = true in d2bs.ini");
  }

  BYTE* aPacket;
  bool del = false;
  uint32_t len = 0;

  if (JS_IsObject(argv[0])) {
    aPacket = JS_GetArrayBuffer(ctx, &len, argv[0]);
    if (!aPacket) {
      THROW_ERROR(ctx, "getPacket: invalid ArrayBuffer parameter");
    }
  } else {
    if (argc % 2 != 0) {
      THROW_WARNING(ctx, "invalid packet format");
    }

    aPacket = new BYTE[2 * argc];
    del = true;
    uint32_t size = 0;

    for (int32_t i = 0; i < argc; i += 2, len += size) {
      JS_ToUint32(ctx, &size, argv[i]);
      JS_ToUint32(ctx, (uint32_t*)&aPacket[len], argv[i + 1]);
    }
  }

  D2NET_ReceivePacket(aPacket, len);
  if (del) {
    delete[] aPacket;
  }
  return JS_TRUE;
}

#pragma comment(lib, "wininet")
JSAPI_FUNC(my_getIP) {
  HINTERNET hInternet, hFile;
  DWORD rSize;
  char buffer[32];

  hInternet = InternetOpen(NULL, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
  hFile = InternetOpenUrl(hInternet, "http://ipv4bot.whatismyipaddress.com", NULL, 0, INTERNET_FLAG_RELOAD, 0);
  InternetReadFile(hFile, &buffer, sizeof(buffer), &rSize);
  buffer[min(rSize, 31)] = '\0';
  InternetCloseHandle(hFile);
  InternetCloseHandle(hInternet);
  return JS_NewString(ctx, buffer);
}

JSAPI_FUNC(my_sendClick) {
  uint32_t x = 0;
  uint32_t y = 0;
  if (JS_ToUint32(ctx, &x, argv[0]) || JS_ToUint32(ctx, &y, argv[1])) {
    return JS_EXCEPTION;
  }
  Vars.bIgnoreMouse = TRUE;
  Sleep(100);
  SendMouseClick(x, y, 0);
  Sleep(100);
  SendMouseClick(x, y, 1);
  Sleep(100);
  Vars.bIgnoreMouse = FALSE;
  return JS_NULL;
}

JSAPI_FUNC(my_sendKey) {
  uint32_t key;
  if (JS_ToUint32(ctx, &key, argv[0])) {
    return JS_EXCEPTION;
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
  return JS_NULL;
}