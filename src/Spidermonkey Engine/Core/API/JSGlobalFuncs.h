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

static JSFunctionSpec global_funcs[] = {
    // "get" functions
    JS_FN("getPath", my_getPath, 0, 0),
    JS_FN("getCollision", my_getCollision, 0, 0),
    JS_FN("getCursorType", my_getCursorType, 0, 0),
    JS_FN("getSkillByName", my_getSkillByName, 1, 0),
    JS_FN("getSkillById", my_getSkillById, 1, 0),
    JS_FN("getLocaleString", my_getLocaleString, 1, 0),
    JS_FN("getTextSize", my_getTextSize, 2, 0),
    JS_FN("getThreadPriority", my_getThreadPriority, 0, 0),
    JS_FN("getUIFlag", my_getUIFlag, 1, 0),
    JS_FN("getTradeInfo", my_getTradeInfo, 0, 0),
    JS_FN("getWaypoint", my_getWaypoint, 1, 0),
    JS_FN("getBaseStat", my_getBaseStat, 0, 0),
    JS_FN("getPlayerFlag", my_getPlayerFlag, 2, 0),
    JS_FN("getTickCount", my_getTickCount, 0, 0),
    JS_FN("getInteractedNPC", my_getInteractedNPC, 0, 0),
    JS_FN("getIsTalkingNPC", my_getIsTalkingNPC, 0, 0),
    JS_FN("getDialogLines", my_getDialogLines, 0, 0),

    // utility functions that don't have anything to do with the game
    JS_FN("print", my_print, 1, 0),
    JS_FN("stringToEUC", my_stringToEUC, 1, 0),
    // JS_FN("utf8ToEuc", my_stringToEUC, 1, 0),
    JS_FN("delay", my_delay, 1, 0),
    JS_FN("load", my_load, 1, 0),
    JS_FN("isIncluded", my_isIncluded, 1, 0),
    JS_FN("include", my_include, 1, 0),
    JS_FN("stop", my_stop, 0, 0),
    JS_FN("stacktrace", my_stacktrace, 0, 0),
    JS_FN("rand", my_rand, 0, 0),
    JS_FN("copy", my_copy, 0, 0),
    JS_FN("paste", my_paste, 0, 0),
    JS_FN("sendCopyData", my_sendCopyData, 4, 0),
    JS_FN("keystate", my_keystate, 0, 0),
    JS_FN("addEventListener", my_addEventListener, 2, 0),
    JS_FN("removeEventListener", my_removeEventListener, 2, 0),
    JS_FN("clearEvent", my_clearEvent, 1, 0),
    JS_FN("clearAllEvents", my_clearAllEvents, 0, 0),
    JS_FN("js_strict", my_js_strict, 0, 0),
    JS_FN("version", my_version, 0, 0),
    JS_FN("scriptBroadcast", my_scriptBroadcast, 1, 0),
    JS_FN("debugLog", my_debugLog, 1, 0),
    JS_FN("showConsole", my_showConsole, 0, 0),
    JS_FN("hideConsole", my_hideConsole, 0, 0),
    JS_FN("handler", my_handler, 0, 0),

    // out of game functions
    JS_FN("login", my_login, 1, 0),
    // JS_FN("createCharacter", my_createCharacter, 4, 0),
    // this function is not finished
    JS_FN("selectCharacter", my_selectChar, 1, 0),
    JS_FN("createGame", my_createGame, 3, 0),
    JS_FN("joinGame", my_joinGame, 2, 0),
    JS_FN("addProfile", my_addProfile, 6, 0),
    JS_FN("getLocation", my_getOOGLocation, 0, 0),
    JS_FN("loadMpq", my_loadMpq, 1, 0),

    // game functions that don't have anything to do with gathering data
    JS_FN("submitItem", my_submitItem, 0, 0),
    JS_FN("getMouseCoords", my_getMouseCoords, 1, 0),
    JS_FN("copyUnit", my_copyUnit, 1, 0),
    JS_FN("clickMap", my_clickMap, 3, 0),
    JS_FN("acceptTrade", my_acceptTrade, 0, 0),
    JS_FN("tradeOk", my_tradeOk, 0, 0),
    JS_FN("beep", my_beep, 0, 0),
    JS_FN("clickItem", my_clickItem, 0, 0),
    JS_FN("getDistance", my_getDistance, 2, 0),
    JS_FN("gold", my_gold, 1, 0),
    JS_FN("checkCollision", my_checkCollision, 2, 0),
    JS_FN("playSound", my_playSound, 1, 0),
    JS_FN("quit", my_quit, 0, 0),
    JS_FN("quitGame", my_quitGame, 0, 0),
    JS_FN("say", my_say, 1, 0),
    JS_FN("clickParty", my_clickParty, 1, 0),
    JS_FN("weaponSwitch", my_weaponSwitch, 0, 0),
    JS_FN("transmute", my_transmute, 0, 0),
    JS_FN("useStatPoint", my_useStatPoint, 1, 0),
    JS_FN("useSkillPoint", my_useSkillPoint, 1, 0),
    JS_FN("takeScreenshot", my_takeScreenshot, 0, 0),
    JS_FN("moveNPC", my_moveNPC, 0, 0),
    JS_FN("getPacket", my_getPacket, 0, 0),
    JS_FN("sendPacket", my_sendPacket, 0, 0),
    JS_FN("getIP", my_getIP, 0, 0),
    JS_FN("sendClick", my_sendClick, 0, 0),
    JS_FN("sendKey", my_sendKey, 0, 0),
    JS_FN("revealLevel", my_revealLevel, 0, 0),

    // drawing functions
    JS_FN("screenToAutomap", screenToAutomap, 1, 0),
    JS_FN("automapToScreen", automapToScreen, 1, 0),

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
