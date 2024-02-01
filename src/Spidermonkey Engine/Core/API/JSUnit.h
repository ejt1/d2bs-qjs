#pragma once

#include <windows.h>
#include "js32.h"
#include "Offset.h"

#define PRIVATE_UNIT 1
#define PRIVATE_ITEM 3

struct Private {
  DWORD dwPrivateType;
};

CLASS_CTOR(unit);

JSAPI_FUNC(unit_getUnit);
JSAPI_FUNC(unit_getNext);
JSAPI_FUNC(unit_cancel);
JSAPI_FUNC(unit_repair);
JSAPI_FUNC(unit_useMenu);
JSAPI_FUNC(unit_interact);
JSAPI_FUNC(unit_getStat);
JSAPI_FUNC(unit_getState);
JSAPI_FUNC(unit_getItems);
JSAPI_FUNC(unit_getSkill);
JSAPI_FUNC(unit_getParent);
JSAPI_FUNC(unit_setskill);
JSAPI_FUNC(unit_getMerc);
JSAPI_FUNC(unit_getMercHP);
JSAPI_FUNC(unit_getItem);
JSAPI_FUNC(unit_move);
JSAPI_FUNC(item_getFlag);
JSAPI_FUNC(item_getFlags);
// JSAPI_FUNC(item_getPrice);
JSAPI_FUNC(item_shop);
JSAPI_FUNC(my_overhead);
JSAPI_FUNC(my_revive);
JSAPI_FUNC(unit_getEnchant);
JSAPI_FUNC(unit_getQuest);
JSAPI_FUNC(unit_getMinionCount);
JSAPI_FUNC(me_getRepairCost);
JSAPI_FUNC(item_getItemCost);

void unit_finalize(JSFreeOp* fop, JSObject* obj);

JSAPI_PROP(unit_getProperty);
JSAPI_STRICT_PROP(unit_setProperty);

JSBool unit_equal(JSContext* cx, JSObject* obj, jsval v, JSBool* bp);

struct myUnit {
  DWORD _dwPrivateType;
  DWORD dwUnitId;
  DWORD dwClassId;
  DWORD dwType;
  DWORD dwMode;
  char szName[128];
};

struct invUnit {
  DWORD _dwPrivateType;
  DWORD dwUnitId;
  DWORD dwClassId;
  DWORD dwType;
  DWORD dwMode;
  char szName[128];
  DWORD dwOwnerId;
  DWORD dwOwnerType;
};

enum unit_tinyid {
  UNIT_TYPE,
  UNIT_CLASSID,
  UNIT_ID,
  UNIT_MODE,
  UNIT_NAME,
  UNIT_BUSY,
  UNIT_ACT,
  UNIT_XPOS,
  UNIT_YPOS,
  ME_WSWITCH,
  UNIT_AREA,
  UNIT_HP,
  UNIT_HPMAX,
  UNIT_MP,
  UNIT_MPMAX,
  UNIT_STAMINA,
  UNIT_STAMINAMAX,
  UNIT_CHARLVL,
  ME_RUNWALK,
  ITEM_CODE,
  ITEM_PREFIX,
  ITEM_SUFFIX,
  ITEM_FNAME,
  ITEM_QUALITY,
  ITEM_NODE,
  UNIT_SELECTABLE,
  ITEM_LOC,
  ITEM_SIZEX,
  ITEM_SIZEY,
  ITEM_TYPE,
  MISSILE_DIR,
  MISSILE_VEL,
  ITEM_CLASS,
  UNIT_SPECTYPE,
  ITEM_DESC,
  ITEM_BODYLOCATION,
  UNIT_ITEMCOUNT,
  ITEM_LEVELREQ,
  UNIT_OWNER,
  UNIT_OWNERTYPE,
  UNIT_UNIQUEID,
  ITEM_LEVEL,
  UNIT_DIRECTION,
  ITEM_SUFFIXNUM,
  ITEM_PREFIXNUM,
  ITEM_PREFIXES,
  ITEM_SUFFIXES,
  ITEM_SUFFIXNUMS,
  ITEM_PREFIXNUMS,
  ITEM_GFX,
  OBJECT_TYPE,
  OBJECT_LOCKED,
  UNIT_TARGETX,
  UNIT_TARGETY
};

enum me_tinyid {
  ME_ACCOUNT = 90,
  ME_CHARNAME,
  ME_CHICKENHP,
  ME_CHICKENMP,
  ME_DIFF,
  ME_MAXDIFF,
  ME_GAMENAME,
  ME_GAMEPASSWORD,
  ME_GAMESERVERIP,
  ME_GAMESTARTTIME,
  ME_GAMETYPE,
  ME_ITEMONCURSOR,
  ME_AUTOMAP,
  ME_LADDER,
  ME_PING,
  ME_FPS,
  ME_LOCALE,
  ME_PLAYERTYPE,
  ME_QUITONHOSTILE,
  ME_REALM,
  ME_REALMSHORT,
  ME_MERCREVIVECOST,
  ME_MAPID,
  OOG_WINDOWTITLE,
  OOG_SCREENSIZE,
  OOG_INGAME,
  OOG_QUITONERROR,
  OOG_MAXGAMETIME,
  ME_BLOCKKEYS,
  ME_BLOCKMOUSE,
  ME_GAMEREADY,
  ME_PROFILE,
  ME_NOPICKUP,
  ME_PID,
  ME_UNSUPPORTED,
  ME_CHARFLAGS
};

static JSPropertySpec me_props[] = {{"account", ME_ACCOUNT, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"charname", ME_CHARNAME, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"diff", ME_DIFF, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"maxdiff", ME_MAXDIFF, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"gamename", ME_GAMENAME, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"gamepassword", ME_GAMEPASSWORD, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"gameserverip", ME_GAMESERVERIP, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"gamestarttime", ME_GAMESTARTTIME, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"gametype", ME_GAMETYPE, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"itemoncursor", ME_ITEMONCURSOR, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"automap", ME_AUTOMAP, JSPROP_STATIC_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_WRAPPER(unit_setProperty)},
                                    {"ladder", ME_LADDER, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"ping", ME_PING, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"fps", ME_FPS, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"locale", ME_LOCALE, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"playertype", ME_PLAYERTYPE, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"realm", ME_REALM, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"realmshort", ME_REALMSHORT, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"mercrevivecost", ME_MERCREVIVECOST, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"runwalk", ME_RUNWALK, JSPROP_STATIC_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_WRAPPER(unit_setProperty)},
                                    {"weaponswitch", ME_WSWITCH, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"chickenhp", ME_CHICKENHP, JSPROP_STATIC_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_WRAPPER(unit_setProperty)},
                                    {"chickenmp", ME_CHICKENMP, JSPROP_STATIC_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_WRAPPER(unit_setProperty)},
                                    {"quitonhostile", ME_QUITONHOSTILE, JSPROP_STATIC_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_WRAPPER(unit_setProperty)},
                                    {"blockKeys", ME_BLOCKKEYS, JSPROP_STATIC_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_WRAPPER(unit_setProperty)},
                                    {"blockMouse", ME_BLOCKMOUSE, JSPROP_STATIC_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_WRAPPER(unit_setProperty)},
                                    {"gameReady", ME_GAMEREADY, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"profile", ME_PROFILE, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"nopickup", ME_NOPICKUP, JSPROP_STATIC_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_WRAPPER(unit_setProperty)},
                                    {"pid", ME_PID, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"unsupported", ME_UNSUPPORTED, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"charflags", ME_CHARFLAGS, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},

                                    {"screensize", OOG_SCREENSIZE, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"windowtitle", OOG_WINDOWTITLE, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"ingame", OOG_INGAME, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"quitonerror", OOG_QUITONERROR, JSPROP_STATIC_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_WRAPPER(unit_setProperty)},
                                    {"maxgametime", OOG_MAXGAMETIME, JSPROP_STATIC_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_WRAPPER(unit_setProperty)},

                                    {"type", UNIT_TYPE, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"classid", UNIT_CLASSID, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"mode", UNIT_MODE, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"name", UNIT_NAME, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"mapid", ME_MAPID, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"act", UNIT_ACT, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"gid", UNIT_ID, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"x", UNIT_XPOS, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"y", UNIT_YPOS, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"targetx", UNIT_TARGETX, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"targety", UNIT_TARGETY, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"area", UNIT_AREA, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"hp", UNIT_HP, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"hpmax", UNIT_HPMAX, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"mp", UNIT_MP, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"mpmax", UNIT_MPMAX, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"stamina", UNIT_STAMINA, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"staminamax", UNIT_STAMINAMAX, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"charlvl", UNIT_CHARLVL, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"itemcount", UNIT_ITEMCOUNT, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"owner", UNIT_OWNER, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"ownertype", UNIT_OWNERTYPE, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"spectype", UNIT_SPECTYPE, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"direction", UNIT_DIRECTION, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},

                                    {"code", ITEM_CODE, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"prefix", ITEM_PREFIX, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"suffix", ITEM_SUFFIX, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"prefixes", ITEM_PREFIXES, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"suffixes", ITEM_SUFFIXES, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"prefixnum", ITEM_PREFIXNUM, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"suffixnum", ITEM_SUFFIXNUM, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"prefixnums", ITEM_PREFIXNUMS, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"suffixnums", ITEM_SUFFIXNUMS, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"fname", ITEM_FNAME, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"quality", ITEM_QUALITY, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"node", ITEM_NODE, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"location", ITEM_LOC, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"sizex", ITEM_SIZEX, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"sizey", ITEM_SIZEY, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"itemType", ITEM_TYPE, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"description", ITEM_DESC, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"bodylocation", ITEM_BODYLOCATION, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {"ilvl", ITEM_LEVEL, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}};

static JSPropertySpec unit_props[] = {{"type", UNIT_TYPE, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"classid", UNIT_CLASSID, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"mode", UNIT_MODE, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"name", UNIT_NAME, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"act", UNIT_ACT, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"gid", UNIT_ID, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"x", UNIT_XPOS, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"y", UNIT_YPOS, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"targetx", UNIT_TARGETX, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"targety", UNIT_TARGETY, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"area", UNIT_AREA, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"hp", UNIT_HP, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"hpmax", UNIT_HPMAX, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"mp", UNIT_MP, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"mpmax", UNIT_MPMAX, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"stamina", UNIT_STAMINA, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"staminamax", UNIT_STAMINAMAX, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"charlvl", UNIT_CHARLVL, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"itemcount", UNIT_ITEMCOUNT, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"owner", UNIT_OWNER, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"ownertype", UNIT_OWNERTYPE, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"spectype", UNIT_SPECTYPE, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"direction", UNIT_DIRECTION, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"uniqueid", UNIT_UNIQUEID, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},

                                      {"code", ITEM_CODE, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"prefix", ITEM_PREFIX, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"suffix", ITEM_SUFFIX, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"prefixes", ITEM_PREFIXES, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"suffixes", ITEM_SUFFIXES, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"prefixnum", ITEM_PREFIXNUM, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"suffixnum", ITEM_SUFFIXNUM, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"prefixnums", ITEM_PREFIXNUMS, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"suffixnums", ITEM_SUFFIXNUMS, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"fname", ITEM_FNAME, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"quality", ITEM_QUALITY, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"node", ITEM_NODE, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"location", ITEM_LOC, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"sizex", ITEM_SIZEX, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"sizey", ITEM_SIZEY, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"itemType", ITEM_TYPE, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"description", ITEM_DESC, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"bodylocation", ITEM_BODYLOCATION, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"ilvl", ITEM_LEVEL, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"lvlreq", ITEM_LEVELREQ, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"gfx", ITEM_GFX, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},

                                      {"runwalk", ME_RUNWALK, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"weaponswitch", ME_WSWITCH, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},

                                      {"objtype", OBJECT_TYPE, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {"islocked", OBJECT_LOCKED, JSPROP_PERMANENT_VAR, JSOP_WRAPPER(unit_getProperty), JSOP_NULLWRAPPER},
                                      {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}};

static JSFunctionSpec unit_methods[] = {
    JS_FS("getNext", unit_getNext, 0, FUNCTION_FLAGS), JS_FS("cancel", unit_cancel, 0, FUNCTION_FLAGS), JS_FS("repair", unit_repair, 0, FUNCTION_FLAGS),
    JS_FS("useMenu", unit_useMenu, 0, FUNCTION_FLAGS), JS_FS("interact", unit_interact, 0, FUNCTION_FLAGS), JS_FS("getItem", unit_getItem, 3, FUNCTION_FLAGS),
    JS_FS("getItems", unit_getItems, 0, FUNCTION_FLAGS), JS_FS("getMerc", unit_getMerc, 0, FUNCTION_FLAGS), JS_FS("getMercHP", unit_getMercHP, 0, FUNCTION_FLAGS),
    JS_FS("getSkill", unit_getSkill, 0, FUNCTION_FLAGS), JS_FS("getParent", unit_getParent, 0, FUNCTION_FLAGS), JS_FS("overhead", my_overhead, 0, FUNCTION_FLAGS),
    JS_FS("revive", my_revive, 0, FUNCTION_FLAGS), JS_FS("getFlags", item_getFlags, 1, FUNCTION_FLAGS), JS_FS("getFlag", item_getFlag, 1, FUNCTION_FLAGS),
    JS_FS("getStat", unit_getStat, 1, FUNCTION_FLAGS), JS_FS("getState", unit_getState, 1, FUNCTION_FLAGS),
    //	{"geSPrice",		item_getPrice,		1},
    JS_FS("getEnchant", unit_getEnchant, 1, FUNCTION_FLAGS), JS_FS("shop", item_shop, 2, FUNCTION_FLAGS), JS_FS("setSkill", unit_setskill, 2, FUNCTION_FLAGS),
    JS_FS("move", unit_move, 2, FUNCTION_FLAGS), JS_FS("getQuest", unit_getQuest, 2, FUNCTION_FLAGS), JS_FS("getMinionCount", unit_getMinionCount, 1, FUNCTION_FLAGS),
    JS_FS("getRepairCost", me_getRepairCost, 1, FUNCTION_FLAGS), JS_FS("getItemCost", item_getItemCost, 1, FUNCTION_FLAGS), JS_FS_END};
