#pragma once

#include "js32.h"

bool my_rand(JSContext* ctx, JS::CallArgs& args);
bool my_submitItem(JSContext* ctx, JS::CallArgs& args);
bool my_copyUnit(JSContext* ctx, JS::CallArgs& args);
bool my_clickMap(JSContext* ctx, JS::CallArgs& args);
bool my_acceptTrade(JSContext* ctx, JS::CallArgs& args);
bool my_tradeOk(JSContext* ctx, JS::CallArgs& args);
bool my_getDialogLines(JSContext* ctx, JS::CallArgs& args);
bool my_clickDialog(JSContext* ctx, JS::CallArgs& args);
bool my_clickItem(JSContext* ctx, JS::CallArgs& args);
bool my_gold(JSContext* ctx, JS::CallArgs& args);
bool my_checkCollision(JSContext* ctx, JS::CallArgs& args);
bool my_playSound(JSContext* ctx, JS::CallArgs& args);
bool my_quit(JSContext* ctx, JS::CallArgs& args);
bool my_quitGame(JSContext* ctx, JS::CallArgs& args);
bool my_say(JSContext* ctx, JS::CallArgs& args);
bool my_weaponSwitch(JSContext* ctx, JS::CallArgs& args);
bool my_transmute(JSContext* ctx, JS::CallArgs& args);
bool my_clickParty(JSContext* ctx, JS::CallArgs& args);
bool my_useStatPoint(JSContext* ctx, JS::CallArgs& args);
bool my_useSkillPoint(JSContext* ctx, JS::CallArgs& args);

bool my_getInteractedNPC(JSContext* ctx, JS::CallArgs& args);
bool my_getIsTalkingNPC(JSContext* ctx, JS::CallArgs& args);

bool my_takeScreenshot(JSContext* ctx, JS::CallArgs& args);
bool my_getMouseCoords(JSContext* ctx, JS::CallArgs& args);
bool my_getDistance(JSContext* ctx, JS::CallArgs& args);
bool my_getPath(JSContext* ctx, JS::CallArgs& args);
bool my_getCollision(JSContext* ctx, JS::CallArgs& args);
bool my_getCursorType(JSContext* ctx, JS::CallArgs& args);
bool my_getSkillByName(JSContext* ctx, JS::CallArgs& args);
bool my_getSkillById(JSContext* ctx, JS::CallArgs& args);
bool my_getLocaleString(JSContext* ctx, JS::CallArgs& args);
bool my_getTextSize(JSContext* ctx, JS::CallArgs& args);
bool my_getUIFlag(JSContext* ctx, JS::CallArgs& args);
bool my_getTradeInfo(JSContext* ctx, JS::CallArgs& args);
bool my_getWaypoint(JSContext* ctx, JS::CallArgs& args);
bool my_getBaseStat(JSContext* ctx, JS::CallArgs& args);
bool my_getPlayerFlag(JSContext* ctx, JS::CallArgs& args);
bool my_moveNPC(JSContext* ctx, JS::CallArgs& args);
bool my_revealLevel(JSContext* ctx, JS::CallArgs& args);

bool screenToAutomap(JSContext* ctx, JS::CallArgs& args);
bool automapToScreen(JSContext* ctx, JS::CallArgs& args);
