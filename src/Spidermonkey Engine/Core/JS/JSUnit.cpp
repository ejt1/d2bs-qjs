#include "JSUnit.h"

#include "Bindings.h"
#include "Core.h"
#include "CriticalSections.h"
#include "D2Helpers.h"
#include "Game/UI/NPCMenu.h"
#include "Game/D2Game.h"
#include "Game/D2Quests.h"
#include "Game/Unorganized.h"
#include "Globals.h"
#include "Localization.h"
#include "MPQStats.h"
#include "Script.h"

JSObject* UnitWrap::Instantiate(JSContext* ctx, UnitData* priv, bool isMe) {
  JS::RootedObject global(ctx, JS::CurrentGlobalOrNull(ctx));
  JS::RootedValue constructor_val(ctx);
  if (!JS_GetProperty(ctx, global, "Unit", &constructor_val)) {
    JS_ReportErrorASCII(ctx, "Could not find constructor object for Unit");
    return nullptr;
  }
  if (!constructor_val.isObject()) {
    JS_ReportErrorASCII(ctx, "Unit is not a constructor");
    return nullptr;
  }
  JS::RootedObject constructor(ctx, &constructor_val.toObject());

  JS::RootedObject obj(ctx, JS_New(ctx, constructor, JS::HandleValueArray::empty()));
  if (!obj) {
    JS_ReportErrorASCII(ctx, "Calling Unit constructor failed");
    return nullptr;
  }
  if (isMe) {
    // NOTE(ejt): hack to define additional properties for 'me'
    static JSPropertySpec me_props[] = {
        JS_PSG("account", trampoline<GetAccount>, JSPROP_ENUMERATE),
        JS_PSG("charname", trampoline<GetCharName>, JSPROP_ENUMERATE),
        JS_PSG("diff", trampoline<GetDiff>, JSPROP_ENUMERATE),
        JS_PSG("maxdiff", trampoline<GetDiffMax>, JSPROP_ENUMERATE),
        JS_PSG("gamename", trampoline<GetGameName>, JSPROP_ENUMERATE),
        JS_PSG("gamepassword", trampoline<GetGamePassword>, JSPROP_ENUMERATE),
        JS_PSG("gameserverip", trampoline<GetGameServerIp>, JSPROP_ENUMERATE),
        JS_PSG("gamestarttime", trampoline<GetGameStartTime>, JSPROP_ENUMERATE),
        JS_PSG("gametype", trampoline<GetGameType>, JSPROP_ENUMERATE),
        JS_PSG("itemoncursor", trampoline<GetItemOnCursor>, JSPROP_ENUMERATE),
        JS_PSGS("automap", trampoline<GetAutomap>, trampoline<SetAutomap>, JSPROP_ENUMERATE),
        JS_PSG("ladder", trampoline<GetLadder>, JSPROP_ENUMERATE),
        JS_PSG("ping", trampoline<GetPing>, JSPROP_ENUMERATE),
        JS_PSG("fps", trampoline<GetFPS>, JSPROP_ENUMERATE),
        JS_PSG("locale", trampoline<GetLocale>, JSPROP_ENUMERATE),
        JS_PSG("playertype", trampoline<GetPlayerType>, JSPROP_ENUMERATE),
        JS_PSG("realm", trampoline<GetRealm>, JSPROP_ENUMERATE),
        JS_PSG("realmshort", trampoline<GetRealmShort>, JSPROP_ENUMERATE),
        JS_PSG("mercrevivecost", trampoline<GetMercReviveCost>, JSPROP_ENUMERATE),
        JS_PSGS("runwalk", trampoline<GetRunWalk>, trampoline<SetRunWalk>, JSPROP_ENUMERATE),
        JS_PSG("weaponswitch", trampoline<GetWeaaponSwitch>, JSPROP_ENUMERATE),
        JS_PSGS("chickenhp", trampoline<GetChickenHP>, trampoline<SetChickenHP>, JSPROP_ENUMERATE),
        JS_PSGS("chickenmp", trampoline<GetChickenMP>, trampoline<SetChickenMP>, JSPROP_ENUMERATE),
        JS_PSGS("quitonhostile", trampoline<GetQuitOnHostile>, trampoline<SetQuitOnHostile>, JSPROP_ENUMERATE),
        JS_PSGS("blockKeys", trampoline<GetBlockKeys>, trampoline<SetBlockKeys>, JSPROP_ENUMERATE),
        JS_PSGS("blockMouse", trampoline<GetBlockMouse>, trampoline<SetBlockMouse>, JSPROP_ENUMERATE),
        JS_PSG("gameReady", trampoline<GetGameReady>, JSPROP_ENUMERATE),
        JS_PSG("profile", trampoline<GetProfile>, JSPROP_ENUMERATE),
        JS_PSGS("nopickup", trampoline<GetNoPickup>, trampoline<SetNoPickup>, JSPROP_ENUMERATE),
        JS_PSG("pid", trampoline<GetPID>, JSPROP_ENUMERATE),
        JS_PSG("unsupported", trampoline<GetUnsupported>, JSPROP_ENUMERATE),
        JS_PSG("charflags", trampoline<GetCharFlags>, JSPROP_ENUMERATE),
        JS_PSG("screensize", trampoline<GetScreenSize>, JSPROP_ENUMERATE),
        JS_PSG("windowtitle", trampoline<GetWindowTitle>, JSPROP_ENUMERATE),
        JS_PSG("ingame", trampoline<GetInGame>, JSPROP_ENUMERATE),
        JS_PSGS("quitonerror", trampoline<GetQuitOnError>, trampoline<SetQuitOnError>, JSPROP_ENUMERATE),
        JS_PSGS("maxgametime", trampoline<GetMaxGameTime>, trampoline<SetMaxGameTime>, JSPROP_ENUMERATE),
        JS_PSG("mapid", trampoline<GetMapId>, JSPROP_ENUMERATE),
        JS_PS_END,
    };
    JS_DefineProperties(ctx, obj, me_props);
  }
  UnitWrap* wrap = new UnitWrap(ctx, obj, priv);
  if (!wrap) {
    JS_ReportOutOfMemory(ctx);
    return nullptr;
  }
  return obj;
}

void UnitWrap::Initialize(JSContext* ctx, JS::HandleObject target) {
  JS::RootedObject proto(ctx, JS_InitClass(ctx, target, nullptr, &m_class, trampoline<New>, 0, m_unit_props, m_unit_methods, nullptr, nullptr));
  if (!proto) {
    Log("failed to initialize class Unit");
    return;
  }

  JS_DefineFunction(ctx, target, "getUnit", trampoline<getUnit>, 0, JSPROP_ENUMERATE);
  JS_DefineFunction(ctx, target, "getMercHP", trampoline<getMercHP>, 0, JSPROP_ENUMERATE);
}

UnitWrap::UnitData* UnitWrap::GetData() {
  return pData;
}

UnitWrap::UnitWrap(JSContext* ctx, JS::HandleObject obj, UnitData* priv) : BaseObject(ctx, obj), pData(priv) {
}

UnitWrap::~UnitWrap() {
  if (pData) {
    switch (pData->dwPrivateType) {
      case PRIVATE_UNIT: {
        delete pData;
        break;
      }
      case PRIVATE_ITEM: {
        ItemData* lpItem = static_cast<ItemData*>(pData);
        delete lpItem;
        break;
      }
    }
  }
}

void UnitWrap::finalize(JSFreeOp* /*fop*/, JSObject* obj) {
  BaseObject* wrap = BaseObject::FromJSObject(obj);
  if (wrap) {
    delete wrap;
  }
}

bool UnitWrap::New(JSContext* ctx, JS::CallArgs& args) {
  JS::RootedObject newObject(ctx, JS_NewObjectForConstructor(ctx, &m_class, args));
  if (!newObject) {
    THROW_ERROR(ctx, "failed to construct Unit");
  }
  // BUG(ejt): ask ejt
  JS_DefineProperties(ctx, newObject, m_unit_props);

  args.rval().setObject(*newObject);
  return true;
}

// properties
bool UnitWrap::GetType(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, &wrap, args.thisv());
  args.rval().setNumber(pUnit->dwType);
  return true;
}

bool UnitWrap::GetClassID(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, &wrap, args.thisv());
  args.rval().setNumber(pUnit->dwTxtFileNo);
  return true;
}

bool UnitWrap::GetMode(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, &wrap, args.thisv());
  args.rval().setNumber(pUnit->dwMode);
  return true;
}

bool UnitWrap::GetName(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, &wrap, args.thisv());
  char tmp[128]{};
  pUnit->GetUnitName(tmp, 128);
  args.rval().setString(JS_NewStringCopyZ(ctx, tmp));
  return true;
}

bool UnitWrap::GetAct(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  args.rval().setNumber(pUnit->dwAct + 1);
  return true;
}

bool UnitWrap::GetGid(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  args.rval().setNumber(pUnit->dwUnitId);
  return true;
}

bool UnitWrap::GetX(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  args.rval().setInt32(D2CLIENT_GetUnitX(pUnit));
  return true;
}

bool UnitWrap::GetY(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  args.rval().setInt32(D2CLIENT_GetUnitY(pUnit));
  return true;
}

bool UnitWrap::GetTargetX(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  switch (pUnit->dwType) {
    case 0:
    case 1:
    case 3:
      args.rval().setInt32(pUnit->pPath->xTarget);
      return true;
  }
  args.rval().setUndefined();
  return true;
}

bool UnitWrap::GetTargetY(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  switch (pUnit->dwType) {
    case 0:
    case 1:
    case 3:
      args.rval().setInt32(pUnit->pPath->yTarget);
      return true;
  }
  args.rval().setUndefined();
  return true;
}

bool UnitWrap::GetArea(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  auto pRoom = D2COMMON_GetRoomFromUnit(pUnit);
  if (pRoom && pRoom->pRoom2 && pRoom->pRoom2->pLevel) {
    args.rval().setNumber(pRoom->pRoom2->pLevel->dwLevelNo);
    return true;
  }
  args.rval().setUndefined();
  return true;
}

bool UnitWrap::GetHP(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  args.rval().setInt32(D2COMMON_GetUnitStat(pUnit, 6, 0) >> 8);
  return true;
}

bool UnitWrap::GetHPMax(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  args.rval().setInt32(D2COMMON_GetUnitStat(pUnit, 7, 0) >> 8);
  return true;
}

bool UnitWrap::GetMP(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  args.rval().setInt32(D2COMMON_GetUnitStat(pUnit, 8, 0) >> 8);
  return true;
}

bool UnitWrap::GetMPMax(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  args.rval().setInt32(D2COMMON_GetUnitStat(pUnit, 9, 0) >> 8);
  return true;
}

bool UnitWrap::GetStamina(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  args.rval().setInt32(D2COMMON_GetUnitStat(pUnit, 10, 0) >> 8);
  return true;
}

bool UnitWrap::GetStaminaMax(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  args.rval().setInt32(D2COMMON_GetUnitStat(pUnit, 11, 0) >> 8);
  return true;
}

bool UnitWrap::GetCharLevel(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  args.rval().setInt32(D2COMMON_GetUnitStat(pUnit, 12, 0));
  return true;
}

bool UnitWrap::GetItemCount(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (!pUnit->pInventory) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setInt32(pUnit->pInventory->dwItemCount);
  return true;
}

bool UnitWrap::GetOwner(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  args.rval().setInt32(pUnit->dwOwnerId);
  return true;
}

bool UnitWrap::GetOwnerType(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  args.rval().setInt32(pUnit->dwOwnerType);
  return true;
}

bool UnitWrap::GetSpecType(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  uint32_t specType = NULL;
  if (pUnit->dwType == UNIT_MONSTER && pUnit->pMonsterData) {
    if (pUnit->pMonsterData->UFlags.fMinion & 1)
      specType |= 0x08;
    if (pUnit->pMonsterData->UFlags.fBoss & 1)
      specType |= 0x04;
    if (pUnit->pMonsterData->UFlags.fChamp & 1)
      specType |= 0x02;
    if ((pUnit->pMonsterData->UFlags.fBoss & 1) && (pUnit->pMonsterData->UFlags.fNormal & 1))
      specType |= 0x01;
    if (pUnit->pMonsterData->UFlags.fNormal & 1)
      specType |= 0x00;
    args.rval().setInt32(specType);
    return true;
  }
  args.rval().setUndefined();
  return true;
}

bool UnitWrap::GetDirection(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (!pUnit->pPath || !pUnit->pPath->pRoom1) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setInt32(pUnit->pPath->bDirection);
  return true;
}

bool UnitWrap::GetUniqueId(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType == UNIT_MONSTER && pUnit->pMonsterData->UFlags.fBoss && pUnit->pMonsterData->UFlags.fNormal) {
    args.rval().setInt32(pUnit->pMonsterData->wUniqueNo);
    return true;
  }
  args.rval().setInt32(-1);
  return true;
}

bool UnitWrap::GetCode(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_ITEM || !pUnit->pItemData) {
    args.rval().setUndefined();
    return true;
  }
  D2ItemsTxt* pTxt = D2COMMON_GetItemText(pUnit->dwTxtFileNo);
  if (!pTxt) {
    args.rval().setString(JS_NewStringCopyZ(ctx, "Unknown"));
    return true;
  }
  char szCode[4]{};
  memcpy(szCode, pTxt->szCode, 3);
  args.rval().setString(JS_NewStringCopyZ(ctx, szCode));
  return true;
}

bool UnitWrap::GetPrefix(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_ITEM || !pUnit->pItemData) {
    args.rval().setUndefined();
    return true;
  }
  char* str = D2COMMON_GetItemMagicalMods(pUnit->pItemData->wMagicPrefix[0]);
  if (!str) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setString(JS_NewStringCopyZ(ctx, str));
  return true;
}

bool UnitWrap::GetSuffix(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_ITEM || !pUnit->pItemData) {
    args.rval().setUndefined();
    return true;
  }
  char* str = D2COMMON_GetItemMagicalMods(pUnit->pItemData->wMagicSuffix[0]);
  if (!str) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setString(JS_NewStringCopyZ(ctx, str));
  return true;
}

bool UnitWrap::GetPrefixes(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_ITEM || !pUnit->pItemData) {
    args.rval().setUndefined();
    return true;
  }
  JS::RootedObject arr(ctx, JS_NewArrayObject(ctx, 3));
  for (int i = 0; i < 3; ++i) {
    char* str = D2COMMON_GetItemMagicalMods(pUnit->pItemData->wMagicPrefix[i]);
    if (str) {
      JS::RootedString str_val(ctx, JS_NewStringCopyZ(ctx, str));
      JS_SetElement(ctx, arr, i, str_val);
    }
  }
  args.rval().setObject(*arr);
  return true;
}

bool UnitWrap::GetSuffixes(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_ITEM || !pUnit->pItemData) {
    args.rval().setUndefined();
    return true;
  }
  JS::RootedObject arr(ctx, JS_NewArrayObject(ctx, 3));
  for (int i = 0; i < 3; ++i) {
    char* str = D2COMMON_GetItemMagicalMods(pUnit->pItemData->wMagicSuffix[i]);
    if (str) {
      JS::RootedString str_val(ctx, JS_NewStringCopyZ(ctx, str));
      JS_SetElement(ctx, arr, i, str_val);
    }
  }
  args.rval().setObject(*arr);
  return true;
}

bool UnitWrap::GetPrefixNum(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_ITEM || !pUnit->pItemData) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setInt32(pUnit->pItemData->wMagicPrefix[0]);
  return true;
}

bool UnitWrap::GetSuffixNum(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_ITEM || !pUnit->pItemData) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setInt32(pUnit->pItemData->wMagicSuffix[0]);
  return true;
}

bool UnitWrap::GetPrefixNums(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_ITEM || !pUnit->pItemData) {
    args.rval().setUndefined();
    return true;
  }
  JS::RootedObject arr(ctx, JS_NewArrayObject(ctx, 3));
  for (int i = 0; i < 3; ++i) {
    if (pUnit->pItemData->wMagicPrefix[i]) {
      JS_SetElement(ctx, arr, i, pUnit->pItemData->wMagicPrefix[i]);
    }
  }
  args.rval().setObject(*arr);
  return true;
}

bool UnitWrap::GetSuffixNums(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_ITEM || !pUnit->pItemData) {
    args.rval().setUndefined();
    return true;
  }
  JS::RootedObject arr(ctx, JS_NewArrayObject(ctx, 3));
  for (int i = 0; i < 3; ++i) {
    if (pUnit->pItemData->wMagicSuffix[i]) {
      JS_SetElement(ctx, arr, i, pUnit->pItemData->wMagicSuffix[i]);
    }
  }
  args.rval().setObject(*arr);
  return true;
}

bool UnitWrap::GetFName(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_ITEM || !pUnit->pItemData) {
    args.rval().setUndefined();
    return true;
  }
  wchar_t szfname[256]{};
  D2CLIENT_GetItemName(pUnit, szfname, 256);
  args.rval().setString(JS_NewUCStringCopyZ(ctx, reinterpret_cast<const char16_t*>(szfname)));
  return true;
}

bool UnitWrap::GetQuality(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_ITEM || !pUnit->pItemData) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setInt32(pUnit->pItemData->dwQuality);
  return true;
}

bool UnitWrap::GetNode(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_ITEM || !pUnit->pItemData) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setInt32(pUnit->pItemData->NodePage);
  return true;
}

bool UnitWrap::GetLocation(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_ITEM || !pUnit->pItemData) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setInt32(pUnit->pItemData->GameLocation);
  return true;
}

bool UnitWrap::GetSizeX(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_ITEM || !pUnit->pItemData) {
    args.rval().setUndefined();
    return true;
  }
  D2ItemsTxt* pTxt = D2COMMON_GetItemText(pUnit->dwTxtFileNo);
  if (!pTxt) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setInt32(pTxt->xSize);
  return true;
}

bool UnitWrap::GetSizeY(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_ITEM || !pUnit->pItemData) {
    args.rval().setUndefined();
    return true;
  }
  D2ItemsTxt* pTxt = D2COMMON_GetItemText(pUnit->dwTxtFileNo);
  if (!pTxt) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setInt32(pTxt->ySize);
  return true;
}

bool UnitWrap::GetItemType(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_ITEM || !pUnit->pItemData) {
    args.rval().setUndefined();
    return true;
  }
  D2ItemsTxt* pTxt = D2COMMON_GetItemText(pUnit->dwTxtFileNo);
  if (!pTxt) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setInt32(pTxt->nType);
  return true;
}

bool UnitWrap::GetDescription(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_ITEM || !pUnit->pItemData || !pUnit->pItemData->pOwnerInventory || !pUnit->pItemData->pOwnerInventory->pOwner) {
    args.rval().setUndefined();
    return true;
  }
  AutoCriticalRoom cRoom;

  wchar_t wBuffer[2048] = L"";
  wchar_t bBuffer[1] = {1};
  // TODO(ejt): rewrite this to use defined offsets instead then remove ReadProcessBYTES from D2Helpers.h/cpp
  size_t base = reinterpret_cast<size_t>(GetModuleHandle(nullptr));
  ::WriteProcessMemory(GetCurrentProcess(), (void*)(base + 0x7BCBE8 - 0x400000), bBuffer, 1, NULL);
  ::WriteProcessMemory(GetCurrentProcess(), (void*)(base + 0x7BCBF4 - 0x400000), &pUnit, 4, NULL);

  // D2CLIENT_LoadItemDesc(D2CLIENT_GetPlayerUnit(), 0);
  D2CLIENT_LoadItemDesc(pUnit->pItemData->pOwnerInventory->pOwner, 0);
  ReadProcessBYTES(GetCurrentProcess(), (base + 0x841EC8 - 0x400000), wBuffer, 2047);
  if (wcslen(wBuffer) > 0) {
    args.rval().setString(JS_NewUCStringCopyZ(ctx, reinterpret_cast<const char16_t*>(wBuffer)));
    return true;
  }
  args.rval().setUndefined();
  return true;
}

bool UnitWrap::GetBodyLocation(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_ITEM || !pUnit->pItemData) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setInt32(pUnit->pItemData->BodyLocation);
  return true;
}

bool UnitWrap::GetItemLevel(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_ITEM || !pUnit->pItemData) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setInt32(pUnit->pItemData->dwItemLevel);
  return true;
}

bool UnitWrap::GetLevelRequirement(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_ITEM || !pUnit->pItemData) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setInt32(D2COMMON_GetItemLevelRequirement(pUnit, D2CLIENT_GetPlayerUnit()));
  return true;
}

bool UnitWrap::GetGfx(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_ITEM || !pUnit->pItemData) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setInt32(pUnit->pItemData->bInvGfxIdx);
  return true;
}

bool UnitWrap::GetRunWalk(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit == D2CLIENT_GetPlayerUnit()) {
    args.rval().setInt32(*D2CLIENT_AlwaysRun);
    return true;
  }
  args.rval().setUndefined();
  return true;
}

bool UnitWrap::SetRunWalk(JSContext* ctx, JS::CallArgs& args) {
  if (!args.requireAtLeast(ctx, "runwalk", 1)) {
    return false;
  }
  bool val = JS::ToBoolean(args[0]);
  *D2CLIENT_AlwaysRun = val;
  return true;
}

bool UnitWrap::GetWeaaponSwitch(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit != D2CLIENT_GetPlayerUnit()) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setInt32(*D2CLIENT_bWeapSwitch);
  return true;
}

bool UnitWrap::GetObjType(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_OBJECT || !pUnit->pObjectData) {
    args.rval().setUndefined();
    return true;
  }
  D2ActiveRoomStrc* pRoom = D2COMMON_GetRoomFromUnit(pUnit);
  if (pRoom || D2COMMON_GetLevelNoFromRoom(pRoom)) {
    args.rval().setInt32(pUnit->pObjectData->UData.Type & 255);
  } else {
    args.rval().setInt32(pUnit->pObjectData->UData.Type);
  }
  return true;
}

bool UnitWrap::GetIsLocked(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  if (pUnit->dwType != UNIT_OBJECT || !pUnit->pObjectData) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setInt32(pUnit->pObjectData->UData.UFlags.ChestLocked);
  return true;
}

// properties for 'me'
bool UnitWrap::GetAccount(JSContext* ctx, JS::CallArgs& args) {
  BnetData* pData = *D2LAUNCH_BnData;
  if (!pData) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setString(JS_NewStringCopyZ(ctx, pData->szAccountName));
  return true;
}

bool UnitWrap::GetCharName(JSContext* ctx, JS::CallArgs& args) {
  BnetData* pData = *D2LAUNCH_BnData;
  if (!pData) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setString(JS_NewStringCopyZ(ctx, pData->szPlayerName));
  return true;
}

bool UnitWrap::GetDiff(JSContext* /*ctx*/, JS::CallArgs& args) {
  args.rval().setInt32(D2CLIENT_GetDifficulty());
  return true;
}

bool UnitWrap::GetDiffMax(JSContext* /*ctx*/, JS::CallArgs& args) {
  BnetData* pData = *D2LAUNCH_BnData;
  if (!pData) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setInt32(pData->nMaxDiff);
  return true;
}

bool UnitWrap::GetGameName(JSContext* ctx, JS::CallArgs& args) {
  D2GameInfoStrc* pInfo = *D2CLIENT_GameInfo;
  if (!pInfo) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setString(JS_NewStringCopyZ(ctx, pInfo->szGameName));
  return true;
}

bool UnitWrap::GetGamePassword(JSContext* ctx, JS::CallArgs& args) {
  D2GameInfoStrc* pInfo = *D2CLIENT_GameInfo;
  if (!pInfo) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setString(JS_NewStringCopyZ(ctx, pInfo->szGamePassword));
  return true;
}

bool UnitWrap::GetGameServerIp(JSContext* ctx, JS::CallArgs& args) {
  D2GameInfoStrc* pInfo = *D2CLIENT_GameInfo;
  if (!pInfo) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setString(JS_NewStringCopyZ(ctx, pInfo->szGameServerIp));
  return true;
}

bool UnitWrap::GetGameStartTime(JSContext* /*ctx*/, JS::CallArgs& args) {
  args.rval().setNumber(static_cast<uint32_t>(Vars.dwGameTime));
  return true;
}

bool UnitWrap::GetGameType(JSContext* /*ctx*/, JS::CallArgs& args) {
  args.rval().setInt32(*D2CLIENT_ExpCharFlag);
  return true;
}

bool UnitWrap::GetItemOnCursor(JSContext* /*ctx*/, JS::CallArgs& args) {
  args.rval().setBoolean(static_cast<bool>(D2CLIENT_GetCursorItem()));
  return true;
}

bool UnitWrap::GetAutomap(JSContext* /*ctx*/, JS::CallArgs& args) {
  args.rval().setBoolean(static_cast<bool>(*D2CLIENT_AutomapOn));
  return true;
}

bool UnitWrap::SetAutomap(JSContext* ctx, JS::CallArgs& args) {
  if (!args.requireAtLeast(ctx, "automap", 1)) {
    return false;
  }
  bool val = JS::ToBoolean(args[0]);
  *D2CLIENT_AutomapOn = val;
  return true;
}

bool UnitWrap::GetLadder(JSContext* /*ctx*/, JS::CallArgs& args) {
  BnetData* pData = *D2LAUNCH_BnData;
  if (!pData) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setNumber(static_cast<uint32_t>(pData->ladderflag));
  return true;
}

bool UnitWrap::GetPing(JSContext* /*ctx*/, JS::CallArgs& args) {
  args.rval().setInt32(*D2CLIENT_Ping);
  return true;
}

bool UnitWrap::GetFPS(JSContext* /*ctx*/, JS::CallArgs& args) {
  args.rval().setInt32(*D2CLIENT_FPS);
  return true;
}

bool UnitWrap::GetLocale(JSContext* /*ctx*/, JS::CallArgs& args) {
  args.rval().setInt32(*D2CLIENT_Lang);
  return true;
}

bool UnitWrap::GetPlayerType(JSContext* /*ctx*/, JS::CallArgs& args) {
  BnetData* pData = *D2LAUNCH_BnData;
  if (!pData) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setBoolean(static_cast<bool>(pData->nCharFlags & PLAYER_TYPE_HARDCORE));
  return true;
}

bool UnitWrap::GetRealm(JSContext* ctx, JS::CallArgs& args) {
  BnetData* pData = *D2LAUNCH_BnData;
  if (!pData) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setString(JS_NewStringCopyZ(ctx, pData->szRealmName));
  return true;
}

bool UnitWrap::GetRealmShort(JSContext* ctx, JS::CallArgs& args) {
  BnetData* pData = *D2LAUNCH_BnData;
  if (!pData) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setString(JS_NewStringCopyZ(ctx, pData->szRealmName2));
  return true;
}

bool UnitWrap::GetMercReviveCost(JSContext* /*ctx*/, JS::CallArgs& args) {
  args.rval().setInt32(*D2CLIENT_MercReviveCost);
  return true;
}

bool UnitWrap::GetChickenHP(JSContext* /*ctx*/, JS::CallArgs& args) {
  args.rval().setInt32(Vars.nChickenHP);
  return true;
}

bool UnitWrap::SetChickenHP(JSContext* ctx, JS::CallArgs& args) {
  if (!args.requireAtLeast(ctx, "chickenhp", 1)) {
    return false;
  }
  int32_t val = args[0].toInt32();
  Vars.nChickenHP = val;
  return true;
}

bool UnitWrap::GetChickenMP(JSContext* /*ctx*/, JS::CallArgs& args) {
  args.rval().setInt32(Vars.nChickenMP);
  return true;
}

bool UnitWrap::SetChickenMP(JSContext* ctx, JS::CallArgs& args) {
  if (!args.requireAtLeast(ctx, "chickenmp", 1)) {
    return false;
  }
  int32_t val = args[0].toInt32();
  Vars.nChickenHP = val;
  return true;
}

bool UnitWrap::GetQuitOnHostile(JSContext* /*ctx*/, JS::CallArgs& args) {
  args.rval().setBoolean(static_cast<bool>(Vars.bQuitOnHostile));
  return true;
}

bool UnitWrap::SetQuitOnHostile(JSContext* ctx, JS::CallArgs& args) {
  if (!args.requireAtLeast(ctx, "quitonhostile", 1)) {
    return false;
  }
  bool val = JS::ToBoolean(args[0]);
  Vars.bQuitOnHostile = val;
  return true;
}

bool UnitWrap::GetBlockKeys(JSContext* /*ctx*/, JS::CallArgs& args) {
  args.rval().setBoolean(Vars.bBlockKeys);
  return true;
}

bool UnitWrap::SetBlockKeys(JSContext* ctx, JS::CallArgs& args) {
  if (!args.requireAtLeast(ctx, "blockkeys", 1)) {
    return false;
  }
  bool val = JS::ToBoolean(args[0]);
  Vars.bBlockKeys = val;
  return true;
}

bool UnitWrap::GetBlockMouse(JSContext* /*ctx*/, JS::CallArgs& args) {
  args.rval().setBoolean(Vars.bBlockMouse);
  return true;
}

bool UnitWrap::SetBlockMouse(JSContext* ctx, JS::CallArgs& args) {
  if (!args.requireAtLeast(ctx, "blockmouse", 1)) {
    return false;
  }
  bool val = JS::ToBoolean(args[0]);
  Vars.bBlockMouse = val;
  return true;
}

bool UnitWrap::GetGameReady(JSContext* /*ctx*/, JS::CallArgs& args) {
  args.rval().setBoolean(GameReady());
  return true;
}

bool UnitWrap::GetProfile(JSContext* ctx, JS::CallArgs& args) {
  args.rval().setString(JS_NewStringCopyZ(ctx, Vars.szProfile));
  return true;
}

bool UnitWrap::GetNoPickup(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  args.rval().setBoolean(static_cast<bool>(*D2CLIENT_NoPickUp));
  return true;
}

bool UnitWrap::SetNoPickup(JSContext* ctx, JS::CallArgs& args) {
  if (!args.requireAtLeast(ctx, "maxgametime", 1)) {
    return false;
  }
  bool val = JS::ToBoolean(args[0]);
  *D2CLIENT_NoPickUp = val;
  return true;
}

bool UnitWrap::GetPID(JSContext* /*ctx*/, JS::CallArgs& args) {
  args.rval().setNumber(static_cast<double>(GetCurrentProcessId()));
  return true;
}

bool UnitWrap::GetUnsupported(JSContext* /*ctx*/, JS::CallArgs& args) {
  args.rval().setBoolean(Vars.bEnableUnsupported);
  return true;
}

bool UnitWrap::GetCharFlags(JSContext* /*ctx*/, JS::CallArgs& args) {
  BnetData* pData = *D2LAUNCH_BnData;
  if (!pData) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setInt32(pData->nCharFlags);
  return true;
}

bool UnitWrap::GetScreenSize(JSContext* /*ctx*/, JS::CallArgs& args) {
  args.rval().setInt32(D2GFX_GetScreenSize());
  return true;
}

bool UnitWrap::GetWindowTitle(JSContext* ctx, JS::CallArgs& args) {
  char szTitle[256]{};
  GetWindowText(D2GFX_GetHwnd(), szTitle, 256);
  args.rval().setString(JS_NewStringCopyZ(ctx, szTitle));
  return true;
}

bool UnitWrap::GetInGame(JSContext* /*ctx*/, JS::CallArgs& args) {
  args.rval().setBoolean(ClientState() == ClientStateInGame);
  return true;
}

bool UnitWrap::GetQuitOnError(JSContext* /*ctx*/, JS::CallArgs& args) {
  args.rval().setBoolean(Vars.bQuitOnError);
  return true;
}

bool UnitWrap::SetQuitOnError(JSContext* ctx, JS::CallArgs& args) {
  if (!args.requireAtLeast(ctx, "quitonerror", 1)) {
    return false;
  }
  bool val = JS::ToBoolean(args[0]);
  Vars.bQuitOnError = val;
  return true;
}

bool UnitWrap::GetMaxGameTime(JSContext* /*ctx*/, JS::CallArgs& args) {
  args.rval().setInt32(Vars.dwMaxGameTime);
  return true;
}

bool UnitWrap::SetMaxGameTime(JSContext* ctx, JS::CallArgs& args) {
  if (!args.requireAtLeast(ctx, "maxgametime", 1)) {
    return false;
  }
  int32_t val = args[0].toInt32();
  Vars.dwMaxGameTime = val;
  return true;
}

bool UnitWrap::GetMapId(JSContext* ctx, JS::CallArgs& args) {
  UNWRAP_UNIT_OR_RETURN(ctx, 0, args.thisv());
  args.rval().setInt32(*D2CLIENT_MapId);
  return true;
}

// function
bool UnitWrap::getNext(JSContext* ctx, JS::CallArgs& args) {
  UnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  UnitData* unit = wrap->pData;
  if (!unit)
    THROW_ERROR(ctx, "invalid unit");

  if (unit->dwPrivateType == PRIVATE_UNIT) {
    UnitData* lpUnit = (UnitData*)unit;
    D2UnitStrc* pUnit = D2UnitStrc::FindUnit(lpUnit->dwUnitId, lpUnit->dwType);
    if (!pUnit) {
      args.rval().setUndefined();
      return true;
    }
    if (args.length() > 0 && args[0].isString()) {
      char* szText = JS_EncodeString(ctx, args[0].toString());
      strcpy_s(lpUnit->szName, 128, szText);
      JS_free(ctx, szText);
    }

    if (args.length() > 0 && args[0].isNumber() && !args[1].isNull())
      JS::ToUint32(ctx, args[0], (uint32_t*)&(lpUnit->dwClassId));

    if (args.length() > 1 && args[1].isNumber() && !args[2].isNull())
      JS::ToUint32(ctx, args[1], (uint32_t*)&(lpUnit->dwMode));

    pUnit = pUnit->GetNext(lpUnit->szName, lpUnit->dwClassId, lpUnit->dwType, lpUnit->dwMode);
    if (!pUnit) {
      args.rval().setBoolean(false);
      return true;
    } else {
      lpUnit->dwUnitId = pUnit->dwUnitId;
      args.rval().setBoolean(true);
      return true;
    }
  } else if (unit->dwPrivateType == PRIVATE_ITEM) {
    ItemData* pmyUnit = (ItemData*)unit;
    if (!pmyUnit) {
      args.rval().setUndefined();
      return true;
    }

    D2UnitStrc* pUnit = D2UnitStrc::FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);
    D2UnitStrc* pOwner = D2UnitStrc::FindUnit(pmyUnit->dwOwnerId, pmyUnit->dwOwnerType);
    if (!pUnit || !pOwner) {
      args.rval().setUndefined();
      return true;
    }

    if (args.length() > 0 && args[0].isString()) {
      char* szText = JS_EncodeString(ctx, args[0].toString());
      strcpy_s(pmyUnit->szName, 128, szText);
      JS_free(ctx, szText);
    }

    if (args.length() > 0 && args[0].isNumber() && !args[1].isNull())
      JS::ToUint32(ctx, args[0], (uint32_t*)&(pmyUnit->dwClassId));

    if (args.length() > 1 && args[1].isNumber() && !args[2].isNull())
      JS::ToUint32(ctx, args[1], (uint32_t*)&(pmyUnit->dwMode));

    D2UnitStrc* nextItem = pUnit->GetNextItem(pOwner, pmyUnit->szName, pmyUnit->dwClassId, pmyUnit->dwMode);
    if (!nextItem) {
      args.rval().setBoolean(false);
      return true;
    } else {
      pmyUnit->dwUnitId = nextItem->dwUnitId;
      args.rval().setBoolean(true);
      return true;
    }
  }

  args.rval().setUndefined();
  return true;
}

bool UnitWrap::cancel(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  DWORD automapOn = *D2CLIENT_AutomapOn;
  int32_t mode = -1;

  if (args.length() > 0) {
    if (args[0].isNumber())
      JS::ToUint32(ctx, args[0], (uint32_t*)&mode);
  } else if (IsScrollingText())
    mode = 3;
  else if (D2CLIENT_GetCurrentInteractingNPC())
    mode = 2;
  else if (D2CLIENT_GetCursorItem())
    mode = 1;
  else
    mode = 0;

  switch (mode) {
    case 0:
      D2CLIENT_CloseInteract();
      break;
    case 1:
      D2CLIENT_ClickMap(0, 10, 10, 0x08);
      break;
    case 2:
      D2CLIENT_CloseNPCInteract();
      break;
    case 3:
      D2CLIENT_ClearScreen();
      break;
  }

  *D2CLIENT_AutomapOn = automapOn;
  args.rval().setBoolean(true);
  return true;
}

bool UnitWrap::repair(JSContext* ctx, JS::CallArgs& args) {
  UnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  UnitData* lpUnit = wrap->pData;
  if (!lpUnit || (lpUnit->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
    args.rval().setBoolean(false);
    return true;
  }

  D2UnitStrc* pUnit = D2UnitStrc::FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

  if (!pUnit) {
    args.rval().setBoolean(false);
    return true;
  }

  BYTE aPacket[17] = {NULL};
  aPacket[0] = 0x35;
  *(DWORD*)&aPacket[1] = *D2CLIENT_RecentInteractId;
  aPacket[16] = 0x80;
  D2NET_SendPacket(17, 1, aPacket);

  // note: this crashes while minimized
  //	D2CLIENT_PerformNpcAction(pUnit,1, NULL);

  args.rval().setBoolean(true);
  return true;
}

bool UnitWrap::useMenu(JSContext* ctx, JS::CallArgs& args) {
  UnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  UnitData* lpUnit = wrap->pData;
  if (args.length() < 1 || !args[0].isNumber()) {
    args.rval().setBoolean(false);
    return true;
  }

  if (!lpUnit || (lpUnit->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
    args.rval().setBoolean(false);
    return true;
  }

  D2UnitStrc* pUnit = D2UnitStrc::FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

  if (!pUnit) {
    args.rval().setBoolean(false);
    return true;
  }

  uint32_t menuId;
  JS::ToUint32(ctx, args[0], &menuId);
  args.rval().setBoolean(NPCMenu::Click(pUnit->dwTxtFileNo, menuId));
  return true;
}

bool UnitWrap::interact(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  UnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  UnitData* lpUnit = wrap->pData;
  if (!lpUnit || (lpUnit->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
    args.rval().setBoolean(false);
    return true;
  }

  D2UnitStrc* pUnit = D2UnitStrc::FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

  if (!pUnit || pUnit == D2CLIENT_GetPlayerUnit()) {
    args.rval().setBoolean(false);
    return true;
  }

  if (pUnit->dwType == UNIT_ITEM && pUnit->dwMode != ITEM_MODE_ON_GROUND && pUnit->dwMode != ITEM_MODE_BEING_DROPPED) {
    int nLocation = pUnit->GetItemLocation();

    BYTE aPacket[13] = {NULL};

    if (nLocation == LOCATION_INVENTORY || nLocation == LOCATION_STASH) {
      aPacket[0] = 0x20;
      *(DWORD*)&aPacket[1] = pUnit->dwUnitId;
      *(DWORD*)&aPacket[5] = D2CLIENT_GetPlayerUnit()->pPath->xPos;
      *(DWORD*)&aPacket[9] = D2CLIENT_GetPlayerUnit()->pPath->yPos;
      D2NET_SendPacket(13, 1, aPacket);
      args.rval().setBoolean(false);
      return true;
    } else if (nLocation == LOCATION_BELT) {
      aPacket[0] = 0x26;
      *(DWORD*)&aPacket[1] = pUnit->dwUnitId;
      *(DWORD*)&aPacket[5] = 0;
      *(DWORD*)&aPacket[9] = 0;
      D2NET_SendPacket(13, 1, aPacket);
      args.rval().setBoolean(false);
      return true;
    }
  }

  if (pUnit->dwType == UNIT_OBJECT && args.length() == 1 && args[0].isInt32()) {
    int32_t nWaypointID = args[0].toInt32();
    int retVal = 0;
    if (FillBaseStat("levels", nWaypointID, "Waypoint", &retVal, sizeof(int)))
      if (retVal == 255) {
        args.rval().setBoolean(false);
        return true;
      }

    D2CLIENT_TakeWaypoint(pUnit->dwUnitId, nWaypointID);
    if (!D2CLIENT_GetUIState(UI_GAME))
      D2CLIENT_CloseInteract();

    args.rval().setBoolean(true);
    return true;
  }
  //	else if(pUnit->dwType == UNIT_PLAYER && args.length() == 1 && JSVAL_IS_INT(argv[0]) && JSVAL_TO_INT(argv[0]) == 1)
  //	{
  // Accept Trade
  //	}

  ClickMap(0, D2CLIENT_GetUnitX(pUnit), D2CLIENT_GetUnitY(pUnit), FALSE, pUnit);
  // D2CLIENT_Interact(pUnit, 0x45);
  args.rval().setBoolean(true);
  return true;
}

bool UnitWrap::getItem(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  UnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  UnitData* pmyUnit = wrap->pData;
  if (!pmyUnit || (pmyUnit->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
    args.rval().setUndefined();
    return true;
  }

  D2UnitStrc* pUnit = D2UnitStrc::FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

  if (!pUnit || !pUnit->pInventory) {
    args.rval().setUndefined();
    return true;
  }

  uint32_t nClassId = (uint32_t)-1;
  uint32_t nMode = (uint32_t)-1;
  uint32_t nUnitId = (uint32_t)-1;
  char szName[128] = "";

  if (args.length() > 0 && args[0].isString()) {
    char* szText = JS_EncodeString(ctx, args[0].toString());
    strcpy_s(szName, sizeof(szName), szText);
    JS_free(ctx, szText);
  }

  if (args.length() > 0 && args[0].isNumber() && !args[0].isNull())
    JS::ToUint32(ctx, args[0], &nClassId);

  if (args.length() > 1 && args[1].isNumber() && !args[1].isNull())
    JS::ToUint32(ctx, args[1], &nMode);

  if (args.length() > 2 && args[2].isNumber() && !args[2].isNull())
    JS::ToUint32(ctx, args[2], &nUnitId);

  D2UnitStrc* pItem = pUnit->FindItem(szName, nClassId, nMode, nUnitId);
  if (!pItem) {
    args.rval().setUndefined();
    return true;
  }

  ItemData* pmyItem = new ItemData;  // leaked?
  if (!pmyItem) {
    args.rval().setUndefined();
    return true;
  }

  pmyItem->dwPrivateType = PRIVATE_ITEM;
  pmyItem->dwClassId = nClassId;
  pmyItem->dwMode = nMode;
  pmyItem->dwType = pItem->dwType;
  pmyItem->dwUnitId = pItem->dwUnitId;
  pmyItem->dwOwnerId = pmyUnit->dwUnitId;
  pmyItem->dwOwnerType = pmyUnit->dwType;
  strcpy_s(pmyItem->szName, sizeof(pmyItem->szName), szName);

  JS::RootedObject newObject(ctx, UnitWrap::Instantiate(ctx, pmyItem));
  if (!newObject) {
    THROW_ERROR(ctx, "failed to instantiate unit");
  }
  args.rval().setObject(*newObject);
  return true;
}

bool UnitWrap::getItems(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  UnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  UnitData* lpUnit = wrap->pData;
  if (!lpUnit || (lpUnit->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
    args.rval().setUndefined();
    return true;
  }

  D2UnitStrc* pUnit = D2UnitStrc::FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

  if (!pUnit || !pUnit->pInventory || !pUnit->pInventory->pFirstItem) {
    args.rval().setUndefined();
    return true;
  }

  JS::RootedObject pReturnArray(ctx, JS_NewArrayObject(ctx, 0));
  if (!pReturnArray) {
    args.rval().setUndefined();
    return true;
  }

  DWORD dwArrayCount = 0;

  for (D2UnitStrc* pItem = pUnit->pInventory->pFirstItem; pItem; pItem = D2COMMON_GetNextItemFromInventory(pItem), dwArrayCount++) {
    ItemData* pmyUnit = new ItemData;
    if (!pmyUnit)
      continue;

    pmyUnit->dwPrivateType = PRIVATE_UNIT;
    pmyUnit->szName[0] = NULL;
    pmyUnit->dwMode = pItem->dwMode;
    pmyUnit->dwClassId = pItem->dwTxtFileNo;
    pmyUnit->dwUnitId = pItem->dwUnitId;
    pmyUnit->dwType = UNIT_ITEM;
    pmyUnit->dwOwnerId = pUnit->dwUnitId;
    pmyUnit->dwOwnerType = pUnit->dwType;

    JS::RootedObject jsunit(ctx, UnitWrap::Instantiate(ctx, pmyUnit));
    if (!jsunit) {
      THROW_ERROR(ctx, "Failed to build item array");
    }
    JS_SetElement(ctx, pReturnArray, dwArrayCount, jsunit);
  }

  args.rval().setObject(*pReturnArray);
  return true;
}

bool UnitWrap::getMerc(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  UnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  UnitData* lpUnit = wrap->pData;
  if (lpUnit && (lpUnit->dwPrivateType & PRIVATE_UNIT) == PRIVATE_UNIT) {
    D2UnitStrc* pUnit = D2UnitStrc::FindUnit(lpUnit->dwUnitId, lpUnit->dwType);
    if (pUnit && pUnit->dwType == UNIT_PLAYER) {
      D2UnitStrc* pMerc = pUnit->FindMercUnit();
      if (pMerc) {
        UnitData* pmyUnit = new UnitData;

        pmyUnit->dwPrivateType = PRIVATE_UNIT;
        pmyUnit->dwUnitId = pMerc->dwUnitId;
        pmyUnit->dwClassId = pMerc->dwTxtFileNo;
        pmyUnit->dwMode = NULL;
        pmyUnit->dwType = UNIT_MONSTER;
        pmyUnit->szName[0] = NULL;

        JS::RootedObject newObject(ctx, UnitWrap::Instantiate(ctx, pmyUnit));
        if (!newObject) {
          THROW_ERROR(ctx, "failed to instantiate unit");
        }
        args.rval().setObject(*newObject);
        return true;
      }
    }
  }
  args.rval().setNull();
  return true;
}

bool UnitWrap::getMercHP(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");
  // this is stupid because getMercHP is both a method of Unit and a global function... FOR SOME REASON!
  D2UnitStrc* pUnit;
  if (args.thisv().isUndefined()) {
    pUnit = D2CLIENT_GetPlayerUnit();
  } else {
    UnitWrap* wrap;
    UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
    UnitData* lpUnit = wrap->pData;
    pUnit = D2UnitStrc::FindUnit(lpUnit->dwUnitId, lpUnit->dwType);
  }

  if (pUnit) {
    D2UnitStrc* pMerc = pUnit->FindMercUnit();
    if (pMerc) {
      args.rval().setInt32(pUnit->dwMode == 12 ? 0 : D2CLIENT_GetUnitHPPercent(pMerc->dwUnitId));
      return true;
    }
  }

  args.rval().setUndefined();
  return true;
}

bool UnitWrap::getSkill(JSContext* ctx, JS::CallArgs& args) {
  if (args.length() == NULL || args.length() > 3) {
    args.rval().setUndefined();
    return true;
  }

  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  bool nCharge = false;
  int32_t nSkillId = NULL;
  int32_t nExt = NULL;

  UnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  UnitData* pmyUnit = wrap->pData;
  if (!pmyUnit) {
    args.rval().setUndefined();
    return true;
  }

  D2UnitStrc* pUnit = D2UnitStrc::FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);
  if (!pUnit) {
    args.rval().setUndefined();
    return true;
  }

  if (args.length() >= 1) {
    if (!args[0].isInt32()) {
      args.rval().setUndefined();
      return true;
    }
    nSkillId = args[0].toInt32();
  }

  if (args.length() >= 2) {
    if (!args[1].isInt32()) {
      args.rval().setUndefined();
      return true;
    }

    nExt = args[1].toInt32();
  }

  if (args.length() == 3) {
    if (!args[2].isBoolean()) {
      args.rval().setUndefined();
      return true;
    }

    nCharge = JS::ToBoolean(args[2]);
  }

  if (args.length() == 1) {
    WORD wLeftSkillId = pUnit->pInfo->pLeftSkill->pSkillInfo->wSkillId;
    WORD wRightSkillId = pUnit->pInfo->pRightSkill->pSkillInfo->wSkillId;
    switch (nSkillId) {
      case 0: {
        int row = 0;
        if (FillBaseStat("skills", wRightSkillId, "skilldesc", &row, sizeof(int)))
          if (FillBaseStat("skilldesc", row, "str name", &row, sizeof(int))) {
            wchar_t* szName = D2LANG_GetLocaleText((WORD)row);
            args.rval().setString(JS_NewUCStringCopyZ(ctx, reinterpret_cast<const char16_t*>(szName)));
            return true;
          }
      } break;
      case 1: {
        int row = 0;
        if (FillBaseStat("skills", wLeftSkillId, "skilldesc", &row, sizeof(int)))
          if (FillBaseStat("skilldesc", row, "str name", &row, sizeof(int))) {
            wchar_t* szName = D2LANG_GetLocaleText((WORD)row);
            args.rval().setString(JS_NewUCStringCopyZ(ctx, reinterpret_cast<const char16_t*>(szName)));
            return true;
          }
      } break;

      case 2:
        args.rval().setInt32(wRightSkillId);
        return true;
        break;
      case 3:
        args.rval().setInt32(wLeftSkillId);
        return true;
        break;
      case 4: {
        JS::RootedObject pReturnArray(ctx, JS_NewArrayObject(ctx, 0));
        int i = 0;
        for (D2SkillStrc* pSkill = pUnit->pInfo->pFirstSkill; pSkill; pSkill = pSkill->pNextSkill) {
          JS::RootedObject pArrayInsert(ctx, JS_NewArrayObject(ctx, 0));
          if (!pArrayInsert)
            continue;

          JS_SetElement(ctx, pArrayInsert, 0, pSkill->pSkillInfo->wSkillId);
          JS_SetElement(ctx, pArrayInsert, 1, pSkill->dwSkillLevel);
          JS_SetElement(ctx, pArrayInsert, 2, D2COMMON_GetSkillLevel(pUnit, pSkill, 1));
          JS_SetElement(ctx, pReturnArray, i, pArrayInsert);
          i++;
        }
        args.rval().setObject(*pReturnArray);
        return true;
        break;
      }
      default:
        break;
    }
    args.rval().setBoolean(false);
    return true;
  }

  if (pUnit && pUnit->pInfo && pUnit->pInfo->pFirstSkill) {
    for (D2SkillStrc* pSkill = pUnit->pInfo->pFirstSkill; pSkill; pSkill = pSkill->pNextSkill) {
      if (pSkill->pSkillInfo && pSkill->pSkillInfo->wSkillId == nSkillId) {
        if ((args.length() == 2 && pSkill->IsCharge == 0) || (args.length() == 3 && (nCharge == false || pSkill->IsCharge == 1))) {
          args.rval().setInt32(D2COMMON_GetSkillLevel(pUnit, pSkill, nExt));
          return true;
        }
      }
    }
  }

  args.rval().setBoolean(false);
  return true;
}

bool UnitWrap::getParent(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  UnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  UnitData* lpUnit = wrap->pData;
  if (!lpUnit || (lpUnit->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
    args.rval().setUndefined();
    return true;
  }

  D2UnitStrc* pUnit = D2UnitStrc::FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

  if (!pUnit) {
    args.rval().setUndefined();
    return true;
  }

  if (pUnit->dwType == UNIT_MONSTER) {
    DWORD dwOwnerId = D2CLIENT_GetMonsterOwner(pUnit->dwUnitId);
    if (dwOwnerId == -1) {
      args.rval().setUndefined();
      return true;
    }

    D2UnitStrc* pMonster = D2UnitStrc::FindUnit(NULL, (DWORD)-1, (DWORD)-1, (DWORD)-1, dwOwnerId);
    if (!pMonster) {
      args.rval().setUndefined();
      return true;
    }

    UnitData* pmyUnit = new UnitData;
    if (!pmyUnit) {
      args.rval().setUndefined();
      return true;
    }

    pmyUnit->dwPrivateType = PRIVATE_UNIT;
    pmyUnit->dwUnitId = pMonster->dwUnitId;
    pmyUnit->dwClassId = pMonster->dwTxtFileNo;
    pmyUnit->dwMode = pMonster->dwMode;
    pmyUnit->dwType = pMonster->dwType;
    pmyUnit->szName[0] = NULL;

    JS::RootedObject newObject(ctx, UnitWrap::Instantiate(ctx, pmyUnit));
    if (!newObject) {
      THROW_ERROR(ctx, "failed to instantiate unit");
    }
    args.rval().setObject(*newObject);
    return true;
  } else if (pUnit->dwType == UNIT_OBJECT) {
    if (pUnit->pObjectData) {
      char szBuffer[128] = "";
      strcpy_s(szBuffer, sizeof(szBuffer), pUnit->pObjectData->szOwner);
      args.rval().setString(JS_NewStringCopyZ(ctx, szBuffer));
      return true;
    }
  } else if (pUnit->dwType == UNIT_ITEM) {
    if (pUnit->pItemData && pUnit->pItemData->pOwnerInventory && pUnit->pItemData->pOwnerInventory->pOwner) {
      UnitData* pmyUnit = new UnitData;  // leaks
      if (!pmyUnit) {
        args.rval().setUndefined();
        return true;
      }

      pmyUnit->dwPrivateType = PRIVATE_UNIT;
      pmyUnit->dwUnitId = pUnit->pItemData->pOwnerInventory->pOwner->dwUnitId;
      pmyUnit->dwClassId = pUnit->pItemData->pOwnerInventory->pOwner->dwTxtFileNo;
      pmyUnit->dwMode = pUnit->pItemData->pOwnerInventory->pOwner->dwMode;
      pmyUnit->dwType = pUnit->pItemData->pOwnerInventory->pOwner->dwType;
      pmyUnit->szName[0] = NULL;

      JS::RootedObject newObject(ctx, UnitWrap::Instantiate(ctx, pmyUnit));
      if (!newObject) {
        THROW_ERROR(ctx, "failed to instantiate unit");
      }
      args.rval().setObject(*newObject);
    }
  } else if (pUnit->dwType == UNIT_MISSILE) {
    auto* pmyUnit = new UnitData;
    if (!pmyUnit) {
      args.rval().setUndefined();
      return true;
    }

    D2UnitStrc* pOwner = D2COMMON_GetMissileOwnerUnit(pUnit);
    if (!pOwner) {
      args.rval().setUndefined();
      return true;
    }

    pmyUnit->dwPrivateType = PRIVATE_UNIT;
    pmyUnit->dwUnitId = pOwner->dwUnitId;
    pmyUnit->dwClassId = pOwner->dwTxtFileNo;
    pmyUnit->dwMode = pOwner->dwMode;
    pmyUnit->dwType = pOwner->dwType;
    pmyUnit->szName[0] = NULL;

    JS::RootedObject newObject(ctx, UnitWrap::Instantiate(ctx, pmyUnit));
    if (!newObject) {
      THROW_ERROR(ctx, "failed to instantiate unit");
    }
    args.rval().setObject(*newObject);
  }

  args.rval().setUndefined();
  return true;
}

bool UnitWrap::overhead(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  UnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  UnitData* pmyUnit = wrap->pData;
  if (!pmyUnit || (pmyUnit->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
    args.rval().setBoolean(false);
    return true;
  }

  D2UnitStrc* pUnit = D2UnitStrc::FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

  if (!pUnit) {
    args.rval().setBoolean(false);
    return true;
  }

  if (!args[0].isNullOrUndefined()) {
    char* lpszText = JS_EncodeString(ctx, args[0].toString());
    if (!lpszText) {
      THROW_ERROR(ctx, "failed to encode string");
    }
    std::string ansi = UTF8ToANSI(lpszText);
    D2HoverTextStrc* pMsg = D2COMMON_GenerateOverheadMsg(NULL, ansi.c_str(), *D2CLIENT_OverheadTrigger);
    if (pMsg) {
      pUnit->pOMsg = pMsg;
    }
    JS_free(ctx, lpszText);
  }

  args.rval().setBoolean(true);
  return true;
}

bool UnitWrap::revive(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  BYTE pPacket[] = {0x41};
  D2NET_SendPacket(1, 1, pPacket);
  args.rval().setUndefined();
  return true;
}

bool UnitWrap::getFlags(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  UnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  UnitData* lpUnit = wrap->pData;
  if (!lpUnit || (lpUnit->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
    args.rval().setUndefined();
    return true;
  }

  D2UnitStrc* pUnit = D2UnitStrc::FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

  if (!pUnit || pUnit->dwType != UNIT_ITEM) {
    args.rval().setUndefined();
    return true;
  }

  args.rval().setInt32(pUnit->pItemData->dwFlags);
  return true;
}

bool UnitWrap::getFlag(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  if (args.length() < 1 || !args[0].isInt32()) {
    args.rval().setUndefined();
    return true;
  }

  UnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  UnitData* lpUnit = wrap->pData;
  if (!lpUnit || (lpUnit->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
    args.rval().setUndefined();
    return true;
  }

  D2UnitStrc* pUnit = D2UnitStrc::FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

  if (!pUnit || pUnit->dwType != UNIT_ITEM) {
    args.rval().setUndefined();
    return true;
  }

  int32_t nFlag = args[0].toInt32();
  args.rval().setBoolean(!!(nFlag & pUnit->pItemData->dwFlags));
  return true;
}

void InsertStatsToGenericObject(D2UnitStrc* pUnit, D2StatListExStrc* pStatList, JSContext* pJSContext, JS::MutableHandleObject pGenericObject);
void InsertStatsNow(D2StatStrc* pStat, int nStat, JSContext* cx, JS::MutableHandleObject pArray);

bool UnitWrap::getStat(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  UnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  UnitData* lpUnit = wrap->pData;
  if (!lpUnit || (lpUnit->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
    args.rval().setBoolean(false);
    return true;
  }

  D2UnitStrc* pUnit = D2UnitStrc::FindUnit(lpUnit->dwUnitId, lpUnit->dwType);
  if (!pUnit) {
    args.rval().setBoolean(false);
    return true;
  }

  int32_t nStat = 0;
  int32_t nSubIndex = 0;
  if (args.length() == 0) {
    THROW_ERROR(ctx, "not enough arguments");
  }
  if (args[0].isInt32()) {
    nStat = args[0].toInt32();
  }

  if (args.length() > 1 && args[1].isInt32()) {
    nSubIndex = args[1].toInt32();
  }
  if (nStat >= STAT_HP && nStat <= STAT_MAXSTAMINA) {
    args.rval().setInt32(D2COMMON_GetUnitStat(pUnit, nStat, nSubIndex) >> 8);
    return true;
  } else if (nStat == STAT_EXP || nStat == STAT_LASTEXP || nStat == STAT_NEXTEXP) {
    args.rval().setNumber((unsigned int)D2COMMON_GetUnitStat(pUnit, nStat, nSubIndex));
    return true;
  } else if (nStat == STAT_ITEMLEVELREQ) {
    args.rval().setInt32(D2COMMON_GetItemLevelRequirement(pUnit, D2CLIENT_GetPlayerUnit()));
    return true;
  } else if (nStat == -1) {
    D2StatStrc aStatList[256] = {NULL};
    D2StatListExStrc* pStatList = D2COMMON_GetStatList(pUnit, NULL, 0x40);

    if (pStatList) {
      DWORD dwStats = D2COMMON_CopyStatList(pStatList, (D2StatStrc*)aStatList, 256);

      JS::RootedObject statArray(ctx, JS_NewArrayObject(ctx, 0));
      for (int j = 0; j < pUnit->pStats->StatVec.wCount; j++) {
        bool inListAlready = false;
        for (DWORD k = 0; k < dwStats; k++) {
          if (aStatList[k].dwStatValue == pUnit->pStats->StatVec.pStats[j].dwStatValue && aStatList[k].wStatIndex == pUnit->pStats->StatVec.pStats[j].wStatIndex &&
              aStatList[k].wSubIndex == pUnit->pStats->StatVec.pStats[j].wSubIndex)
            inListAlready = true;
        }
        if (!inListAlready) {
          aStatList[dwStats].dwStatValue = pUnit->pStats->StatVec.pStats[j].dwStatValue;
          aStatList[dwStats].wStatIndex = pUnit->pStats->StatVec.pStats[j].wStatIndex;
          aStatList[dwStats].wSubIndex = pUnit->pStats->StatVec.pStats[j].wSubIndex;
          dwStats++;
        }
      }
      for (UINT i = 0; i < dwStats; i++) {
        JS::RootedObject pArrayInsert(ctx, JS_NewArrayObject(ctx, 0));
        if (!pArrayInsert)
          continue;

        JS_SetElement(ctx, pArrayInsert, 0, aStatList[i].wStatIndex);
        JS_SetElement(ctx, pArrayInsert, 1, aStatList[i].wSubIndex);
        JS_SetElement(ctx, pArrayInsert, 2, aStatList[i].dwStatValue);

        JS_SetElement(ctx, statArray, i, pArrayInsert);
      }

      args.rval().setObject(*statArray);
      return true;
    }

    // TODO(ejt): does this ever occur?
    args.rval().setBoolean(false);
    return true;
  } else if (nStat == -2) {
    JS::RootedObject statArray(ctx, JS_NewArrayObject(ctx, 0));

    InsertStatsToGenericObject(pUnit, pUnit->pStats, ctx, &statArray);
    InsertStatsToGenericObject(pUnit, D2COMMON_GetStatList(pUnit, NULL, 0x40), ctx, &statArray);

    args.rval().setObject(*statArray);
    return true;
  }

  long result = D2COMMON_GetUnitStat(pUnit, nStat, nSubIndex);
  if (result == 0)  // if stat isnt found look up preset list
  {
    D2StatListExStrc* pStatList = D2COMMON_GetStatList(pUnit, NULL, 0x40);
    D2StatStrc aStatList[256] = {NULL};
    if (pStatList) {
      DWORD dwStats = D2COMMON_CopyStatList(pStatList, (D2StatStrc*)aStatList, 256);
      for (UINT i = 0; i < dwStats; i++) {
        if (nStat == aStatList[i].wStatIndex && nSubIndex == aStatList[i].wSubIndex)
          result = (aStatList[i].dwStatValue);
      }
    }
  }

  args.rval().setNumber((double)result);
  return true;
}

void InsertStatsToGenericObject(D2UnitStrc* pUnit, D2StatListExStrc* pStatList, JSContext* cx, JS::MutableHandleObject pArray) {
  D2StatStrc* pStat = NULL;
  if (!pStatList)
    return;
  if ((pStatList->dwUnitId == pUnit->dwUnitId && pStatList->dwUnitType == pUnit->dwType) || pStatList->pUnit == pUnit) {
    pStat = pStatList->StatVec.pStats;

    for (int nStat = 0; nStat < pStatList->StatVec.wCount; nStat++) {
      InsertStatsNow(pStat, nStat, cx, pArray);
    }
  }
  if ((pStatList->dwFlags >> 24 & 0x80)) {
    pStat = pStatList->SetStatVec.pStats;

    for (int nStat = 0; nStat < pStatList->SetStatVec.wCount; nStat++) {
      InsertStatsNow(pStat, nStat, cx, pArray);
    }
  }
}

void InsertStatsNow(D2StatStrc* pStat, int nStat, JSContext* cx, JS::MutableHandleObject pArray) {
  if (pStat[nStat].wSubIndex > 0x200) {
    // subindex is the skill id and level
    int skill = pStat[nStat].wSubIndex >> 6, level = pStat[nStat].wSubIndex & 0x3F, charges = 0, maxcharges = 0;
    if (pStat[nStat].dwStatValue > 0x200) {
      charges = pStat[nStat].dwStatValue & 0xFF;
      maxcharges = pStat[nStat].dwStatValue >> 8;
    }
    JS::RootedObject obj(cx, JS_NewPlainObject(cx));
    // val is an anonymous object that holds properties
    JS::RootedValue skill_val(cx, JS::Int32Value(skill));
    JS::RootedValue level_val(cx, JS::Int32Value(level));
    JS::RootedValue charges_val(cx, JS::Int32Value(charges));
    JS::RootedValue maxcharges_val(cx, JS::Int32Value(maxcharges));
    JS_SetProperty(cx, obj, "skill", skill_val);
    JS_SetProperty(cx, obj, "level", level_val);
    if (maxcharges > 0) {
      JS_SetProperty(cx, obj, "charges", charges_val);
      JS_SetProperty(cx, obj, "maxcharges", maxcharges_val);
    }

    // find where we should put it
    // TODO(ejt): FIX THIS!
    JS::RootedValue index(cx, JS::UndefinedValue());
    if (!JS_GetElement(cx, pArray, pStat[nStat].wStatIndex, &index))
      return;
    if (!index.isUndefined()) {
      // modify the existing object by stuffing it into an array
      bool isArray;
      if (JS_IsArrayObject(cx, index, &isArray) && !isArray) {
        // it's not an array, build one
        JS::RootedObject arr(cx, JS_NewArrayObject(cx, 0));
        JS_SetElement(cx, arr, 0, index);
        JS_SetElement(cx, arr, 1, obj);
        JS_SetElement(cx, pArray, pStat[nStat].wStatIndex, arr);
      } else {
        // it is an array, append the new value
        JS::RootedObject arr(cx, index.toObjectOrNull());
        uint32_t len = 0;
        if (!JS_GetArrayLength(cx, arr, &len))
          return;
        len++;
        JS_SetElement(cx, arr, len, obj);
      }
    } else {
      JS_SetElement(cx, pArray, pStat[nStat].wStatIndex, obj);
    }
  } else {
    // Make sure to bit shift life, mana and stamina properly!
    int value = pStat[nStat].dwStatValue;
    if (pStat[nStat].wStatIndex >= 6 && pStat[nStat].wStatIndex <= 11)
      value = value >> 8;

    JS::RootedValue index(cx, JS::UndefinedValue());
    JS::RootedValue val(cx, JS::Int32Value(value));
    if (!JS_GetElement(cx, pArray, pStat[nStat].wStatIndex, &index))
      return;
    if (index.isUndefined()) {
      // the array index doesn't exist, make it
      index.setObjectOrNull(JS_NewArrayObject(cx, 0));
      if (!JS_SetElement(cx, pArray, pStat[nStat].wStatIndex, index)) {
        return;
      }
    }
    // index now points to the correct array index
    JS::RootedObject arr(cx, index.toObjectOrNull());
    JS_SetElement(cx, arr, pStat[nStat].wSubIndex, val);
  }
}

bool UnitWrap::getState(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  UnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  UnitData* lpUnit = wrap->pData;
  if (!lpUnit || (lpUnit->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
    args.rval().setBoolean(false);
    return true;
  }

  D2UnitStrc* pUnit = D2UnitStrc::FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

  if (!pUnit || !args[0].isInt32()) {
    args.rval().setBoolean(false);
    return true;
  }

  int32_t nState = args[0].toInt32();

  // TODO: make these constants so we know what we're checking here
  if (nState > 183 || nState < 0) {
    args.rval().setBoolean(false);
    return true;
  }

  args.rval().setBoolean(D2COMMON_GetUnitState(pUnit, nState));
  return true;
}

bool UnitWrap::getEnchant(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  if (args.length() < 1 || !args[0].isInt32()) {
    args.rval().setUndefined();
    return true;
  }

  UnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  UnitData* pmyUnit = wrap->pData;
  if (!pmyUnit || (pmyUnit->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
    args.rval().setUndefined();
    return true;
  }

  D2UnitStrc* pUnit = D2UnitStrc::FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

  if (!pUnit || pUnit->dwType != UNIT_MONSTER) {
    args.rval().setUndefined();
    return true;
  }

  int nEnchant = args[0].toInt32();
  for (int i = 0; i < 9; i++)
    if (pUnit->pMonsterData->anEnchants[i] == nEnchant) {
      args.rval().setBoolean(true);
      return true;
    }

  args.rval().setInt32(0);
  return true;
}

bool UnitWrap::shop(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  AutoCriticalRoom cRoom;

  if (*D2CLIENT_TransactionDialog != 0 || *D2CLIENT_TransactionDialogs != 0 || *D2CLIENT_TransactionDialogs_2 != 0) {
    args.rval().setBoolean(false);
    return true;
  }

  UnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  UnitData* lpItem = wrap->pData;
  if (!lpItem || (lpItem->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
    args.rval().setBoolean(false);
    return true;
  }

  D2UnitStrc* pItem = D2UnitStrc::FindUnit(lpItem->dwUnitId, lpItem->dwType);

  if (!pItem || pItem->dwType != UNIT_ITEM) {
    args.rval().setBoolean(false);
    return true;
  }

  if (!D2CLIENT_GetUIState(UI_NPCSHOP)) {
    args.rval().setBoolean(false);
    return true;
  }

  D2UnitStrc* pNPC = D2CLIENT_GetCurrentInteractingNPC();
  uint32_t dwMode;
  JS::ToUint32(ctx, args[args.length() - 1], &dwMode);

  // Check if we are interacted.
  if (!pNPC) {
    args.rval().setBoolean(false);
    return true;
  }

  // Check for proper mode.
  if ((dwMode != 1) && (dwMode != 2) && (dwMode != 6)) {
    args.rval().setBoolean(false);
    return true;
  }

  // Selling an Item
  if (dwMode == 1) {
    // Check if we own the item!
    if (pItem->pItemData->pOwnerInventory->pOwner->dwUnitId != D2CLIENT_GetPlayerUnit()->dwUnitId) {
      args.rval().setBoolean(false);
      return true;
    }

    D2CLIENT_ShopAction(pNPC, pItem, 1, 0, 0, 1, 1, NULL);
  } else {
    // Make sure the item is owned by the NPC interacted with.
    if (pItem->pItemData->pOwnerInventory->pOwner->dwUnitId != pNPC->dwUnitId) {
      args.rval().setBoolean(false);
      return true;
    }

    D2CLIENT_ShopAction(pNPC, pItem, 0, 0, 0, dwMode, 1, NULL);
  }

  /*BYTE pPacket[17] = {NULL};

   if(dwMode == 2 || dwMode == 6)
           pPacket[0] = 0x32;
   else pPacket[0] = 0x33;

  *(DWORD*)&pPacket[1] = pNPC->dwUnitId;
  *(DWORD*)&pPacket[5] = pItem->dwUnitId;

   if(dwMode == 1) // Sell
  {
           if(D2CLIENT_GetCursorItem() && D2CLIENT_GetCursorItem() == pItem)
                   *(DWORD*)&pPacket[9] = 0x04;
           else
                   *(DWORD*)&pPacket[9] = 0;
   }
   else if(dwMode == 2) // Buy
  {
           if(pItem->pItemData->dwFlags & 0x10)
                   *(DWORD*)&pPacket[9] = 0x00;
           else
                   *(DWORD*)&pPacket[9] = 0x02;
   }
   else
           *(BYTE*)&pPacket[9+3] = 0x80;

   int nBuySell = NULL;

   if(dwMode == 2 || dwMode == 6)
           nBuySell = NULL;
   else nBuySell = 1;

  *(DWORD*)&pPacket[13] = D2COMMON_GetItemPrice(D2CLIENT_GetPlayerUnit(), pItem, D2CLIENT_GetDifficulty(), *p_D2CLIENT_ItemPriceList, pNPC->dwTxtFileNo,
   nBuySell);

   D2NET_SendPacket(sizeof(pPacket), 1, pPacket);*/

  args.rval().setBoolean(true);
  return true;
}

// TODO(ejt): cleanup, remove dependency on JSContext and move to D2UnitStrc
static BOOL SetSkill(JSContext* cx, WORD wSkillId, BOOL bLeft, DWORD dwItemId) {
  if (ClientState() != ClientStateInGame)
    return FALSE;

  D2UnitStrc* player = D2CLIENT_GetPlayerUnit();
  if (!player->GetSkillFromSkillId(wSkillId))
    return FALSE;

  BYTE aPacket[9];

  aPacket[0] = 0x3C;
  *(WORD*)&aPacket[1] = wSkillId;
  aPacket[3] = 0;
  aPacket[4] = (bLeft) ? 0x80 : 0;
  *(DWORD*)&aPacket[5] = dwItemId;

  D2CLIENT_SendGamePacket(9, aPacket);

  D2UnitStrc* Me = D2CLIENT_GetPlayerUnit();

  int timeout = 0;
  D2SkillStrc* hand = NULL;
  while (ClientState() == ClientStateInGame) {
    hand = (bLeft ? Me->pInfo->pLeftSkill : Me->pInfo->pRightSkill);
    if (hand->pSkillInfo->wSkillId != wSkillId) {
      if (timeout > 10)
        return FALSE;
      timeout++;
    } else
      return TRUE;

    ThreadState* ts = (ThreadState*)JS_GetContextPrivate(cx);  // run events to avoid packet block deadlock
    ts->script->BlockThread(100);
  }

  return FALSE;
}

bool UnitWrap::setSkill(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  // uint32_t nSkillId = (WORD)-1;
  D2SkillStrc* pSkill = nullptr;
  BOOL nHand = FALSE;
  DWORD itemId = (DWORD)-1;

  if (args.length() < 1) {
    args.rval().setBoolean(false);
    return true;
  }

  D2UnitStrc* player = D2CLIENT_GetPlayerUnit();
  if (!player) {
    args.rval().setBoolean(false);
    return true;
  }

  if (args[0].isString()) {
    char* name = JS_EncodeString(ctx, args[0].toString());
    pSkill = player->GetSkillFromSkillName(name);
    JS_free(ctx, name);
  } else if (args[0].isNumber()) {
    uint32_t skillId;
    JS::ToUint32(ctx, args[0], &skillId);
    pSkill = player->GetSkillFromSkillId(skillId);
  } else {
    args.rval().setBoolean(false);
    return true;
  }

  if (args[1].isInt32()) {
    nHand = args[1].toInt32();
  } else {
    args.rval().setBoolean(false);
    return true;
  }

  if (args.length() == 3 && args[2].isObject()) {
    JS::RootedObject test_obj(ctx, args[2].toObjectOrNull());
    if (JS_InstanceOf(ctx, test_obj, &m_class, &args)) {
      UnitWrap* wrap;
      UNWRAP_OR_RETURN(ctx, &wrap, args[2]);
      UnitData* unit = wrap->pData;
      if (unit->dwType == UNIT_ITEM)
        itemId = unit->dwUnitId;
    }
  }

  if (!pSkill) {
    args.rval().setBoolean(false);
    return true;
  }

  if (SetSkill(ctx, pSkill->pSkillInfo->wSkillId, nHand, itemId)) {
    args.rval().setBoolean(true);
    return true;
  }

  args.rval().setBoolean(false);
  return true;
}

bool UnitWrap::move(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  UnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  UnitData* pmyUnit = wrap->pData;
  if (!pmyUnit || (pmyUnit->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
    args.rval().setUndefined();
    return true;
  }

  D2UnitStrc* pUnit = D2UnitStrc::FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

  D2UnitStrc* pPlayer = D2CLIENT_GetPlayerUnit();

  if (!pPlayer || !pUnit) {
    args.rval().setUndefined();
    return true;
  }

  int32_t x, y;

  if (pUnit == pPlayer) {
    if (args.length() < 2) {
      args.rval().setUndefined();
      return true;
    }

    if (!JS::ToInt32(ctx, args[0], &x)) {
      args.rval().setUndefined();
      return true;
    }
    if (!JS::ToInt32(ctx, args[1], &y)) {
      args.rval().setUndefined();
      return true;
    }
  } else {
    x = D2CLIENT_GetUnitX(pUnit);
    y = D2CLIENT_GetUnitY(pUnit);
  }

  ClickMap(0, (WORD)x, (WORD)y, FALSE, NULL);
  Sleep(50);
  ClickMap(2, (WORD)x, (WORD)y, FALSE, NULL);
  //	D2CLIENT_Move((WORD)x, (WORD)y);
  args.rval().setUndefined();
  return true;
}

bool UnitWrap::getQuest(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  if (args.length() < 2 || !args[0].isInt32() || !args[1].isInt32()) {
    args.rval().setUndefined();
    return true;
  }

  int32_t nAct = args[0].toInt32();
  int32_t nQuest = args[1].toInt32();
  args.rval().setInt32(D2COMMON_GetQuestFlag(D2CLIENT_GetQuestInfo(), nAct, nQuest));
  return true;
}

bool UnitWrap::getMinionCount(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  if (args.length() < 1 || !args[0].isInt32()) {
    args.rval().setUndefined();
    return true;
  }

  int32_t nType = args[0].toInt32();

  UnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  UnitData* pmyUnit = wrap->pData;
  if (!pmyUnit || (pmyUnit->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
    args.rval().setUndefined();
    return true;
  }

  D2UnitStrc* pUnit = D2UnitStrc::FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

  if (!pUnit || (pUnit->dwType != UNIT_MONSTER && pUnit->dwType != UNIT_PLAYER)) {
    args.rval().setUndefined();
    return true;
  }

  args.rval().setInt32(D2CLIENT_GetMinionCount(pUnit, (DWORD)nType));
  return true;
}

bool UnitWrap::getRepairCost(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  D2UnitStrc* npc = D2CLIENT_GetCurrentInteractingNPC();
  int32_t nNpcClassId = (npc ? npc->dwTxtFileNo : 0x9A);

  if (args.length() > 0 && args[0].isInt32())
    nNpcClassId = args[0].toInt32();

  args.rval().setInt32(D2COMMON_GetRepairCost(NULL, D2CLIENT_GetPlayerUnit(), nNpcClassId, D2CLIENT_GetDifficulty(), *D2CLIENT_ItemPriceList, 0));
  return true;
}

bool UnitWrap::getItemCost(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  int32_t nMode;
  D2UnitStrc* npc = D2CLIENT_GetCurrentInteractingNPC();
  int32_t nNpcClassId = (npc ? npc->dwTxtFileNo : 0x9A);  // defaults to Charsi's NPC id
  int32_t nDifficulty = D2CLIENT_GetDifficulty();

  if (args.length() < 1 || !args[0].isInt32()) {
    args.rval().setUndefined();
    return true;
  }

  UnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  UnitData* lpUnit = wrap->pData;
  if (!lpUnit || (lpUnit->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
    args.rval().setUndefined();
    return true;
  }

  D2UnitStrc* pUnit = D2UnitStrc::FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

  if (!pUnit || pUnit->dwType != UNIT_ITEM) {
    args.rval().setUndefined();
    return true;
  }

  nMode = args[0].toInt32();

  if (args.length() > 1) {
    if (args[1].isObject()) {
      UnitWrap* npc_wrap;
      UNWRAP_OR_RETURN(ctx, &npc_wrap, args[1]);
      UnitData* pmyNpc = npc_wrap->pData;

      if (!pmyNpc || (pmyNpc->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
        args.rval().setUndefined();
        return true;
      }

      D2UnitStrc* pNpc = D2UnitStrc::FindUnit(pmyNpc->dwUnitId, pmyNpc->dwType);

      if (!pNpc) {
        args.rval().setUndefined();
        return true;
      }
      nNpcClassId = pNpc->dwTxtFileNo;
    } else if (args[1].isInt32() && !args[1].isNull()) {
      if (!JS::ToInt32(ctx, args[1], &nNpcClassId)) {
        args.rval().setUndefined();
        return true;
      }
    }
    // TODO:: validate the base stat table sizes to make sure the game doesn't crash with checking values past the end of the table
    int retVal = 0;
    if (FillBaseStat("monstats", nNpcClassId, "inventory", &retVal, sizeof(int)) && retVal == 0)
      nNpcClassId = 0x9A;  // invalid npcid incoming! default to charsi to allow the game to continue
  }

  if (args.length() > 2 && args[2].isInt32())
    nDifficulty = args[2].toInt32();

  switch (nMode) {
    case 0:  // Buy
    case 1:  // Sell
      args.rval().setInt32(D2COMMON_GetItemPrice(D2CLIENT_GetPlayerUnit(), pUnit, nDifficulty, *D2CLIENT_ItemPriceList, nNpcClassId, nMode));
      return true;
      break;
    case 2:  // Repair
      args.rval().setInt32(D2COMMON_GetItemPrice(D2CLIENT_GetPlayerUnit(), pUnit, nDifficulty, *D2CLIENT_ItemPriceList, nNpcClassId, 3));
      return true;
      break;
    default:
      break;
  }

  args.rval().setUndefined();
  return true;
}

bool UnitWrap::getUnit(JSContext* ctx, JS::CallArgs& args) {
  if (args.length() < 1) {
    args.rval().setUndefined();
    return true;
  }

  int nType = -1;
  uint32_t nClassId = (uint32_t)-1;
  uint32_t nMode = (uint32_t)-1;
  uint32_t nUnitId = (uint32_t)-1;
  char* szName = nullptr;

  if (args.length() > 0 && args[0].isNumber())
    JS::ToInt32(ctx, args[0], &nType);

  if (args.length() > 1 && args[1].isString())
    szName = JS_EncodeString(ctx, args[1].toString());

  if (args.length() > 1 && args[1].isNumber() && !args[1].isNull())
    JS::ToUint32(ctx, args[1], &nClassId);

  if (args.length() > 2 && args[2].isNumber() && !args[2].isNull())
    JS::ToUint32(ctx, args[2], &nMode);

  if (args.length() > 3 && args[3].isNumber() && !args[3].isNull())
    JS::ToUint32(ctx, args[3], &nUnitId);

  D2UnitStrc* pUnit = NULL;

  if (nType == 100)
    pUnit = D2CLIENT_GetCursorItem();
  else if (nType == 101) {
    pUnit = D2CLIENT_GetSelectedUnit();
    if (!pUnit)
      pUnit = (*D2CLIENT_SelectedInvItem);
  } else
    pUnit = D2UnitStrc::FindUnit(szName ? szName : "", nClassId, nType, nMode, nUnitId);

  if (!pUnit) {
    if (szName) {
      JS_free(ctx, szName);
    }
    args.rval().setUndefined();
    return true;
  }

  UnitData* pmyUnit = new UnitData;  // leaked?

  if (!pmyUnit) {
    if (szName) {
      JS_free(ctx, szName);
    }
    args.rval().setUndefined();
    return true;
  }

  pmyUnit->dwPrivateType = PRIVATE_UNIT;
  pmyUnit->dwClassId = nClassId;
  pmyUnit->dwMode = nMode;
  pmyUnit->dwType = pUnit->dwType;
  pmyUnit->dwUnitId = pUnit->dwUnitId;
  strcpy_s(pmyUnit->szName, sizeof(pmyUnit->szName), szName ? szName : "");
  if (szName) {
    JS_free(ctx, szName);
  }

  JS::RootedObject newObject(ctx, UnitWrap::Instantiate(ctx, pmyUnit));
  if (!newObject) {
    THROW_ERROR(ctx, "failed to instantiate unit");
  }
  args.rval().setObject(*newObject);
  return true;
}

D2BS_BINDING_INTERNAL(UnitWrap, UnitWrap::Initialize)
