#pragma once

#include "js32.h"

bool my_getThreadPriority(JSContext* ctx, JS::CallArgs& args);
bool my_debugLog(JSContext* ctx, JS::CallArgs& args);
bool my_stringToEUC(JSContext* ctx, JS::CallArgs& args);
bool my_print(JSContext* ctx, JS::CallArgs& args);
bool my_delay(JSContext* ctx, JS::CallArgs& args);
bool my_load(JSContext* ctx, JS::CallArgs& args);
bool my_include(JSContext* ctx, JS::CallArgs& args);
bool my_isIncluded(JSContext* ctx, JS::CallArgs& args);
bool my_stop(JSContext* ctx, JS::CallArgs& args);
bool my_stacktrace(JSContext* ctx, JS::CallArgs& args);
bool my_beep(JSContext* ctx, JS::CallArgs& args);
bool my_copy(JSContext* ctx, JS::CallArgs& args);
bool my_paste(JSContext* ctx, JS::CallArgs& args);
bool my_sendCopyData(JSContext* ctx, JS::CallArgs& args);
bool my_keystate(JSContext* ctx, JS::CallArgs& args);
bool my_getTickCount(JSContext* ctx, JS::CallArgs& args);
bool my_addEventListener(JSContext* ctx, JS::CallArgs& args);
bool my_removeEventListener(JSContext* ctx, JS::CallArgs& args);
bool my_clearEvent(JSContext* ctx, JS::CallArgs& args);
bool my_clearAllEvents(JSContext* ctx, JS::CallArgs& args);
bool my_js_strict(JSContext* ctx, JS::CallArgs& args);
bool my_version(JSContext* ctx, JS::CallArgs& args);
bool my_scriptBroadcast(JSContext* ctx, JS::CallArgs& args);
bool my_showConsole(JSContext* ctx, JS::CallArgs& args);
bool my_hideConsole(JSContext* ctx, JS::CallArgs& args);
bool my_handler(JSContext* ctx, JS::CallArgs& args);
bool my_loadMpq(JSContext* ctx, JS::CallArgs& args);
bool my_getPacket(JSContext* ctx, JS::CallArgs& args);
bool my_sendPacket(JSContext* ctx, JS::CallArgs& args);
bool my_getIP(JSContext* ctx, JS::CallArgs& args);
bool my_sendClick(JSContext* ctx, JS::CallArgs& args);
bool my_sendKey(JSContext* ctx, JS::CallArgs& args);
