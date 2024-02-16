#pragma once

#include "js32.h"

bool my_rand(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_submitItem(JSContext * ctx, unsigned argc, JS::Value* vp);
bool my_copyUnit(JSContext * ctx, unsigned argc, JS::Value* vp);
bool my_clickMap(JSContext * ctx, unsigned argc, JS::Value* vp);
bool my_acceptTrade(JSContext * ctx, unsigned argc, JS::Value* vp);
bool my_tradeOk(JSContext * ctx, unsigned argc, JS::Value* vp);
bool my_getDialogLines(JSContext * ctx, unsigned argc, JS::Value* vp);
bool my_clickDialog(JSContext * ctx, unsigned argc, JS::Value* vp);
bool my_clickItem(JSContext * ctx, unsigned argc, JS::Value* vp);
bool my_gold(JSContext * ctx, unsigned argc, JS::Value* vp);
bool my_checkCollision(JSContext * ctx, unsigned argc, JS::Value* vp);
bool my_playSound(JSContext * ctx, unsigned argc, JS::Value* vp);
bool my_quit(JSContext * ctx, unsigned argc, JS::Value* vp);
bool my_quitGame(JSContext * ctx, unsigned argc, JS::Value* vp);
bool my_say(JSContext * ctx, unsigned argc, JS::Value* vp);
bool my_weaponSwitch(JSContext * ctx, unsigned argc, JS::Value* vp);
bool my_transmute(JSContext * ctx, unsigned argc, JS::Value* vp);
bool my_clickParty(JSContext * ctx, unsigned argc, JS::Value* vp);
bool my_useStatPoint(JSContext * ctx, unsigned argc, JS::Value* vp);
bool my_useSkillPoint(JSContext * ctx, unsigned argc, JS::Value* vp);

bool my_getInteractedNPC(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_getIsTalkingNPC(JSContext* ctx, unsigned argc, JS::Value* vp);

bool my_takeScreenshot(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_getMouseCoords(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_getDistance(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_getPath(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_getCollision(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_getCursorType(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_getSkillByName(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_getSkillById(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_getLocaleString(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_getTextSize(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_getUIFlag(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_getTradeInfo(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_getWaypoint(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_getBaseStat(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_getPlayerFlag(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_moveNPC(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_revealLevel(JSContext* ctx, unsigned argc, JS::Value* vp);

bool screenToAutomap(JSContext* ctx, unsigned argc, JS::Value* vp);
bool automapToScreen(JSContext* ctx, unsigned argc, JS::Value* vp);
