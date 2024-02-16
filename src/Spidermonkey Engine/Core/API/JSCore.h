#pragma once

#include "js32.h"

bool my_getThreadPriority(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_debugLog(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_stringToEUC(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_print(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_delay(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_load(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_include(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_isIncluded(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_stop(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_stacktrace(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_beep(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_copy(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_paste(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_sendCopyData(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_keystate(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_getTickCount(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_addEventListener(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_removeEventListener(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_clearEvent(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_clearAllEvents(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_js_strict(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_version(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_scriptBroadcast(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_showConsole(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_hideConsole(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_handler(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_loadMpq(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_getPacket(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_sendPacket(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_getIP(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_sendClick(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_sendKey(JSContext* ctx, unsigned argc, JS::Value* vp);
