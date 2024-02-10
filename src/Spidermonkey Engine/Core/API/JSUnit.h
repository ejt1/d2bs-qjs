#pragma once

#include <windows.h>
#include "js32.h"
#include "Offset.h"

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

#define PRIVATE_UNIT 1
#define PRIVATE_ITEM 3

class UnitWrap {
 public:
  struct UnitData {
    DWORD dwPrivateType;
    DWORD dwUnitId;
    DWORD dwClassId;
    DWORD dwType;
    DWORD dwMode;
    char szName[128];
  };

  struct ItemData : UnitData {
    DWORD dwOwnerId;
    DWORD dwOwnerType;
  };

  static JSValue Instantiate(JSContext* ctx, JSValue new_target, UnitData* priv);
  static void Initialize(JSContext* ctx, JSValue target);

  // helper functions to make migration to new wrap model easier
  UnitData* GetData();
  static UnitWrap* FromJSObject(JSValue value);
  static UnitData* DataFromJSObject(JSValue value);

 private:
  UnitWrap(JSContext* ctx, UnitData* priv);
  virtual ~UnitWrap();

  // constructor
  static JSValue New(JSContext* ctx, JSValue new_target, int argc, JSValue* argv);

  // properties
  static JSValue GetProperty(JSContext* ctx, JSValue this_val, int magic);
  static JSValue SetProperty(JSContext* ctx, JSValue this_val, JSValue val, int magic);

  // function
  static JSValue getNext(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue cancel(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue repair(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue useMenu(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue interact(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue getItem(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue getItems(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue getMerc(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue getMercHP(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue getSkill(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue getParent(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue overhead(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue revive(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue getFlags(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue getFlag(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue getStat(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue getState(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue getEnchant(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue shop(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue setSkill(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue move(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue getQuest(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue getMinionCount(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue getRepairCost(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue getItemCost(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  // globals
  static JSValue getUnit(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  static inline JSClassID m_unit_class_id = 0;
  static inline JSCFunctionListEntry m_unit_proto_funcs[] = {
      JS_CGETSET_MAGIC_DEF("type", GetProperty, nullptr, UNIT_TYPE),
      JS_CGETSET_MAGIC_DEF("classid", GetProperty, nullptr, UNIT_CLASSID),
      JS_CGETSET_MAGIC_DEF("mode", GetProperty, nullptr, UNIT_MODE),
      JS_CGETSET_MAGIC_DEF("name", GetProperty, nullptr, UNIT_NAME),
      JS_CGETSET_MAGIC_DEF("act", GetProperty, nullptr, UNIT_ACT),
      JS_CGETSET_MAGIC_DEF("gid", GetProperty, nullptr, UNIT_ID),
      JS_CGETSET_MAGIC_DEF("x", GetProperty, nullptr, UNIT_XPOS),
      JS_CGETSET_MAGIC_DEF("y", GetProperty, nullptr, UNIT_YPOS),
      JS_CGETSET_MAGIC_DEF("targetx", GetProperty, nullptr, UNIT_TARGETX),
      JS_CGETSET_MAGIC_DEF("targety", GetProperty, nullptr, UNIT_TARGETY),
      JS_CGETSET_MAGIC_DEF("area", GetProperty, nullptr, UNIT_AREA),
      JS_CGETSET_MAGIC_DEF("hp", GetProperty, nullptr, UNIT_HP),
      JS_CGETSET_MAGIC_DEF("hpmax", GetProperty, nullptr, UNIT_HPMAX),
      JS_CGETSET_MAGIC_DEF("mp", GetProperty, nullptr, UNIT_MP),
      JS_CGETSET_MAGIC_DEF("mpmax", GetProperty, nullptr, UNIT_MPMAX),
      JS_CGETSET_MAGIC_DEF("stamina", GetProperty, nullptr, UNIT_STAMINA),
      JS_CGETSET_MAGIC_DEF("staminamax", GetProperty, nullptr, UNIT_STAMINAMAX),
      JS_CGETSET_MAGIC_DEF("charlvl", GetProperty, nullptr, UNIT_CHARLVL),
      JS_CGETSET_MAGIC_DEF("itemcount", GetProperty, nullptr, UNIT_ITEMCOUNT),
      JS_CGETSET_MAGIC_DEF("owner", GetProperty, nullptr, UNIT_OWNER),
      JS_CGETSET_MAGIC_DEF("ownertype", GetProperty, nullptr, UNIT_OWNERTYPE),
      JS_CGETSET_MAGIC_DEF("spectype", GetProperty, nullptr, UNIT_SPECTYPE),
      JS_CGETSET_MAGIC_DEF("direction", GetProperty, nullptr, UNIT_DIRECTION),
      JS_CGETSET_MAGIC_DEF("uniqueid", GetProperty, nullptr, UNIT_UNIQUEID),
      JS_CGETSET_MAGIC_DEF("code", GetProperty, nullptr, ITEM_CODE),
      JS_CGETSET_MAGIC_DEF("prefix", GetProperty, nullptr, ITEM_PREFIX),
      JS_CGETSET_MAGIC_DEF("suffix", GetProperty, nullptr, ITEM_SUFFIX),
      JS_CGETSET_MAGIC_DEF("prefixes", GetProperty, nullptr, ITEM_PREFIXES),
      JS_CGETSET_MAGIC_DEF("suffixes", GetProperty, nullptr, ITEM_SUFFIXES),
      JS_CGETSET_MAGIC_DEF("prefixnum", GetProperty, nullptr, ITEM_PREFIXNUM),
      JS_CGETSET_MAGIC_DEF("suffixnum", GetProperty, nullptr, ITEM_SUFFIXNUM),
      JS_CGETSET_MAGIC_DEF("prefixnums", GetProperty, nullptr, ITEM_PREFIXNUMS),
      JS_CGETSET_MAGIC_DEF("suffixnums", GetProperty, nullptr, ITEM_SUFFIXNUMS),
      JS_CGETSET_MAGIC_DEF("fname", GetProperty, nullptr, ITEM_FNAME),
      JS_CGETSET_MAGIC_DEF("quality", GetProperty, nullptr, ITEM_QUALITY),
      JS_CGETSET_MAGIC_DEF("node", GetProperty, nullptr, ITEM_NODE),
      JS_CGETSET_MAGIC_DEF("location", GetProperty, nullptr, ITEM_LOC),
      JS_CGETSET_MAGIC_DEF("sizex", GetProperty, nullptr, ITEM_SIZEX),
      JS_CGETSET_MAGIC_DEF("sizey", GetProperty, nullptr, ITEM_SIZEY),
      JS_CGETSET_MAGIC_DEF("itemType", GetProperty, nullptr, ITEM_TYPE),
      JS_CGETSET_MAGIC_DEF("description", GetProperty, nullptr, ITEM_DESC),
      JS_CGETSET_MAGIC_DEF("bodylocation", GetProperty, nullptr, ITEM_BODYLOCATION),
      JS_CGETSET_MAGIC_DEF("ilvl", GetProperty, nullptr, ITEM_LEVEL),
      JS_CGETSET_MAGIC_DEF("lvlreq", GetProperty, nullptr, ITEM_LEVELREQ),
      JS_CGETSET_MAGIC_DEF("gfx", GetProperty, nullptr, ITEM_GFX),
      JS_CGETSET_MAGIC_DEF("runwalk", GetProperty, nullptr, ME_RUNWALK),
      JS_CGETSET_MAGIC_DEF("weaponswitch", GetProperty, nullptr, ME_WSWITCH),
      JS_CGETSET_MAGIC_DEF("objtype", GetProperty, nullptr, OBJECT_TYPE),
      JS_CGETSET_MAGIC_DEF("islocked", GetProperty, nullptr, OBJECT_LOCKED),

      JS_FS("getNext", getNext, 0, FUNCTION_FLAGS),
      JS_FS("cancel", cancel, 0, FUNCTION_FLAGS),
      JS_FS("repair", repair, 0, FUNCTION_FLAGS),
      JS_FS("useMenu", useMenu, 0, FUNCTION_FLAGS),
      JS_FS("interact", interact, 0, FUNCTION_FLAGS),
      JS_FS("getItem", getItem, 3, FUNCTION_FLAGS),
      JS_FS("getItems", getItems, 0, FUNCTION_FLAGS),
      JS_FS("getMerc", getMerc, 0, FUNCTION_FLAGS),
      JS_FS("getMercHP", getMercHP, 0, FUNCTION_FLAGS),
      JS_FS("getSkill", getSkill, 0, FUNCTION_FLAGS),
      JS_FS("getParent", getParent, 0, FUNCTION_FLAGS),
      JS_FS("overhead", overhead, 0, FUNCTION_FLAGS),
      JS_FS("revive", revive, 0, FUNCTION_FLAGS),
      JS_FS("getFlags", getFlags, 1, FUNCTION_FLAGS),
      JS_FS("getFlag", getFlag, 1, FUNCTION_FLAGS),
      JS_FS("getStat", getStat, 1, FUNCTION_FLAGS),
      JS_FS("getState", getState, 1, FUNCTION_FLAGS),
      JS_FS("getEnchant", getEnchant, 1, FUNCTION_FLAGS),
      JS_FS("shop", shop, 2, FUNCTION_FLAGS),
      JS_FS("setSkill", setSkill, 2, FUNCTION_FLAGS),
      JS_FS("move", move, 2, FUNCTION_FLAGS),
      JS_FS("getQuest", getQuest, 2, FUNCTION_FLAGS),
      JS_FS("getMinionCount", getMinionCount, 1, FUNCTION_FLAGS),
      JS_FS("getRepairCost", getRepairCost, 1, FUNCTION_FLAGS),
      JS_FS("getItemCost", getItemCost, 1, FUNCTION_FLAGS),
  };

  UnitData* pData;

 public:
  // dont worry about it :)
  static inline JSCFunctionListEntry me_proto_funcs[] = {
      JS_CGETSET_MAGIC_DEF("account", GetProperty, nullptr, ME_ACCOUNT),
      JS_CGETSET_MAGIC_DEF("charname", GetProperty, nullptr, ME_CHARNAME),
      JS_CGETSET_MAGIC_DEF("diff", GetProperty, nullptr, ME_DIFF),
      JS_CGETSET_MAGIC_DEF("maxdiff", GetProperty, nullptr, ME_MAXDIFF),
      JS_CGETSET_MAGIC_DEF("gamename", GetProperty, nullptr, ME_GAMENAME),
      JS_CGETSET_MAGIC_DEF("gamepassword", GetProperty, nullptr, ME_GAMEPASSWORD),
      JS_CGETSET_MAGIC_DEF("gameserverip", GetProperty, nullptr, ME_GAMESERVERIP),
      JS_CGETSET_MAGIC_DEF("gamestarttime", GetProperty, nullptr, ME_GAMESTARTTIME),
      JS_CGETSET_MAGIC_DEF("gametype", GetProperty, nullptr, ME_GAMETYPE),
      JS_CGETSET_MAGIC_DEF("itemoncursor", GetProperty, nullptr, ME_ITEMONCURSOR),
      JS_CGETSET_MAGIC_DEF("automap", GetProperty, SetProperty, ME_AUTOMAP),
      JS_CGETSET_MAGIC_DEF("ladder", GetProperty, nullptr, ME_LADDER),
      JS_CGETSET_MAGIC_DEF("ping", GetProperty, nullptr, ME_PING),
      JS_CGETSET_MAGIC_DEF("fps", GetProperty, nullptr, ME_FPS),
      JS_CGETSET_MAGIC_DEF("locale", GetProperty, nullptr, ME_LOCALE),
      JS_CGETSET_MAGIC_DEF("playertype", GetProperty, nullptr, ME_PLAYERTYPE),
      JS_CGETSET_MAGIC_DEF("realm", GetProperty, nullptr, ME_REALM),
      JS_CGETSET_MAGIC_DEF("realmshort", GetProperty, nullptr, ME_REALMSHORT),
      JS_CGETSET_MAGIC_DEF("mercrevivecost", GetProperty, nullptr, ME_MERCREVIVECOST),
      JS_CGETSET_MAGIC_DEF("runwalk", GetProperty, SetProperty, ME_RUNWALK),
      JS_CGETSET_MAGIC_DEF("weaponswitch", GetProperty, nullptr, ME_WSWITCH),
      JS_CGETSET_MAGIC_DEF("chickenhp", GetProperty, SetProperty, ME_CHICKENHP),
      JS_CGETSET_MAGIC_DEF("chickenmp", GetProperty, SetProperty, ME_CHICKENMP),
      JS_CGETSET_MAGIC_DEF("quitonhostile", GetProperty, SetProperty, ME_QUITONHOSTILE),
      JS_CGETSET_MAGIC_DEF("blockKeys", GetProperty, SetProperty, ME_BLOCKKEYS),
      JS_CGETSET_MAGIC_DEF("blockMouse", GetProperty, SetProperty, ME_BLOCKMOUSE),
      JS_CGETSET_MAGIC_DEF("gameReady", GetProperty, nullptr, ME_GAMEREADY),
      JS_CGETSET_MAGIC_DEF("profile", GetProperty, nullptr, ME_PROFILE),
      JS_CGETSET_MAGIC_DEF("nopickup", GetProperty, SetProperty, ME_NOPICKUP),
      JS_CGETSET_MAGIC_DEF("pid", GetProperty, nullptr, ME_PID),
      JS_CGETSET_MAGIC_DEF("unsupported", GetProperty, nullptr, ME_UNSUPPORTED),
      JS_CGETSET_MAGIC_DEF("charflags", GetProperty, nullptr, ME_CHARFLAGS),
      JS_CGETSET_MAGIC_DEF("screensize", GetProperty, nullptr, OOG_SCREENSIZE),
      JS_CGETSET_MAGIC_DEF("windowtitle", GetProperty, nullptr, OOG_WINDOWTITLE),
      JS_CGETSET_MAGIC_DEF("ingame", GetProperty, nullptr, OOG_INGAME),
      JS_CGETSET_MAGIC_DEF("quitonerror", GetProperty, SetProperty, OOG_QUITONERROR),
      JS_CGETSET_MAGIC_DEF("maxgametime", GetProperty, SetProperty, OOG_MAXGAMETIME),
      JS_CGETSET_MAGIC_DEF("mapid", GetProperty, nullptr, ME_MAPID),
  };
};