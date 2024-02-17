#pragma once

#include "JSCore.h"
#include "JSGame.h"
#include "JSMenu.h"
#include "JSHash.h"

#include "JSFile.h"
#include "JSSocket.h"
#include "JSFileTools.h"
#include "JSDirectory.h"
#include "JSScreenHook.h"
#include "JSParty.h"
#include "JSArea.h"
#include "JSPresetUnit.h"
#include "JSUnit.h"
#include "JSScript.h"
#include "JSRoom.h"

#include "Trampoline.h"

static JSFunctionSpec global_funcs[] = {
    // "get" functions
    JS_FN("getPath", trampoline<my_getPath>, 0, 0),
    JS_FN("getCollision", trampoline<my_getCollision>, 0, 0),
    JS_FN("getCursorType", trampoline<my_getCursorType>, 0, 0),
    JS_FN("getSkillByName", trampoline<my_getSkillByName>, 1, 0),
    JS_FN("getSkillById", trampoline<my_getSkillById>, 1, 0),
    JS_FN("getLocaleString", trampoline<my_getLocaleString>, 1, 0),
    JS_FN("getTextSize", trampoline<my_getTextSize>, 2, 0),
    JS_FN("getThreadPriority", trampoline<my_getThreadPriority>, 0, 0),
    JS_FN("getUIFlag", trampoline<my_getUIFlag>, 1, 0),
    JS_FN("getTradeInfo", trampoline<my_getTradeInfo>, 0, 0),
    JS_FN("getWaypoint", trampoline<my_getWaypoint>, 1, 0),
    JS_FN("getBaseStat", trampoline<my_getBaseStat>, 0, 0),
    JS_FN("getPlayerFlag", trampoline<my_getPlayerFlag>, 2, 0),
    JS_FN("getTickCount", trampoline<my_getTickCount>, 0, 0),
    JS_FN("getInteractedNPC", trampoline<my_getInteractedNPC>, 0, 0),
    JS_FN("getIsTalkingNPC", trampoline<my_getIsTalkingNPC>, 0, 0),
    JS_FN("getDialogLines", trampoline<my_getDialogLines>, 0, 0),

    // utility functions that don't have anything to do with the game
    JS_FN("print", trampoline<my_print>, 1, 0),
    JS_FN("stringToEUC", trampoline<my_stringToEUC>, 1, 0),
    // JS_FN("utf8ToEuc", my_stringToEUC, 1, 0),
    JS_FN("delay", trampoline<my_delay>, 1, 0),
    JS_FN("load", trampoline<my_load>, 1, 0),
    JS_FN("isIncluded", trampoline<my_isIncluded>, 1, 0),
    JS_FN("include", trampoline<my_include>, 1, 0),
    JS_FN("stop", trampoline<my_stop>, 0, 0),
    JS_FN("stacktrace", trampoline<my_stacktrace>, 0, 0),
    JS_FN("rand", trampoline<my_rand>, 0, 0),
    JS_FN("copy", trampoline<my_copy>, 0, 0),
    JS_FN("paste", trampoline<my_paste>, 0, 0),
    JS_FN("sendCopyData", trampoline<my_sendCopyData>, 4, 0),
    JS_FN("keystate", trampoline<my_keystate>, 0, 0),
    JS_FN("addEventListener", trampoline<my_addEventListener>, 2, 0),
    JS_FN("removeEventListener", trampoline<my_removeEventListener>, 2, 0),
    JS_FN("clearEvent", trampoline<my_clearEvent>, 1, 0),
    JS_FN("clearAllEvents", trampoline<my_clearAllEvents>, 0, 0),
    JS_FN("js_strict", trampoline<my_js_strict>, 0, 0),
    JS_FN("version", trampoline<my_version>, 0, 0),
    JS_FN("scriptBroadcast", trampoline<my_scriptBroadcast>, 1, 0),
    JS_FN("debugLog", trampoline<my_debugLog>, 1, 0),
    JS_FN("showConsole", trampoline<my_showConsole>, 0, 0),
    JS_FN("hideConsole", trampoline<my_hideConsole>, 0, 0),
    JS_FN("handler", trampoline<my_handler>, 0, 0),

    // out of game functions
    JS_FN("login", trampoline<my_login>, 1, 0),
    // JS_FN("createCharacter", my_createCharacter, 4, 0),
    // this function is not finished
    JS_FN("selectCharacter", trampoline<my_selectChar>, 1, 0),
    JS_FN("createGame", trampoline<my_createGame>, 3, 0),
    JS_FN("joinGame", trampoline<my_joinGame>, 2, 0),
    JS_FN("addProfile", trampoline<my_addProfile>, 6, 0),
    JS_FN("getLocation", trampoline<my_getOOGLocation>, 0, 0),
    JS_FN("loadMpq", trampoline<my_loadMpq>, 1, 0),

    // game functions that don't have anything to do with gathering data
    JS_FN("submitItem", trampoline<my_submitItem>, 0, 0),
    JS_FN("getMouseCoords", trampoline<my_getMouseCoords>, 1, 0),
    JS_FN("copyUnit", trampoline<my_copyUnit>, 1, 0),
    JS_FN("clickMap", trampoline<my_clickMap>, 3, 0),
    JS_FN("acceptTrade", trampoline<my_acceptTrade>, 0, 0),
    JS_FN("tradeOk", trampoline<my_tradeOk>, 0, 0),
    JS_FN("beep", trampoline<my_beep>, 0, 0),
    JS_FN("clickItem", trampoline<my_clickItem>, 0, 0),
    JS_FN("getDistance", trampoline<my_getDistance>, 2, 0),
    JS_FN("gold", trampoline<my_gold>, 1, 0),
    JS_FN("checkCollision", trampoline<my_checkCollision>, 2, 0),
    JS_FN("playSound", trampoline<my_playSound>, 1, 0),
    JS_FN("quit", trampoline<my_quit>, 0, 0),
    JS_FN("quitGame", trampoline<my_quitGame>, 0, 0),
    JS_FN("say", trampoline<my_say>, 1, 0),
    JS_FN("clickParty", trampoline<my_clickParty>, 1, 0),
    JS_FN("weaponSwitch", trampoline<my_weaponSwitch>, 0, 0),
    JS_FN("transmute", trampoline<my_transmute>, 0, 0),
    JS_FN("useStatPoint", trampoline<my_useStatPoint>, 1, 0),
    JS_FN("useSkillPoint", trampoline<my_useSkillPoint>, 1, 0),
    JS_FN("takeScreenshot", trampoline<my_takeScreenshot>, 0, 0),
    JS_FN("moveNPC", trampoline<my_moveNPC>, 0, 0),
    JS_FN("getPacket", trampoline<my_getPacket>, 0, 0),
    JS_FN("sendPacket", trampoline<my_sendPacket>, 0, 0),
    JS_FN("getIP", trampoline<my_getIP>, 0, 0),
    JS_FN("sendClick", trampoline<my_sendClick>, 0, 0),
    JS_FN("sendKey", trampoline<my_sendKey>, 0, 0),
    JS_FN("revealLevel", trampoline<my_revealLevel>, 0, 0),

    // drawing functions
    JS_FN("screenToAutomap", trampoline<screenToAutomap>, 1, 0),
    JS_FN("automapToScreen", trampoline<automapToScreen>, 1, 0),

    // hash functions
    // JS_FN("md5", my_md5, 1, 0),
    // JS_FN("sha1", my_sha1, 1, 0),
    // JS_FN("sha256", my_sha256, 1, 0),
    // JS_FN("sha384", my_sha384, 1, 0),
    // JS_FN("sha512", my_sha512, 1, 0),
    // JS_FN("md5_file", my_md5_file, 1, 0),
    // JS_FN("sha1_file", my_sha1_file, 1, 0),
    // JS_FN("sha256_file", my_sha256_file, 1, 0),
    // JS_FN("sha384_file", my_sha384_file, 1, 0),
    // JS_FN("sha512_file", my_sha512_file, 1, 0),

    // JS_PROP_INT32_DEF("FILE_READ", FILE_READ, JS_PROP_CONFIGURABLE),
    // JS_PROP_INT32_DEF("FILE_WRITE", FILE_WRITE, JS_PROP_CONFIGURABLE),
    // JS_PROP_INT32_DEF("FILE_APPEND", FILE_APPEND, JS_PROP_CONFIGURABLE),

    JS_FS_END,
};
