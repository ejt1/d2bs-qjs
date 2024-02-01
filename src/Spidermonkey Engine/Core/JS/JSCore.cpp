#include "JSCore.h"

#include <sstream>

#include "D2Helpers.h"

#include <io.h>
#include <windows.h>
#include <ddeml.h>
#include <wininet.h>
#include <iostream>

// #include "js32.h"
// #include "Script.h"
#include "JSCore.h"
#include "Core.h"
#include "Helpers.h"
#include "dde.h"
// #include "ScriptEngine.h"
// #include "Engine.h"
#include "Events.h"
#include "Console.h"
#include "D2Ptrs.h"
#include "File.h"

#include "JSScript.h"

JSAPI_FUNC(my_stringToEUC) {
  if (argc == 0 || JS_IsNull(argv[0])) {
    return JS_NULL;
  }

  const char* szText = JS_ToCString(ctx, argv[0]);
  if (!szText) {
    return JS_EXCEPTION;
  }
  wchar_t* szwText = AnsiToUnicode(szText);
  JS_FreeCString(ctx, szText);

  if (szwText == NULL) {
    JS_ReportError(ctx, "Could not get string for value");
    return JS_EXCEPTION;
  }

  char* euc = UnicodeToAnsi(szwText, CP_ACP);
  JSValue rval = JS_NewString(ctx, euc);
  delete[] euc;
  delete[] szwText;
  return rval;
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
  wchar_t* wstr = AnsiToUnicode(finalstr.c_str());
  Log(wstr);
  delete[] wstr;
  return JS_UNDEFINED;
}

// TODO(ejt): setTimeout, setInterval and clearInterval is not currently used by kolbot (it overrides it).
// The plan is to use libuv timers once we get to adding libuv, for now just throw an error.
JSAPI_FUNC(my_setTimeout) {
  JS_ThrowInternalError(ctx, "setTimeout temporarily disabled during development");
  return JS_FALSE;

  // JS_SET_RVAL(cx, vp, JSVAL_NULL);

  // if (argc < 2 || !JSVAL_IS_NUMBER(JS_ARGV(cx, vp)[1]))
  //   JS_ReportError(cx, "invalid params passed to setTimeout");

  // if (JSVAL_IS_FUNCTION(cx, JS_ARGV(cx, vp)[0]) && JSVAL_IS_NUMBER(JS_ARGV(cx, vp)[1])) {
  //   Script* self = (Script*)JS_GetContextPrivate(cx);
  //   int freq = JSVAL_TO_INT(JS_ARGV(cx, vp)[1]);
  //   self->RegisterEvent("setTimeout", JS_ARGV(cx, vp)[0]);
  //   Event* evt = new Event;
  //   evt->owner = self;
  //   evt->name = "setTimeout";
  //   evt->arg3 = new jsval(JS_ARGV(cx, vp)[0]);
  //   JS_SET_RVAL(cx, vp, INT_TO_JSVAL(sScriptEngine->AddDelayedEvent(evt, freq)));
  // }

  // return JS_TRUE;
}

JSAPI_FUNC(my_setInterval) {
  JS_ThrowInternalError(ctx, "setInterval temporarily disabled during development");
  return JS_FALSE;

  // JS_SET_RVAL(cx, vp, JSVAL_NULL);

  // if (argc < 2 || !JSVAL_IS_NUMBER(JS_ARGV(cx, vp)[1]))
  //   JS_ReportError(cx, "invalid params passed to setInterval");

  // if (JSVAL_IS_FUNCTION(cx, JS_ARGV(cx, vp)[0]) && JSVAL_IS_NUMBER(JS_ARGV(cx, vp)[1])) {
  //   Script* self = (Script*)JS_GetContextPrivate(cx);
  //   int freq = JSVAL_TO_INT(JS_ARGV(cx, vp)[1]);
  //   self->RegisterEvent("setInterval", JS_ARGV(cx, vp)[0]);
  //   Event* evt = new Event;
  //   evt->owner = self;
  //   evt->name = "setInterval";
  //   evt->arg3 = new jsval(JS_ARGV(cx, vp)[0]);
  //   JS_SET_RVAL(cx, vp, INT_TO_JSVAL(sScriptEngine->AddDelayedEvent(evt, freq)));
  // }

  // return JS_TRUE;
}
JSAPI_FUNC(my_clearInterval) {
  JS_ThrowInternalError(ctx, "clearInterval temporarily disabled during development");
  return JS_FALSE;

  // JS_SET_RVAL(cx, vp, JSVAL_NULL);
  // if (argc != 1 || !JSVAL_IS_NUMBER(JS_ARGV(cx, vp)[0]))
  //   JS_ReportError(cx, "invalid params passed to clearInterval");

  // sScriptEngine->RemoveDelayedEvent(JSVAL_TO_INT(JS_ARGV(cx, vp)[0]));
  // return JS_TRUE;
}

JSAPI_FUNC(my_delay) {
  uint32 nDelay = 0;
  if (JS_ToUint32(ctx, &nDelay, argv[0])) {
    return JS_EXCEPTION;
  }

  Script* script = (Script*)JS_GetContextPrivate(ctx);

  if (nDelay) {  // loop so we can exec events while in delay
    script->BlockThread(nDelay);
  } else {
    JS_ReportError(ctx, "delay(0) called, argument must be >= 1");
  }
  return JS_TRUE;
}

JSAPI_FUNC(my_load) {
  Script* script = (Script*)JS_GetContextPrivate(ctx);
  if (!script) {
    JS_ReportError(ctx, "Failed to get script object");
    return JS_EXCEPTION;
  }

  const char* file = JS_ToCString(ctx, argv[0]);
  if (!file) {
    return JS_EXCEPTION;
  }

  if (strlen(file) > (_MAX_FNAME + _MAX_PATH - wcslen(Vars.szScriptPath))) {
    JS_FreeCString(ctx, file);
    JS_ReportError(ctx, "File name too long!");
    return JS_EXCEPTION;
  }

  ScriptMode scriptState = script->GetMode();
  if (scriptState == kScriptModeCommand)
    scriptState = (ClientState() == ClientStateInGame ? kScriptModeGame : kScriptModeMenu);

  wchar_t buf[_MAX_PATH + _MAX_FNAME];
  swprintf_s(buf, _countof(buf), L"%s\\%S", Vars.szScriptPath, file);
  StringReplace(buf, L'/', L'\\', _countof(buf));

  // JSAutoStructuredCloneBuffer** autoBuffer = new JSAutoStructuredCloneBuffer*[argc - 1];
  // for (uint i = 1; i < argc; i++) {
  //   autoBuffer[i - 1] = new JSAutoStructuredCloneBuffer;
  //   autoBuffer[i - 1]->write(cx, JS_ARGV(cx, vp)[i]);
  // }

  Script* newScript = sScriptEngine->NewScript(buf, scriptState /*, argc - 1, autoBuffer*/);

  if (newScript) {
    newScript->Start();
  } else {
    // TODO: Should this actually be there? No notification is bad, but do we want this? maybe throw an exception?
    Print(L"File \"%s\" not found.", file);
  }
  return JS_NULL;
}

JSAPI_FUNC(my_include) {
  Script* script = (Script*)JS_GetContextPrivate(ctx);
  if (!script) {
    Log(L"!script");
    JS_ReportError(ctx, "Failed to get script object");
    return JS_EXCEPTION;
  }

  const char* file = JS_ToCString(ctx, argv[0]);
  if (!file) {
    Log(L"!file");
    return JS_EXCEPTION;
  }

  if (strlen(file) > (_MAX_FNAME + _MAX_PATH - wcslen(Vars.szScriptPath) - 6)) {
    JS_FreeCString(ctx, file);
    JS_ReportError(ctx, "File name too long!");
    Log(L"strlen(file) > ");
    return JS_EXCEPTION;
  }

  wchar_t buf[_MAX_PATH + _MAX_FNAME];
  swprintf_s(buf, _countof(buf), L"%s\\libs\\%S", Vars.szScriptPath, file);
  JS_FreeCString(ctx, file);

  if (_waccess(buf, 0) == 0)
    return JS_NewBool(ctx, script->Include(buf));

  Log(L"waccess != 0");
  return JS_FALSE;
}

JSAPI_FUNC(my_stop) {
  int ival;
  if (argc > 0 && (JS_IsNumber(argv[0]) && (!JS_ToInt32(ctx, &ival, argv[0]) && ival == 1)) || (JS_IsBool(argv[0]) && JS_ToBool(ctx, argv[0]) == TRUE)) {
    Script* script = (Script*)JS_GetContextPrivate(ctx);
    if (script)
      script->Stop();
  } else
    sScriptEngine->StopAll();
  return JS_EXCEPTION;
}

JSAPI_FUNC(my_stacktrace) {
  GetStackWalk();
  return JS_TRUE;
}

JSAPI_FUNC(my_beep) {
  jsint nBeepId = NULL;

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

  if (strlen(file) > (_MAX_FNAME + _MAX_PATH - wcslen(Vars.szScriptPath) - 6)) {
    JS_FreeCString(ctx, file);
    JS_ReportError(ctx, "File name too long");
    return JS_EXCEPTION;
  }

  wchar_t path[_MAX_FNAME + _MAX_PATH];
  swprintf_s(path, _countof(path), L"%s\\libs\\%S", Vars.szScriptPath, file);
  JS_FreeCString(ctx, file);
  Script* script = (Script*)JS_GetContextPrivate(ctx);
  return JS_NewBool(ctx, script->IsIncluded(path));
}

JSAPI_FUNC(my_version) {
  if (argc < 1) {
    return JS_NewString(ctx, D2BS_VERSION);
  }

  Print(L"\u00FFc4D2BS\u00FFc1 \u00FFc3%s for Diablo II 1.14d.", D2BS_VERSION);
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
  std::wstring wstr(finalstr.begin(), finalstr.end());
  Log(wstr.c_str());
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
  return JS_TRUE;
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
  jsint nModeId = NULL;
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

JSAPI_FUNC(my_sendDDE) {
  // JS_SET_RVAL(cx, vp, JSVAL_FALSE);
  // int32_t mode = 0;
  // const char *pszDDEServer = "\"\"", *pszTopic = "\"\"", *pszItem = "\"\"", *pszData = "\"\"";
  // JS_BeginRequest(cx);

  // if (JSVAL_IS_INT(JS_ARGV(cx, vp)[0]))
  //   JS_ValueToECMAInt32(cx, JS_ARGV(cx, vp)[1], &mode);

  // if (JSVAL_IS_STRING(JS_ARGV(cx, vp)[1]))
  //   pszDDEServer = JS_EncodeStringToUTF8(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[1]));

  // if (JSVAL_IS_STRING(JS_ARGV(cx, vp)[2]))
  //   pszTopic = JS_EncodeStringToUTF8(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[2]));

  // if (JSVAL_IS_STRING(JS_ARGV(cx, vp)[3]))
  //   pszItem = JS_EncodeStringToUTF8(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[3]));

  // if (JSVAL_IS_STRING(JS_ARGV(cx, vp)[4]))
  //   pszData = JS_EncodeStringToUTF8(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[4]));

  // JS_EndRequest(cx);
  // char buffer[255] = "";
  // BOOL result = SendDDE(mode, pszDDEServer, pszTopic, pszItem, pszData, (char**)&buffer, sizeof(buffer));

  // if (JSVAL_IS_STRING(JS_ARGV(cx, vp)[1])) {
  //   JS_free(cx, (char*)pszDDEServer);
  // }
  // if (JSVAL_IS_STRING(JS_ARGV(cx, vp)[2])) {
  //   JS_free(cx, (char*)pszTopic);
  // }
  // if (JSVAL_IS_STRING(JS_ARGV(cx, vp)[3])) {
  //   JS_free(cx, (char*)pszItem);
  // }
  // if (JSVAL_IS_STRING(JS_ARGV(cx, vp)[4])) {
  //   JS_free(cx, (char*)pszData);
  // }

  // if (!result)
  //   THROW_ERROR(cx, "DDE Failed! Check the log for the error message.");

  // if (mode == 0) {
  //   wchar_t* buf = AnsiToUnicode(buffer);
  //   JS_SET_RVAL(cx, vp, STRING_TO_JSVAL(JS_NewUCStringCopyZ(cx, buf)));
  //   delete[] buf;
  // }
  return JS_TRUE;
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
      Script* self = (Script*)JS_GetContextPrivate(ctx);
      self->RegisterEvent(evtName, argv[1]);
    } else {
      THROW_ERROR(ctx, "Event name is invalid!");
    }
    JS_FreeCString(ctx, evtName);
  }
  return JS_TRUE;
}

JSAPI_FUNC(my_removeEventListener) {
  if (JS_IsString(argv[0]) && JS_IsFunction(ctx, argv[1])) {
    const char* evtName = JS_ToCString(ctx, argv[0]);
    if (evtName && strlen(evtName)) {
      Script* self = (Script*)JS_GetContextPrivate(ctx);
      self->UnregisterEvent(evtName, argv[1]);
    } else {
      THROW_ERROR(ctx, "Event name is invalid!");
    }
    JS_FreeCString(ctx, evtName);
  }
  return JS_TRUE;
}

JSAPI_FUNC(my_clearEvent) {
  if (JS_IsString(argv[0])) {
    Script* self = (Script*)JS_GetContextPrivate(ctx);
    const char* evt = JS_ToCString(ctx, argv[0]);
    self->ClearEvent(evt);
    JS_FreeCString(ctx, evt);
  }
  return JS_TRUE;
}

JSAPI_FUNC(my_clearAllEvents) {
  Script* self = (Script*)JS_GetContextPrivate(ctx);
  self->ClearAllEvents();
  return JS_TRUE;
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

  return JS_TRUE;
}

JSAPI_FUNC(my_scriptBroadcast) {
  if (argc < 1)
    THROW_ERROR(ctx, "You must specify something to broadcast");

  ScriptBroadcastEvent(ctx, argc, argv);
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
  const wchar_t* szwPath = AnsiToUnicode(path);
  JS_FreeCString(ctx, path);
  if (isValidPath(szwPath)) {
    LoadMPQ(szwPath);
  }
  delete[] szwPath;
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
  uint32 x = 0;
  uint32 y = 0;
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
  uint32 key;
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