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
  static bool New(JSContext* ctx, JS::CallArgs& args);

  // properties
  static bool GetType(JSContext* ctx, JS::CallArgs& args);
  static bool GetClassID(JSContext* ctx, JS::CallArgs& args);
  static bool GetMode(JSContext* ctx, JS::CallArgs& args);
  static bool GetName(JSContext* ctx, JS::CallArgs& args);
  static bool GetAct(JSContext* ctx, JS::CallArgs& args);
  static bool GetGid(JSContext* ctx, JS::CallArgs& args);
  static bool GetX(JSContext* ctx, JS::CallArgs& args);
  static bool GetY(JSContext* ctx, JS::CallArgs& args);
  static bool GetTargetX(JSContext* ctx, JS::CallArgs& args);
  static bool GetTargetY(JSContext* ctx, JS::CallArgs& args);
  static bool GetArea(JSContext* ctx, JS::CallArgs& args);
  static bool GetHP(JSContext* ctx, JS::CallArgs& args);
  static bool GetHPMax(JSContext* ctx, JS::CallArgs& args);
  static bool GetMP(JSContext* ctx, JS::CallArgs& args);
  static bool GetMPMax(JSContext* ctx, JS::CallArgs& args);
  static bool GetStamina(JSContext* ctx, JS::CallArgs& args);
  static bool GetStaminaMax(JSContext* ctx, JS::CallArgs& args);
  static bool GetCharLevel(JSContext* ctx, JS::CallArgs& args);
  static bool GetItemCount(JSContext* ctx, JS::CallArgs& args);
  static bool GetOwner(JSContext* ctx, JS::CallArgs& args);
  static bool GetOwnerType(JSContext* ctx, JS::CallArgs& args);
  static bool GetSpecType(JSContext* ctx, JS::CallArgs& args);
  static bool GetDirection(JSContext* ctx, JS::CallArgs& args);
  static bool GetUniqueId(JSContext* ctx, JS::CallArgs& args);
  static bool GetCode(JSContext* ctx, JS::CallArgs& args);
  static bool GetPrefix(JSContext* ctx, JS::CallArgs& args);
  static bool GetSuffix(JSContext* ctx, JS::CallArgs& args);
  static bool GetPrefixes(JSContext* ctx, JS::CallArgs& args);
  static bool GetSuffixes(JSContext* ctx, JS::CallArgs& args);
  static bool GetPrefixNum(JSContext* ctx, JS::CallArgs& args);
  static bool GetSuffixNum(JSContext* ctx, JS::CallArgs& args);
  static bool GetPrefixNums(JSContext* ctx, JS::CallArgs& args);
  static bool GetSuffixNums(JSContext* ctx, JS::CallArgs& args);
  static bool GetFName(JSContext* ctx, JS::CallArgs& args);
  static bool GetQuality(JSContext* ctx, JS::CallArgs& args);
  static bool GetNode(JSContext* ctx, JS::CallArgs& args);
  static bool GetLocation(JSContext* ctx, JS::CallArgs& args);
  static bool GetSizeX(JSContext* ctx, JS::CallArgs& args);
  static bool GetSizeY(JSContext* ctx, JS::CallArgs& args);
  static bool GetItemType(JSContext* ctx, JS::CallArgs& args);
  static bool GetDescription(JSContext* ctx, JS::CallArgs& args);
  static bool GetBodyLocation(JSContext* ctx, JS::CallArgs& args);
  static bool GetItemLevel(JSContext* ctx, JS::CallArgs& args);
  static bool GetLevelRequirement(JSContext* ctx, JS::CallArgs& args);
  static bool GetGfx(JSContext* ctx, JS::CallArgs& args);
  static bool GetRunWalk(JSContext* ctx, JS::CallArgs& args);
  static bool SetRunWalk(JSContext* ctx, JS::CallArgs& args);
  static bool GetWeaaponSwitch(JSContext* ctx, JS::CallArgs& args);
  static bool GetObjType(JSContext* ctx, JS::CallArgs& args);
  static bool GetIsLocked(JSContext* ctx, JS::CallArgs& args);

  // properties for 'me'
  static bool GetAccount(JSContext* ctx, JS::CallArgs& args);
  static bool GetCharName(JSContext* ctx, JS::CallArgs& args);
  static bool GetDiff(JSContext* ctx, JS::CallArgs& args);
  static bool GetDiffMax(JSContext* ctx, JS::CallArgs& args);
  static bool GetGameName(JSContext* ctx, JS::CallArgs& args);
  static bool GetGamePassword(JSContext* ctx, JS::CallArgs& args);
  static bool GetGameServerIp(JSContext* ctx, JS::CallArgs& args);
  static bool GetGameStartTime(JSContext* ctx, JS::CallArgs& args);
  static bool GetGameType(JSContext* ctx, JS::CallArgs& args);
  static bool GetItemOnCursor(JSContext* ctx, JS::CallArgs& args);
  static bool GetAutomap(JSContext* ctx, JS::CallArgs& args);
  static bool SetAutomap(JSContext* ctx, JS::CallArgs& args);
  static bool GetLadder(JSContext* ctx, JS::CallArgs& args);
  static bool GetPing(JSContext* ctx, JS::CallArgs& args);
  static bool GetFPS(JSContext* ctx, JS::CallArgs& args);
  static bool GetLocale(JSContext* ctx, JS::CallArgs& args);
  static bool GetPlayerType(JSContext* ctx, JS::CallArgs& args);
  static bool GetRealm(JSContext* ctx, JS::CallArgs& args);
  static bool GetRealmShort(JSContext* ctx, JS::CallArgs& args);
  static bool GetMercReviveCost(JSContext* ctx, JS::CallArgs& args);
  static bool GetChickenHP(JSContext* ctx, JS::CallArgs& args);
  static bool SetChickenHP(JSContext* ctx, JS::CallArgs& args);
  static bool GetChickenMP(JSContext* ctx, JS::CallArgs& args);
  static bool SetChickenMP(JSContext* ctx, JS::CallArgs& args);
  static bool GetQuitOnHostile(JSContext* ctx, JS::CallArgs& args);
  static bool SetQuitOnHostile(JSContext* ctx, JS::CallArgs& args);
  static bool GetBlockKeys(JSContext* ctx, JS::CallArgs& args);
  static bool SetBlockKeys(JSContext* ctx, JS::CallArgs& args);
  static bool GetBlockMouse(JSContext* ctx, JS::CallArgs& args);
  static bool SetBlockMouse(JSContext* ctx, JS::CallArgs& args);
  static bool GetGameReady(JSContext* ctx, JS::CallArgs& args);
  static bool GetProfile(JSContext* ctx, JS::CallArgs& args);
  static bool GetNoPickup(JSContext* ctx, JS::CallArgs& args);
  static bool SetNoPickup(JSContext* ctx, JS::CallArgs& args);
  static bool GetPID(JSContext* ctx, JS::CallArgs& args);
  static bool GetUnsupported(JSContext* ctx, JS::CallArgs& args);
  static bool GetCharFlags(JSContext* ctx, JS::CallArgs& args);
  static bool GetScreenSize(JSContext* ctx, JS::CallArgs& args);
  static bool GetWindowTitle(JSContext* ctx, JS::CallArgs& args);
  static bool GetInGame(JSContext* ctx, JS::CallArgs& args);
  static bool GetQuitOnError(JSContext* ctx, JS::CallArgs& args);
  static bool SetQuitOnError(JSContext* ctx, JS::CallArgs& args);
  static bool GetMaxGameTime(JSContext* ctx, JS::CallArgs& args);
  static bool SetMaxGameTime(JSContext* ctx, JS::CallArgs& args);
  static bool GetMapId(JSContext* ctx, JS::CallArgs& args);

  // function
  static bool getNext(JSContext* ctx, JS::CallArgs& args);
  static bool cancel(JSContext* ctx, JS::CallArgs& args);
  static bool repair(JSContext* ctx, JS::CallArgs& args);
  static bool useMenu(JSContext* ctx, JS::CallArgs& args);
  static bool interact(JSContext* ctx, JS::CallArgs& args);
  static bool getItem(JSContext* ctx, JS::CallArgs& args);
  static bool getItems(JSContext* ctx, JS::CallArgs& args);
  static bool getMerc(JSContext* ctx, JS::CallArgs& args);
  static bool getMercHP(JSContext* ctx, JS::CallArgs& args);
  static bool getSkill(JSContext* ctx, JS::CallArgs& args);
  static bool getParent(JSContext* ctx, JS::CallArgs& args);
  static bool overhead(JSContext* ctx, JS::CallArgs& args);
  static bool revive(JSContext* ctx, JS::CallArgs& args);
  static bool getFlags(JSContext* ctx, JS::CallArgs& args);
  static bool getFlag(JSContext* ctx, JS::CallArgs& args);
  static bool getStat(JSContext* ctx, JS::CallArgs& args);
  static bool getState(JSContext* ctx, JS::CallArgs& args);
  static bool getEnchant(JSContext* ctx, JS::CallArgs& args);
  static bool shop(JSContext* ctx, JS::CallArgs& args);
  static bool setSkill(JSContext* ctx, JS::CallArgs& args);
  static bool move(JSContext* ctx, JS::CallArgs& args);
  static bool getQuest(JSContext* ctx, JS::CallArgs& args);
  static bool getMinionCount(JSContext* ctx, JS::CallArgs& args);
  static bool getRepairCost(JSContext* ctx, JS::CallArgs& args);
  static bool getItemCost(JSContext* ctx, JS::CallArgs& args);

  // globals
  static bool getUnit(JSContext* ctx, JS::CallArgs& args);

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
      JS_PSG("type", trampoline<GetType>, JSPROP_ENUMERATE),
      JS_PSG("classid", trampoline<GetClassID>, JSPROP_ENUMERATE),
      JS_PSG("mode", trampoline<GetMode>, JSPROP_ENUMERATE),
      JS_PSG("name", trampoline<GetName>, JSPROP_ENUMERATE),
      JS_PSG("act", trampoline<GetAct>, JSPROP_ENUMERATE),
      JS_PSG("gid", trampoline<GetGid>, JSPROP_ENUMERATE),
      JS_PSG("x", trampoline<GetX>, JSPROP_ENUMERATE),
      JS_PSG("y", trampoline<GetY>, JSPROP_ENUMERATE),
      JS_PSG("targetx", trampoline<GetTargetX>, JSPROP_ENUMERATE),
      JS_PSG("targety", trampoline<GetTargetY>, JSPROP_ENUMERATE),
      JS_PSG("area", trampoline<GetArea>, JSPROP_ENUMERATE),
      JS_PSG("hp", trampoline<GetHP>, JSPROP_ENUMERATE),
      JS_PSG("hpmax", trampoline<GetHPMax>, JSPROP_ENUMERATE),
      JS_PSG("mp", trampoline<GetMP>, JSPROP_ENUMERATE),
      JS_PSG("mpmax", trampoline<GetMPMax>, JSPROP_ENUMERATE),
      JS_PSG("stamina", trampoline<GetStamina>, JSPROP_ENUMERATE),
      JS_PSG("staminamax", trampoline<GetStaminaMax>, JSPROP_ENUMERATE),
      JS_PSG("charlvl", trampoline<GetCharLevel>, JSPROP_ENUMERATE),
      JS_PSG("itemcount", trampoline<GetItemCount>, JSPROP_ENUMERATE),
      JS_PSG("owner", trampoline<GetOwner>, JSPROP_ENUMERATE),
      JS_PSG("ownertype", trampoline<GetOwnerType>, JSPROP_ENUMERATE),
      JS_PSG("spectype", trampoline<GetSpecType>, JSPROP_ENUMERATE),
      JS_PSG("direction", trampoline<GetDirection>, JSPROP_ENUMERATE),
      JS_PSG("uniqueid", trampoline<GetUniqueId>, JSPROP_ENUMERATE),
      JS_PSG("code", trampoline<GetCode>, JSPROP_ENUMERATE),
      JS_PSG("prefix", trampoline<GetPrefix>, JSPROP_ENUMERATE),
      JS_PSG("suffix", trampoline<GetSuffix>, JSPROP_ENUMERATE),
      JS_PSG("prefixes", trampoline<GetPrefixes>, JSPROP_ENUMERATE),
      JS_PSG("suffixes", trampoline<GetSuffixes>, JSPROP_ENUMERATE),
      JS_PSG("prefixnum", trampoline<GetPrefixNum>, JSPROP_ENUMERATE),
      JS_PSG("suffixnum", trampoline<GetSuffixNum>, JSPROP_ENUMERATE),
      JS_PSG("prefixnums", trampoline<GetPrefixNums>, JSPROP_ENUMERATE),
      JS_PSG("suffixnums", trampoline<GetSuffixNums>, JSPROP_ENUMERATE),
      JS_PSG("fname", trampoline<GetFName>, JSPROP_ENUMERATE),
      JS_PSG("quality", trampoline<GetQuality>, JSPROP_ENUMERATE),
      JS_PSG("node", trampoline<GetNode>, JSPROP_ENUMERATE),
      JS_PSG("location", trampoline<GetLocation>, JSPROP_ENUMERATE),
      JS_PSG("sizex", trampoline<GetSizeX>, JSPROP_ENUMERATE),
      JS_PSG("sizey", trampoline<GetSizeY>, JSPROP_ENUMERATE),
      JS_PSG("itemType", trampoline<GetItemType>, JSPROP_ENUMERATE),
      JS_PSG("description", trampoline<GetDescription>, JSPROP_ENUMERATE),
      JS_PSG("bodylocation", trampoline<GetBodyLocation>, JSPROP_ENUMERATE),
      JS_PSG("ilvl", trampoline<GetItemLevel>, JSPROP_ENUMERATE),
      JS_PSG("lvlreq", trampoline<GetLevelRequirement>, JSPROP_ENUMERATE),
      JS_PSG("gfx", trampoline<GetGfx>, JSPROP_ENUMERATE),
      JS_PSGS("runwalk", trampoline<GetRunWalk>, trampoline<SetRunWalk>, JSPROP_ENUMERATE),
      JS_PSG("weaponswitch", trampoline<GetWeaaponSwitch>, JSPROP_ENUMERATE),
      JS_PSG("objtype", trampoline<GetObjType>, JSPROP_ENUMERATE),
      JS_PSG("islocked", trampoline<GetIsLocked>, JSPROP_ENUMERATE),
      JS_PS_END,
  };
  static inline JSFunctionSpec m_unit_methods[] = {
      JS_FN("getNext", trampoline<getNext>, 0, JSPROP_ENUMERATE),
      JS_FN("cancel", trampoline<cancel>, 0, JSPROP_ENUMERATE),
      JS_FN("repair", trampoline<repair>, 0, JSPROP_ENUMERATE),
      JS_FN("useMenu", trampoline<useMenu>, 0, JSPROP_ENUMERATE),
      JS_FN("interact", trampoline<interact>, 0, JSPROP_ENUMERATE),
      JS_FN("getItem", trampoline<getItem>, 3, JSPROP_ENUMERATE),
      JS_FN("getItems", trampoline<getItems>, 0, JSPROP_ENUMERATE),
      JS_FN("getMerc", trampoline<getMerc>, 0, JSPROP_ENUMERATE),
      JS_FN("getMercHP", trampoline<getMercHP>, 0, JSPROP_ENUMERATE),
      JS_FN("getSkill", trampoline<getSkill>, 0, JSPROP_ENUMERATE),
      JS_FN("getParent", trampoline<getParent>, 0, JSPROP_ENUMERATE),
      JS_FN("overhead", trampoline<overhead>, 0, JSPROP_ENUMERATE),
      JS_FN("revive", trampoline<revive>, 0, JSPROP_ENUMERATE),
      JS_FN("getFlags", trampoline<getFlags>, 1, JSPROP_ENUMERATE),
      JS_FN("getFlag", trampoline<getFlag>, 1, JSPROP_ENUMERATE),
      JS_FN("getStat", trampoline<getStat>, 1, JSPROP_ENUMERATE),
      JS_FN("getState", trampoline<getState>, 1, JSPROP_ENUMERATE),
      JS_FN("getEnchant", trampoline<getEnchant>, 1, JSPROP_ENUMERATE),
      JS_FN("shop", trampoline<shop>, 2, JSPROP_ENUMERATE),
      JS_FN("setSkill", trampoline<setSkill>, 2, JSPROP_ENUMERATE),
      JS_FN("move", trampoline<move>, 2, JSPROP_ENUMERATE),
      JS_FN("getQuest", trampoline<getQuest>, 2, JSPROP_ENUMERATE),
      JS_FN("getMinionCount", trampoline<getMinionCount>, 1, JSPROP_ENUMERATE),
      JS_FN("getRepairCost", trampoline<getRepairCost>, 1, JSPROP_ENUMERATE),
      JS_FN("getItemCost", trampoline<getItemCost>, 1, JSPROP_ENUMERATE),
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
