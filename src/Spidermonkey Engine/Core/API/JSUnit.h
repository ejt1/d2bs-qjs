#pragma once

#include <windows.h>
#include "js32.h"
#include "Offset.h"

CLASS_CTOR(unit);
CLASS_FINALIZER(unit);

JSAPI_PROP(unit_getProperty);
JSAPI_STRICT_PROP(unit_setProperty);

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

#define PRIVATE_UNIT 1
#define PRIVATE_ITEM 3

struct JSUnit {
  DWORD dwPrivateType;
  DWORD dwUnitId;
  DWORD dwClassId;
  DWORD dwType;
  DWORD dwMode;
  char szName[128];
};

struct JSItem : JSUnit {
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

static JSCFunctionListEntry me_proto_funcs[] = {
    JS_CGETSET_MAGIC_DEF("account", unit_getProperty, nullptr, ME_ACCOUNT),
    JS_CGETSET_MAGIC_DEF("charname", unit_getProperty, nullptr, ME_CHARNAME),
    JS_CGETSET_MAGIC_DEF("diff", unit_getProperty, nullptr, ME_DIFF),
    JS_CGETSET_MAGIC_DEF("maxdiff", unit_getProperty, nullptr, ME_MAXDIFF),
    JS_CGETSET_MAGIC_DEF("gamename", unit_getProperty, nullptr, ME_GAMENAME),
    JS_CGETSET_MAGIC_DEF("gamepassword", unit_getProperty, nullptr, ME_GAMEPASSWORD),
    JS_CGETSET_MAGIC_DEF("gameserverip", unit_getProperty, nullptr, ME_GAMESERVERIP),
    JS_CGETSET_MAGIC_DEF("gamestarttime", unit_getProperty, nullptr, ME_GAMESTARTTIME),
    JS_CGETSET_MAGIC_DEF("gametype", unit_getProperty, nullptr, ME_GAMETYPE),
    JS_CGETSET_MAGIC_DEF("itemoncursor", unit_getProperty, nullptr, ME_ITEMONCURSOR),
    JS_CGETSET_MAGIC_DEF("automap", unit_getProperty, unit_setProperty, ME_AUTOMAP),
    JS_CGETSET_MAGIC_DEF("ladder", unit_getProperty, nullptr, ME_LADDER),
    JS_CGETSET_MAGIC_DEF("ping", unit_getProperty, nullptr, ME_PING),
    JS_CGETSET_MAGIC_DEF("fps", unit_getProperty, nullptr, ME_FPS),
    JS_CGETSET_MAGIC_DEF("locale", unit_getProperty, nullptr, ME_LOCALE),
    JS_CGETSET_MAGIC_DEF("playertype", unit_getProperty, nullptr, ME_PLAYERTYPE),
    JS_CGETSET_MAGIC_DEF("realm", unit_getProperty, nullptr, ME_REALM),
    JS_CGETSET_MAGIC_DEF("realmshort", unit_getProperty, nullptr, ME_REALMSHORT),
    JS_CGETSET_MAGIC_DEF("mercrevivecost", unit_getProperty, nullptr, ME_MERCREVIVECOST),
    JS_CGETSET_MAGIC_DEF("runwalk", unit_getProperty, unit_setProperty, ME_RUNWALK),
    JS_CGETSET_MAGIC_DEF("weaponswitch", unit_getProperty, nullptr, ME_WSWITCH),
    JS_CGETSET_MAGIC_DEF("chickenhp", unit_getProperty, unit_setProperty, ME_CHICKENHP),
    JS_CGETSET_MAGIC_DEF("chickenmp", unit_getProperty, unit_setProperty, ME_CHICKENMP),
    JS_CGETSET_MAGIC_DEF("quitonhostile", unit_getProperty, unit_setProperty, ME_QUITONHOSTILE),
    JS_CGETSET_MAGIC_DEF("blockKeys", unit_getProperty, unit_setProperty, ME_BLOCKKEYS),
    JS_CGETSET_MAGIC_DEF("blockMouse", unit_getProperty, unit_setProperty, ME_BLOCKMOUSE),
    JS_CGETSET_MAGIC_DEF("gameReady", unit_getProperty, nullptr, ME_GAMEREADY),
    JS_CGETSET_MAGIC_DEF("profile", unit_getProperty, nullptr, ME_PROFILE),
    JS_CGETSET_MAGIC_DEF("nopickup", unit_getProperty, unit_setProperty, ME_NOPICKUP),
    JS_CGETSET_MAGIC_DEF("pid", unit_getProperty, nullptr, ME_PID),
    JS_CGETSET_MAGIC_DEF("unsupported", unit_getProperty, nullptr, ME_UNSUPPORTED),
    JS_CGETSET_MAGIC_DEF("charflags", unit_getProperty, nullptr, ME_CHARFLAGS),
    JS_CGETSET_MAGIC_DEF("screensize", unit_getProperty, nullptr, OOG_SCREENSIZE),
    JS_CGETSET_MAGIC_DEF("windowtitle", unit_getProperty, nullptr, OOG_WINDOWTITLE),
    JS_CGETSET_MAGIC_DEF("ingame", unit_getProperty, nullptr, OOG_INGAME),
    JS_CGETSET_MAGIC_DEF("quitonerror", unit_getProperty, unit_setProperty, OOG_QUITONERROR),
    JS_CGETSET_MAGIC_DEF("maxgametime", unit_getProperty, unit_setProperty, OOG_MAXGAMETIME),
    JS_CGETSET_MAGIC_DEF("type", unit_getProperty, nullptr, UNIT_TYPE),
    JS_CGETSET_MAGIC_DEF("classid", unit_getProperty, nullptr, UNIT_CLASSID),
    JS_CGETSET_MAGIC_DEF("mode", unit_getProperty, nullptr, UNIT_MODE),
    JS_CGETSET_MAGIC_DEF("name", unit_getProperty, nullptr, UNIT_NAME),
    JS_CGETSET_MAGIC_DEF("mapid", unit_getProperty, nullptr, ME_MAPID),
    JS_CGETSET_MAGIC_DEF("act", unit_getProperty, nullptr, UNIT_ACT),
    JS_CGETSET_MAGIC_DEF("gid", unit_getProperty, nullptr, UNIT_ID),
    JS_CGETSET_MAGIC_DEF("x", unit_getProperty, nullptr, UNIT_XPOS),
    JS_CGETSET_MAGIC_DEF("y", unit_getProperty, nullptr, UNIT_YPOS),
    JS_CGETSET_MAGIC_DEF("targetx", unit_getProperty, nullptr, UNIT_TARGETX),
    JS_CGETSET_MAGIC_DEF("targety", unit_getProperty, nullptr, UNIT_TARGETY),
    JS_CGETSET_MAGIC_DEF("area", unit_getProperty, nullptr, UNIT_AREA),
    JS_CGETSET_MAGIC_DEF("hp", unit_getProperty, nullptr, UNIT_HP),
    JS_CGETSET_MAGIC_DEF("hpmax", unit_getProperty, nullptr, UNIT_HPMAX),
    JS_CGETSET_MAGIC_DEF("mp", unit_getProperty, nullptr, UNIT_MP),
    JS_CGETSET_MAGIC_DEF("mpmax", unit_getProperty, nullptr, UNIT_MPMAX),
    JS_CGETSET_MAGIC_DEF("stamina", unit_getProperty, nullptr, UNIT_STAMINA),
    JS_CGETSET_MAGIC_DEF("staminamax", unit_getProperty, nullptr, UNIT_STAMINAMAX),
    JS_CGETSET_MAGIC_DEF("charlvl", unit_getProperty, nullptr, UNIT_CHARLVL),
    JS_CGETSET_MAGIC_DEF("itemcount", unit_getProperty, nullptr, UNIT_ITEMCOUNT),
    JS_CGETSET_MAGIC_DEF("owner", unit_getProperty, nullptr, UNIT_OWNER),
    JS_CGETSET_MAGIC_DEF("ownertype", unit_getProperty, nullptr, UNIT_OWNERTYPE),
    JS_CGETSET_MAGIC_DEF("spectype", unit_getProperty, nullptr, UNIT_SPECTYPE),
    JS_CGETSET_MAGIC_DEF("direction", unit_getProperty, nullptr, UNIT_DIRECTION),
    JS_CGETSET_MAGIC_DEF("code", unit_getProperty, nullptr, ITEM_CODE),
    JS_CGETSET_MAGIC_DEF("prefix", unit_getProperty, nullptr, ITEM_PREFIX),
    JS_CGETSET_MAGIC_DEF("suffix", unit_getProperty, nullptr, ITEM_SUFFIX),
    JS_CGETSET_MAGIC_DEF("prefixes", unit_getProperty, nullptr, ITEM_PREFIXES),
    JS_CGETSET_MAGIC_DEF("suffixes", unit_getProperty, nullptr, ITEM_SUFFIXES),
    JS_CGETSET_MAGIC_DEF("prefixnum", unit_getProperty, nullptr, ITEM_PREFIXNUM),
    JS_CGETSET_MAGIC_DEF("suffixnum", unit_getProperty, nullptr, ITEM_SUFFIXNUM),
    JS_CGETSET_MAGIC_DEF("prefixnums", unit_getProperty, nullptr, ITEM_PREFIXNUMS),
    JS_CGETSET_MAGIC_DEF("suffixnums", unit_getProperty, nullptr, ITEM_SUFFIXNUMS),
    JS_CGETSET_MAGIC_DEF("fname", unit_getProperty, nullptr, ITEM_FNAME),
    JS_CGETSET_MAGIC_DEF("quality", unit_getProperty, nullptr, ITEM_QUALITY),
    JS_CGETSET_MAGIC_DEF("node", unit_getProperty, nullptr, ITEM_NODE),
    JS_CGETSET_MAGIC_DEF("location", unit_getProperty, nullptr, ITEM_LOC),
    JS_CGETSET_MAGIC_DEF("sizex", unit_getProperty, nullptr, ITEM_SIZEX),
    JS_CGETSET_MAGIC_DEF("sizey", unit_getProperty, nullptr, ITEM_SIZEY),
    JS_CGETSET_MAGIC_DEF("itemType", unit_getProperty, nullptr, ITEM_TYPE),
    JS_CGETSET_MAGIC_DEF("description", unit_getProperty, nullptr, ITEM_DESC),
    JS_CGETSET_MAGIC_DEF("bodylocation", unit_getProperty, nullptr, ITEM_BODYLOCATION),
    JS_CGETSET_MAGIC_DEF("ilvl", unit_getProperty, nullptr, ITEM_LEVEL),

    JS_FS("getNext", unit_getNext, 0, FUNCTION_FLAGS),
    JS_FS("cancel", unit_cancel, 0, FUNCTION_FLAGS),
    JS_FS("repair", unit_repair, 0, FUNCTION_FLAGS),
    JS_FS("useMenu", unit_useMenu, 0, FUNCTION_FLAGS),
    JS_FS("interact", unit_interact, 0, FUNCTION_FLAGS),
    JS_FS("getItem", unit_getItem, 3, FUNCTION_FLAGS),
    JS_FS("getItems", unit_getItems, 0, FUNCTION_FLAGS),
    JS_FS("getMerc", unit_getMerc, 0, FUNCTION_FLAGS),
    JS_FS("getMercHP", unit_getMercHP, 0, FUNCTION_FLAGS),
    JS_FS("getSkill", unit_getSkill, 0, FUNCTION_FLAGS),
    JS_FS("getParent", unit_getParent, 0, FUNCTION_FLAGS),
    JS_FS("overhead", my_overhead, 0, FUNCTION_FLAGS),
    JS_FS("revive", my_revive, 0, FUNCTION_FLAGS),
    JS_FS("getFlags", item_getFlags, 1, FUNCTION_FLAGS),
    JS_FS("getFlag", item_getFlag, 1, FUNCTION_FLAGS),
    JS_FS("getStat", unit_getStat, 1, FUNCTION_FLAGS),
    JS_FS("getState", unit_getState, 1, FUNCTION_FLAGS),
    JS_FS("getEnchant", unit_getEnchant, 1, FUNCTION_FLAGS),
    JS_FS("shop", item_shop, 2, FUNCTION_FLAGS),
    JS_FS("setSkill", unit_setskill, 2, FUNCTION_FLAGS),
    JS_FS("move", unit_move, 2, FUNCTION_FLAGS),
    JS_FS("getQuest", unit_getQuest, 2, FUNCTION_FLAGS),
    JS_FS("getMinionCount", unit_getMinionCount, 1, FUNCTION_FLAGS),
    JS_FS("getRepairCost", me_getRepairCost, 1, FUNCTION_FLAGS),
    JS_FS("getItemCost", item_getItemCost, 1, FUNCTION_FLAGS),
};

static JSCFunctionListEntry unit_proto_funcs[] = {
    JS_CGETSET_MAGIC_DEF("type", unit_getProperty, nullptr, UNIT_TYPE),
    JS_CGETSET_MAGIC_DEF("classid", unit_getProperty, nullptr, UNIT_CLASSID),
    JS_CGETSET_MAGIC_DEF("mode", unit_getProperty, nullptr, UNIT_MODE),
    JS_CGETSET_MAGIC_DEF("name", unit_getProperty, nullptr, UNIT_NAME),
    JS_CGETSET_MAGIC_DEF("act", unit_getProperty, nullptr, UNIT_ACT),
    JS_CGETSET_MAGIC_DEF("gid", unit_getProperty, nullptr, UNIT_ID),
    JS_CGETSET_MAGIC_DEF("x", unit_getProperty, nullptr, UNIT_XPOS),
    JS_CGETSET_MAGIC_DEF("y", unit_getProperty, nullptr, UNIT_YPOS),
    JS_CGETSET_MAGIC_DEF("targetx", unit_getProperty, nullptr, UNIT_TARGETX),
    JS_CGETSET_MAGIC_DEF("targety", unit_getProperty, nullptr, UNIT_TARGETY),
    JS_CGETSET_MAGIC_DEF("area", unit_getProperty, nullptr, UNIT_AREA),
    JS_CGETSET_MAGIC_DEF("hp", unit_getProperty, nullptr, UNIT_HP),
    JS_CGETSET_MAGIC_DEF("hpmax", unit_getProperty, nullptr, UNIT_HPMAX),
    JS_CGETSET_MAGIC_DEF("mp", unit_getProperty, nullptr, UNIT_MP),
    JS_CGETSET_MAGIC_DEF("mpmax", unit_getProperty, nullptr, UNIT_MPMAX),
    JS_CGETSET_MAGIC_DEF("stamina", unit_getProperty, nullptr, UNIT_STAMINA),
    JS_CGETSET_MAGIC_DEF("staminamax", unit_getProperty, nullptr, UNIT_STAMINAMAX),
    JS_CGETSET_MAGIC_DEF("charlvl", unit_getProperty, nullptr, UNIT_CHARLVL),
    JS_CGETSET_MAGIC_DEF("itemcount", unit_getProperty, nullptr, UNIT_ITEMCOUNT),
    JS_CGETSET_MAGIC_DEF("owner", unit_getProperty, nullptr, UNIT_OWNER),
    JS_CGETSET_MAGIC_DEF("ownertype", unit_getProperty, nullptr, UNIT_OWNERTYPE),
    JS_CGETSET_MAGIC_DEF("spectype", unit_getProperty, nullptr, UNIT_SPECTYPE),
    JS_CGETSET_MAGIC_DEF("direction", unit_getProperty, nullptr, UNIT_DIRECTION),
    JS_CGETSET_MAGIC_DEF("uniqueid", unit_getProperty, nullptr, UNIT_UNIQUEID),
    JS_CGETSET_MAGIC_DEF("code", unit_getProperty, nullptr, ITEM_CODE),
    JS_CGETSET_MAGIC_DEF("prefix", unit_getProperty, nullptr, ITEM_PREFIX),
    JS_CGETSET_MAGIC_DEF("suffix", unit_getProperty, nullptr, ITEM_SUFFIX),
    JS_CGETSET_MAGIC_DEF("prefixes", unit_getProperty, nullptr, ITEM_PREFIXES),
    JS_CGETSET_MAGIC_DEF("suffixes", unit_getProperty, nullptr, ITEM_SUFFIXES),
    JS_CGETSET_MAGIC_DEF("prefixnum", unit_getProperty, nullptr, ITEM_PREFIXNUM),
    JS_CGETSET_MAGIC_DEF("suffixnum", unit_getProperty, nullptr, ITEM_SUFFIXNUM),
    JS_CGETSET_MAGIC_DEF("prefixnums", unit_getProperty, nullptr, ITEM_PREFIXNUMS),
    JS_CGETSET_MAGIC_DEF("suffixnums", unit_getProperty, nullptr, ITEM_SUFFIXNUMS),
    JS_CGETSET_MAGIC_DEF("fname", unit_getProperty, nullptr, ITEM_FNAME),
    JS_CGETSET_MAGIC_DEF("quality", unit_getProperty, nullptr, ITEM_QUALITY),
    JS_CGETSET_MAGIC_DEF("node", unit_getProperty, nullptr, ITEM_NODE),
    JS_CGETSET_MAGIC_DEF("location", unit_getProperty, nullptr, ITEM_LOC),
    JS_CGETSET_MAGIC_DEF("sizex", unit_getProperty, nullptr, ITEM_SIZEX),
    JS_CGETSET_MAGIC_DEF("sizey", unit_getProperty, nullptr, ITEM_SIZEY),
    JS_CGETSET_MAGIC_DEF("itemType", unit_getProperty, nullptr, ITEM_TYPE),
    JS_CGETSET_MAGIC_DEF("description", unit_getProperty, nullptr, ITEM_DESC),
    JS_CGETSET_MAGIC_DEF("bodylocation", unit_getProperty, nullptr, ITEM_BODYLOCATION),
    JS_CGETSET_MAGIC_DEF("ilvl", unit_getProperty, nullptr, ITEM_LEVEL),
    JS_CGETSET_MAGIC_DEF("lvlreq", unit_getProperty, nullptr, ITEM_LEVELREQ),
    JS_CGETSET_MAGIC_DEF("gfx", unit_getProperty, nullptr, ITEM_GFX),
    JS_CGETSET_MAGIC_DEF("runwalk", unit_getProperty, nullptr, ME_RUNWALK),
    JS_CGETSET_MAGIC_DEF("weaponswitch", unit_getProperty, nullptr, ME_WSWITCH),
    JS_CGETSET_MAGIC_DEF("objtype", unit_getProperty, nullptr, OBJECT_TYPE),
    JS_CGETSET_MAGIC_DEF("islocked", unit_getProperty, nullptr, OBJECT_LOCKED),

    JS_FS("getNext", unit_getNext, 0, FUNCTION_FLAGS),
    JS_FS("cancel", unit_cancel, 0, FUNCTION_FLAGS),
    JS_FS("repair", unit_repair, 0, FUNCTION_FLAGS),
    JS_FS("useMenu", unit_useMenu, 0, FUNCTION_FLAGS),
    JS_FS("interact", unit_interact, 0, FUNCTION_FLAGS),
    JS_FS("getItem", unit_getItem, 3, FUNCTION_FLAGS),
    JS_FS("getItems", unit_getItems, 0, FUNCTION_FLAGS),
    JS_FS("getMerc", unit_getMerc, 0, FUNCTION_FLAGS),
    JS_FS("getMercHP", unit_getMercHP, 0, FUNCTION_FLAGS),
    JS_FS("getSkill", unit_getSkill, 0, FUNCTION_FLAGS),
    JS_FS("getParent", unit_getParent, 0, FUNCTION_FLAGS),
    JS_FS("overhead", my_overhead, 0, FUNCTION_FLAGS),
    JS_FS("revive", my_revive, 0, FUNCTION_FLAGS),
    JS_FS("getFlags", item_getFlags, 1, FUNCTION_FLAGS),
    JS_FS("getFlag", item_getFlag, 1, FUNCTION_FLAGS),
    JS_FS("getStat", unit_getStat, 1, FUNCTION_FLAGS),
    JS_FS("getState", unit_getState, 1, FUNCTION_FLAGS),
    JS_FS("getEnchant", unit_getEnchant, 1, FUNCTION_FLAGS),
    JS_FS("shop", item_shop, 2, FUNCTION_FLAGS),
    JS_FS("setSkill", unit_setskill, 2, FUNCTION_FLAGS),
    JS_FS("move", unit_move, 2, FUNCTION_FLAGS),
    JS_FS("getQuest", unit_getQuest, 2, FUNCTION_FLAGS),
    JS_FS("getMinionCount", unit_getMinionCount, 1, FUNCTION_FLAGS),
    JS_FS("getRepairCost", me_getRepairCost, 1, FUNCTION_FLAGS),
    JS_FS("getItemCost", item_getItemCost, 1, FUNCTION_FLAGS),
};
