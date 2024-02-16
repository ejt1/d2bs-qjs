#pragma once

#include "JSBaseObject.h"
#include "Offset.h"

#include <Windows.h>

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

class UnitWrap : public BaseObject {
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

  static JSObject* Instantiate(JSContext* ctx, UnitData* priv, bool isMe = false);
  static void Initialize(JSContext* ctx, JS::HandleObject target);

  UnitData* GetData();

 private:
  UnitWrap(JSContext* ctx, JS::HandleObject obj, UnitData* priv);
  virtual ~UnitWrap();

  static void finalize(JSFreeOp* fop, JSObject* obj);

  // constructor
  static bool New(JSContext* ctx, unsigned argc, JS::Value* vp);

  // properties
  static bool GetType(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetClassID(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetMode(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetName(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetAct(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetGid(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetTargetX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetTargetY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetArea(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetHP(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetHPMax(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetMP(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetMPMax(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetStamina(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetStaminaMax(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetCharLevel(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetItemCount(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetOwner(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetOwnerType(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetSpecType(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetDirection(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetUniqueId(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetCode(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetPrefix(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetSuffix(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetPrefixes(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetSuffixes(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetPrefixNum(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetSuffixNum(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetPrefixNums(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetSuffixNums(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetFName(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetQuality(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetNode(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetLocation(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetSizeX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetSizeY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetItemType(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetDescription(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetBodyLocation(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetItemLevel(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetLevelRequirement(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetGfx(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetRunWalk(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetRunWalk(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetWeaaponSwitch(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetObjType(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetIsLocked(JSContext* ctx, unsigned argc, JS::Value* vp);

  // properties for 'me'
  static bool GetAccount(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetCharName(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetDiff(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetDiffMax(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetGameName(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetGamePassword(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetGameServerIp(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetGameStartTime(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetGameType(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetItemOnCursor(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetAutomap(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetAutomap(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetLadder(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetPing(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetFPS(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetLocale(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetPlayerType(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetRealm(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetRealmShort(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetMercReviveCost(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetChickenHP(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetChickenHP(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetChickenMP(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetChickenMP(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetQuitOnHostile(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetQuitOnHostile(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetBlockKeys(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetBlockKeys(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetBlockMouse(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetBlockMouse(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetGameReady(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetProfile(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetNoPickup(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetNoPickup(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetPID(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetUnsupported(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetCharFlags(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetScreenSize(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetWindowTitle(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetInGame(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetQuitOnError(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetQuitOnError(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetMaxGameTime(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetMaxGameTime(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetMapId(JSContext* ctx, unsigned argc, JS::Value* vp);

  // function
  static bool getNext(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool cancel(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool repair(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool useMenu(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool interact(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool getItem(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool getItems(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool getMerc(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool getMercHP(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool getSkill(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool getParent(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool overhead(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool revive(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool getFlags(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool getFlag(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool getStat(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool getState(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool getEnchant(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool shop(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool setSkill(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool move(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool getQuest(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool getMinionCount(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool getRepairCost(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool getItemCost(JSContext* ctx, unsigned argc, JS::Value* vp);

  // globals
  static bool getUnit(JSContext* ctx, unsigned argc, JS::Value* vp);

  static inline JSClassOps m_ops = {
      .addProperty = nullptr,
      .delProperty = nullptr,
      .enumerate = nullptr,
      .newEnumerate = nullptr,
      .resolve = nullptr,
      .mayResolve = nullptr,
      .finalize = finalize,
      .call = nullptr,
      .hasInstance = nullptr,
      .construct = nullptr,
      .trace = nullptr,
  };
  static inline JSClass m_class = {
      "Unit",
      JSCLASS_HAS_RESERVED_SLOTS(kInternalFieldCount) | JSCLASS_FOREGROUND_FINALIZE,
      &m_ops,
  };
  static inline JSPropertySpec m_unit_props[] = {
      JS_PSG("type", GetType, JSPROP_ENUMERATE),
      JS_PSG("classid", GetClassID, JSPROP_ENUMERATE),
      JS_PSG("mode", GetMode, JSPROP_ENUMERATE),
      JS_PSG("name", GetName, JSPROP_ENUMERATE),
      JS_PSG("act", GetAct, JSPROP_ENUMERATE),
      JS_PSG("gid", GetGid, JSPROP_ENUMERATE),
      JS_PSG("x", GetX, JSPROP_ENUMERATE),
      JS_PSG("y", GetY, JSPROP_ENUMERATE),
      JS_PSG("targetx", GetTargetX, JSPROP_ENUMERATE),
      JS_PSG("targety", GetTargetY, JSPROP_ENUMERATE),
      JS_PSG("area", GetArea, JSPROP_ENUMERATE),
      JS_PSG("hp", GetHP, JSPROP_ENUMERATE),
      JS_PSG("hpmax", GetHPMax, JSPROP_ENUMERATE),
      JS_PSG("mp", GetMP, JSPROP_ENUMERATE),
      JS_PSG("mpmax", GetMPMax, JSPROP_ENUMERATE),
      JS_PSG("stamina", GetStamina, JSPROP_ENUMERATE),
      JS_PSG("staminamax", GetStaminaMax, JSPROP_ENUMERATE),
      JS_PSG("charlvl", GetCharLevel, JSPROP_ENUMERATE),
      JS_PSG("itemcount", GetItemCount, JSPROP_ENUMERATE),
      JS_PSG("owner", GetOwner, JSPROP_ENUMERATE),
      JS_PSG("ownertype", GetOwnerType, JSPROP_ENUMERATE),
      JS_PSG("spectype", GetSpecType, JSPROP_ENUMERATE),
      JS_PSG("direction", GetDirection, JSPROP_ENUMERATE),
      JS_PSG("uniqueid", GetUniqueId, JSPROP_ENUMERATE),
      JS_PSG("code", GetCode, JSPROP_ENUMERATE),
      JS_PSG("prefix", GetPrefix, JSPROP_ENUMERATE),
      JS_PSG("suffix", GetSuffix, JSPROP_ENUMERATE),
      JS_PSG("prefixes", GetPrefixes, JSPROP_ENUMERATE),
      JS_PSG("suffixes", GetSuffixes, JSPROP_ENUMERATE),
      JS_PSG("prefixnum", GetPrefixNum, JSPROP_ENUMERATE),
      JS_PSG("suffixnum", GetSuffixNum, JSPROP_ENUMERATE),
      JS_PSG("prefixnums", GetPrefixNums, JSPROP_ENUMERATE),
      JS_PSG("suffixnums", GetSuffixNums, JSPROP_ENUMERATE),
      JS_PSG("fname", GetFName, JSPROP_ENUMERATE),
      JS_PSG("quality", GetQuality, JSPROP_ENUMERATE),
      JS_PSG("node", GetNode, JSPROP_ENUMERATE),
      JS_PSG("location", GetLocation, JSPROP_ENUMERATE),
      JS_PSG("sizex", GetSizeX, JSPROP_ENUMERATE),
      JS_PSG("sizey", GetSizeY, JSPROP_ENUMERATE),
      JS_PSG("itemType", GetItemType, JSPROP_ENUMERATE),
      JS_PSG("description", GetDescription, JSPROP_ENUMERATE),
      JS_PSG("bodylocation", GetBodyLocation, JSPROP_ENUMERATE),
      JS_PSG("ilvl", GetItemLevel, JSPROP_ENUMERATE),
      JS_PSG("lvlreq", GetLevelRequirement, JSPROP_ENUMERATE),
      JS_PSG("gfx", GetGfx, JSPROP_ENUMERATE),
      JS_PSGS("runwalk", GetRunWalk, SetRunWalk, JSPROP_ENUMERATE),
      JS_PSG("weaponswitch", GetWeaaponSwitch, JSPROP_ENUMERATE),
      JS_PSG("objtype", GetObjType, JSPROP_ENUMERATE),
      JS_PSG("islocked", GetIsLocked, JSPROP_ENUMERATE),
      JS_PS_END,
  };
  static inline JSFunctionSpec m_unit_methods[] = {
      JS_FN("getNext", getNext, 0, JSPROP_ENUMERATE),
      JS_FN("cancel", cancel, 0, JSPROP_ENUMERATE),
      JS_FN("repair", repair, 0, JSPROP_ENUMERATE),
      JS_FN("useMenu", useMenu, 0, JSPROP_ENUMERATE),
      JS_FN("interact", interact, 0, JSPROP_ENUMERATE),
      JS_FN("getItem", getItem, 3, JSPROP_ENUMERATE),
      JS_FN("getItems", getItems, 0, JSPROP_ENUMERATE),
      JS_FN("getMerc", getMerc, 0, JSPROP_ENUMERATE),
      JS_FN("getMercHP", getMercHP, 0, JSPROP_ENUMERATE),
      JS_FN("getSkill", getSkill, 0, JSPROP_ENUMERATE),
      JS_FN("getParent", getParent, 0, JSPROP_ENUMERATE),
      JS_FN("overhead", overhead, 0, JSPROP_ENUMERATE),
      JS_FN("revive", revive, 0, JSPROP_ENUMERATE),
      JS_FN("getFlags", getFlags, 1, JSPROP_ENUMERATE),
      JS_FN("getFlag", getFlag, 1, JSPROP_ENUMERATE),
      JS_FN("getStat", getStat, 1, JSPROP_ENUMERATE),
      JS_FN("getState", getState, 1, JSPROP_ENUMERATE),
      JS_FN("getEnchant", getEnchant, 1, JSPROP_ENUMERATE),
      JS_FN("shop", shop, 2, JSPROP_ENUMERATE),
      JS_FN("setSkill", setSkill, 2, JSPROP_ENUMERATE),
      JS_FN("move", move, 2, JSPROP_ENUMERATE),
      JS_FN("getQuest", getQuest, 2, JSPROP_ENUMERATE),
      JS_FN("getMinionCount", getMinionCount, 1, JSPROP_ENUMERATE),
      JS_FN("getRepairCost", getRepairCost, 1, JSPROP_ENUMERATE),
      JS_FN("getItemCost", getItemCost, 1, JSPROP_ENUMERATE),
      JS_FS_END,
  };

  UnitData* pData;
};

#define UNWRAP_UNIT_OR_RETURN(ctx, unused, obj)                                     \
  if (ClientState() != ClientStateInGame) {                                         \
    args.rval().setUndefined();                                                     \
    return true;                                                                    \
  }                                                                                 \
  UnitWrap* wrap;                                                                   \
  UNWRAP_OR_RETURN(ctx, &wrap, obj)                                                 \
  UnitWrap::UnitData* pUnitData = wrap->GetData();                                  \
  if (!pUnitData) {                                                                 \
    args.rval().setUndefined();                                                     \
    return true;                                                                    \
  }                                                                                 \
  D2UnitStrc* pUnit = D2UnitStrc::FindUnit(pUnitData->dwUnitId, pUnitData->dwType); \
  if (!pUnit) {                                                                     \
    args.rval().setUndefined();                                                     \
    return true;                                                                    \
  }

#define UNWRAP_UNIT_OR_ERROR(ctx, unused, obj)                                      \
  if (ClientState() != ClientStateInGame) {                                         \
    args.rval().setUndefined();                                                     \
    return true;                                                                    \
  }                                                                                 \
  UnitWrap* wrap;                                                                   \
  UNWRAP_OR_RETURN(ctx, &wrap, obj)                                                 \
  UnitWrap::UnitData* pUnitData = wrap->GetData();                                  \
  if (!pUnitData) {                                                                 \
    args.rval().setUndefined();                                                     \
    return true;                                                                    \
  }                                                                                 \
  D2UnitStrc* pUnit = D2UnitStrc::FindUnit(pUnitData->dwUnitId, pUnitData->dwType); \
  if (!pUnit) {                                                                     \
    THROW_ERROR(ctx, "Unable to get Unit");                                         \
  }
