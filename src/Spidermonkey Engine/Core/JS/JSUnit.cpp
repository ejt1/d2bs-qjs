#include "JSUnit.h"
#include "D2Helpers.h"
#include "Constants.h"
#include "Helpers.h"
#include "Unit.h"
#include "Core.h"
#include "CriticalSections.h"
#include "D2Skills.h"
#include "MPQStats.h"

EMPTY_CTOR(unit)

CLASS_FINALIZER(unit) {
  Private* lpUnit = (Private*)JS_GetPrivate(val);

  if (lpUnit) {
    switch (lpUnit->dwPrivateType) {
      case PRIVATE_UNIT: {
        myUnit* unit = (myUnit*)lpUnit;
        delete unit;
        break;
      }
      case PRIVATE_ITEM: {
        invUnit* unit = (invUnit*)lpUnit;
        delete unit;
        break;
      }
    }
  }
  JS_SetPrivate(val, NULL);
}

JSAPI_PROP(unit_getProperty) {
  BnetData* pData = *p_D2LAUNCH_BnData;
  GameStructInfo* pInfo = *p_D2CLIENT_GameInfo;

  switch (magic) {
    case ME_PID:
      return JS_NewFloat64(ctx, GetCurrentProcessId());
    case ME_PROFILE:
      return JS_NewString(ctx, Vars.szProfile);
    case ME_GAMEREADY:
      return JS_NewBool(ctx, GameReady());
    case ME_ACCOUNT:
      if (!pData)
        return JS_UNDEFINED;
      return JS_NewString(ctx, pData->szAccountName);
    case ME_CHARNAME:
      if (!pData)
        return JS_UNDEFINED;
      return JS_NewString(ctx, pData->szPlayerName);
    case ME_CHICKENHP:
      return JS_NewInt32(ctx, Vars.nChickenHP);
    case ME_CHICKENMP:
      return JS_NewInt32(ctx, Vars.nChickenMP);
    case ME_DIFF:
      return JS_NewInt32(ctx, D2CLIENT_GetDifficulty());
    case ME_MAXDIFF:
      return JS_NewInt32(ctx, pData->nMaxDiff);
    case ME_GAMENAME:
      if (!pInfo)
        return JS_UNDEFINED;
      return JS_NewString(ctx, pInfo->szGameName);
    case ME_GAMEPASSWORD:
      if (!pInfo)
        return JS_UNDEFINED;
      return JS_NewString(ctx, pInfo->szGamePassword);
    case ME_GAMESERVERIP:
      if (!pInfo)
        return JS_UNDEFINED;
      return JS_NewString(ctx, pInfo->szGameServerIp);
    case ME_GAMESTARTTIME:
      return JS_NewFloat64(ctx, Vars.dwGameTime);
    case ME_GAMETYPE:
      return JS_NewInt32(ctx, *p_D2CLIENT_ExpCharFlag);
    case ME_PLAYERTYPE:
      if (pData) {
        return JS_NewBool(ctx, !!(pData->nCharFlags & PLAYER_TYPE_HARDCORE));
      }
      break;
    case ME_ITEMONCURSOR:
      return JS_NewBool(ctx, !!(D2CLIENT_GetCursorItem()));
    case ME_AUTOMAP:
      return JS_NewBool(ctx, *p_D2CLIENT_AutomapOn);
    case ME_LADDER:
      if (pData) {
        return JS_NewFloat64(ctx, pData->ladderflag);
      }
      break;
    case ME_QUITONHOSTILE:
      return JS_NewBool(ctx, Vars.bQuitOnHostile);
    case ME_REALM:
      return JS_NewString(ctx, pData->szRealmName);
    case ME_REALMSHORT:
      return JS_NewString(ctx, pData->szRealmName2);
    case OOG_SCREENSIZE:
      return JS_NewInt32(ctx, D2GFX_GetScreenSize());
    case OOG_WINDOWTITLE: {
      wchar_t szTitle[256];
      GetWindowTextW(D2GFX_GetHwnd(), szTitle, 256);
      return JS_NewString(ctx, szTitle);
    } break;
    case ME_PING:
      return JS_NewInt32(ctx, *p_D2CLIENT_Ping);
    case ME_FPS:
      return JS_NewInt32(ctx, *p_D2CLIENT_FPS);
    case ME_LOCALE:
      return JS_NewInt32(ctx, *p_D2CLIENT_Lang);
    case OOG_INGAME:
      return JS_NewBool(ctx, (ClientState() == ClientStateMenu ? false : true));
    case OOG_QUITONERROR:
      return JS_NewBool(ctx, Vars.bQuitOnError);
    case OOG_MAXGAMETIME:
      return JS_NewInt32(ctx, Vars.dwMaxGameTime);
    case ME_MERCREVIVECOST:
      return JS_NewInt32(ctx, *p_D2CLIENT_MercReviveCost);
    case ME_BLOCKKEYS:
      return JS_NewBool(ctx, Vars.bBlockKeys);
    case ME_BLOCKMOUSE:
      return JS_NewBool(ctx, Vars.bBlockMouse);
    case ME_UNSUPPORTED:
      return JS_NewBool(ctx, Vars.bEnableUnsupported);
    case ME_CHARFLAGS:
      if (pData) {
        return JS_NewInt32(ctx, pData->nCharFlags);
      }
      break;
    default:
      break;
  }
  if (ClientState() != ClientStateInGame)
    return JS_UNDEFINED;

  myUnit* lpUnit = (myUnit*)JS_GetPrivate(ctx, this_val);
  if (!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
    return JS_UNDEFINED;

  UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);
  if (!pUnit) {
    return JS_UNDEFINED;
  }
  Room1* pRoom = NULL;

  switch (magic) {
    case UNIT_TYPE:
      return JS_NewUint32(ctx, pUnit->dwType);
    case UNIT_CLASSID:
      return JS_NewUint32(ctx, pUnit->dwTxtFileNo);
    case UNIT_MODE:
      return JS_NewUint32(ctx, pUnit->dwMode);
    case UNIT_NAME: {
      char tmp[128] = "";
      GetUnitName(pUnit, tmp, 128);
      return JS_NewString(ctx, tmp);
    } break;
    case ME_MAPID:
      return JS_NewInt32(ctx, *p_D2CLIENT_MapId);
    case ME_NOPICKUP:
      return JS_NewBool(ctx, !!*p_D2CLIENT_NoPickUp);
    case UNIT_ACT:
      return JS_NewUint32(ctx, pUnit->dwAct + 1);
    case UNIT_AREA:
      pRoom = D2COMMON_GetRoomFromUnit(pUnit);
      if (pRoom && pRoom->pRoom2 && pRoom->pRoom2->pLevel) {
        return JS_NewUint32(ctx, pRoom->pRoom2->pLevel->dwLevelNo);
      }
      break;
    case UNIT_ID:
      return JS_NewFloat64(ctx, pUnit->dwUnitId);
      break;
    case UNIT_XPOS:
      return JS_NewInt32(ctx, D2CLIENT_GetUnitX(pUnit));
    case UNIT_YPOS:
      return JS_NewInt32(ctx, D2CLIENT_GetUnitY(pUnit));
    case UNIT_TARGETX:
      switch (pUnit->dwType) {
        case 0:
        case 1:
        case 3:
          return JS_NewInt32(ctx, pUnit->pPath->xTarget);
      }
      break;
    case UNIT_TARGETY:
      switch (pUnit->dwType) {
        case 0:
        case 1:
        case 3:
          return JS_NewInt32(ctx, pUnit->pPath->yTarget);
      }
      break;
    case UNIT_HP:
      return JS_NewInt32(ctx, D2COMMON_GetUnitStat(pUnit, 6, 0) >> 8);
    case UNIT_HPMAX:
      return JS_NewInt32(ctx, D2COMMON_GetUnitStat(pUnit, 7, 0) >> 8);
    case UNIT_MP:
      return JS_NewInt32(ctx, D2COMMON_GetUnitStat(pUnit, 8, 0) >> 8);
    case UNIT_MPMAX:
      return JS_NewInt32(ctx, D2COMMON_GetUnitStat(pUnit, 9, 0) >> 8);
    case UNIT_STAMINA:
      return JS_NewInt32(ctx, D2COMMON_GetUnitStat(pUnit, 10, 0) >> 8);
    case UNIT_STAMINAMAX:
      return JS_NewInt32(ctx, D2COMMON_GetUnitStat(pUnit, 11, 0) >> 8);
    case UNIT_CHARLVL:
      return JS_NewInt32(ctx, D2COMMON_GetUnitStat(pUnit, 12, 0));
    case ME_RUNWALK:
      if (pUnit == D2CLIENT_GetPlayerUnit()) {
        return JS_NewInt32(ctx, *p_D2CLIENT_AlwaysRun);
      }
      break;
    case UNIT_SPECTYPE:
      DWORD SpecType;
      SpecType = NULL;
      if (pUnit->dwType == UNIT_MONSTER && pUnit->pMonsterData) {
        if (pUnit->pMonsterData->fMinion & 1)
          SpecType |= 0x08;
        if (pUnit->pMonsterData->fBoss & 1)
          SpecType |= 0x04;
        if (pUnit->pMonsterData->fChamp & 1)
          SpecType |= 0x02;
        if ((pUnit->pMonsterData->fBoss & 1) && (pUnit->pMonsterData->fNormal & 1))
          SpecType |= 0x01;
        if (pUnit->pMonsterData->fNormal & 1)
          SpecType |= 0x00;
        return JS_NewInt32(ctx, SpecType);
      }
      break;
    case UNIT_UNIQUEID:
      if (pUnit->dwType == UNIT_MONSTER && pUnit->pMonsterData->fBoss && pUnit->pMonsterData->fNormal) {
        return JS_NewInt32(ctx, pUnit->pMonsterData->wUniqueNo);
      } else {
        return JS_NewInt32(ctx, -1);
      }
      break;
    case ITEM_CODE:  // replace with better method if found
      if (!(pUnit->dwType == UNIT_ITEM) && pUnit->pItemData)
        break;
      ItemTxt* pTxt;
      pTxt = D2COMMON_GetItemText(pUnit->dwTxtFileNo);
      if (!pTxt) {
        return JS_NewString(ctx, "Unknown");
      }
      char szCode[4];
      memcpy(szCode, pTxt->szCode, 3);
      szCode[3] = 0x00;
      return JS_NewString(ctx, szCode);
    case ITEM_PREFIX:
      if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData)
        if (D2COMMON_GetItemMagicalMods(pUnit->pItemData->wMagicPrefix[0]))
          return JS_NewString(ctx, D2COMMON_GetItemMagicalMods(pUnit->pItemData->wMagicPrefix[0]));
      break;
    case ITEM_SUFFIX:
      if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData)
        if (D2COMMON_GetItemMagicalMods(pUnit->pItemData->wMagicSuffix[0]))
          return JS_NewString(ctx, D2COMMON_GetItemMagicalMods(pUnit->pItemData->wMagicSuffix[0]));
      break;
    case ITEM_PREFIXNUM:
      if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData) {
        return JS_NewInt32(ctx, pUnit->pItemData->wMagicPrefix[0]);
      }
      break;
    case ITEM_SUFFIXNUM:
      if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData) {
        return JS_NewInt32(ctx, pUnit->pItemData->wMagicSuffix[0]);
      }
      break;

    case ITEM_PREFIXES:
      if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData) {
        JSValue pReturnArray = JS_NewArray(ctx);

        for (int i = 0; i < 3; i++) {
          if (D2COMMON_GetItemMagicalMods(pUnit->pItemData->wMagicPrefix[i])) {
            JS_SetPropertyUint32(ctx, pReturnArray, i, JS_NewString(ctx, D2COMMON_GetItemMagicalMods(pUnit->pItemData->wMagicPrefix[i])));
          }
        }
        return pReturnArray;
      }

      break;
    case ITEM_PREFIXNUMS:
      if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData) {
        JSValue pReturnArray = JS_NewArray(ctx);

        for (int i = 0; i < 3; i++) {
          if (pUnit->pItemData->wMagicPrefix[i]) {
            JS_SetPropertyUint32(ctx, pReturnArray, i, JS_NewInt32(ctx, pUnit->pItemData->wMagicPrefix[i]));
          }
        }
        return pReturnArray;
      }

      break;
    case ITEM_SUFFIXES:
      if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData) {
        JSValue pReturnArray = JS_NewArray(ctx);

        for (int i = 0; i < 3; i++) {
          if (D2COMMON_GetItemMagicalMods(pUnit->pItemData->wMagicSuffix[i])) {
            JS_SetPropertyUint32(ctx, pReturnArray, i, JS_NewString(ctx, D2COMMON_GetItemMagicalMods(pUnit->pItemData->wMagicSuffix[i])));
          }
        }
        return pReturnArray;
      }
      break;
    case ITEM_SUFFIXNUMS:
      if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData) {
        JSValue pReturnArray = JS_NewArray(ctx);

        for (int i = 0; i < 3; i++) {
          if (pUnit->pItemData->wMagicSuffix[i]) {
            JS_SetPropertyUint32(ctx, pReturnArray, i, JS_NewInt32(ctx, pUnit->pItemData->wMagicSuffix[i]));
          }
        }
        return pReturnArray;
      }
      break;

    case ITEM_FNAME:
      if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData) {
        wchar_t wszfname[256] = L"";
        D2CLIENT_GetItemName(pUnit, wszfname, _countof(wszfname));
        if (wszfname) {
          return JS_NewString(ctx, wszfname);
        }
      }
      break;
    case ITEM_QUALITY:
      if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData) {
        return JS_NewInt32(ctx, pUnit->pItemData->dwQuality);
      }
      break;
    case ITEM_NODE:
      if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData) {
        return JS_NewInt32(ctx, pUnit->pItemData->NodePage);
      }
      break;
    case ITEM_LOC:
      if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData) {
        return JS_NewInt32(ctx, pUnit->pItemData->GameLocation);
      }
      break;
    case ITEM_SIZEX:
      if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData) {
        if (!D2COMMON_GetItemText(pUnit->dwTxtFileNo))
          break;
        return JS_NewInt32(ctx, D2COMMON_GetItemText(pUnit->dwTxtFileNo)->xSize);
      }
      break;
    case ITEM_SIZEY:
      if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData) {
        if (!D2COMMON_GetItemText(pUnit->dwTxtFileNo))
          break;
        return JS_NewInt32(ctx, D2COMMON_GetItemText(pUnit->dwTxtFileNo)->ySize);
      }
      break;
    case ITEM_TYPE:
      if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData) {
        if (!D2COMMON_GetItemText(pUnit->dwTxtFileNo))
          break;
        return JS_NewInt32(ctx, D2COMMON_GetItemText(pUnit->dwTxtFileNo)->nType);
      }
      break;
    case ITEM_DESC: {
      if (pUnit->dwType != UNIT_ITEM)
        break;

      AutoCriticalRoom* cRoom = new AutoCriticalRoom;

      wchar_t wBuffer[2048] = L"";
      wchar_t bBuffer[1] = {1};
      if (pUnit->pItemData && pUnit->pItemData->pOwnerInventory && pUnit->pItemData->pOwnerInventory->pOwner) {
        ::WriteProcessMemory(GetCurrentProcess(), (void*)GetDllOffset("D2Client.dll", 0x7BCBE8 - 0x400000), bBuffer, 1, NULL);
        ::WriteProcessMemory(GetCurrentProcess(), (void*)GetDllOffset("D2Client.dll", 0x7BCBF4 - 0x400000), &pUnit, 4, NULL);

        // D2CLIENT_LoadItemDesc(D2CLIENT_GetPlayerUnit(), 0);
        D2CLIENT_LoadItemDesc(pUnit->pItemData->pOwnerInventory->pOwner, 0);
        ReadProcessBYTES(GetCurrentProcess(), GetDllOffset("D2Win.dll", 0x841EC8 - 0x400000), wBuffer, 2047);
      }
      delete cRoom;
      if (wcslen(wBuffer) > 0) {
        return JS_NewString(ctx, wBuffer);
      }
    }

    break;
    case ITEM_GFX:
      if (pUnit->dwType == UNIT_ITEM && pUnit->pItemData) {
        return JS_NewInt32(ctx, pUnit->pItemData->bInvGfxIdx);
      }
      break;
    case UNIT_ITEMCOUNT:
      if (pUnit->pInventory) {
        return JS_NewInt32(ctx, pUnit->pInventory->dwItemCount);
      }
      break;
    case ITEM_BODYLOCATION:
      if (pUnit->dwType != UNIT_ITEM)
        break;
      if (pUnit->pItemData) {
        return JS_NewInt32(ctx, pUnit->pItemData->BodyLocation);
      }
      break;
    case UNIT_OWNER:
      return JS_NewFloat64(ctx, pUnit->dwOwnerId);
    case UNIT_OWNERTYPE:
      return JS_NewInt32(ctx, pUnit->dwOwnerType);
    case ITEM_LEVEL:
      if (pUnit->dwType != UNIT_ITEM)
        break;
      if (pUnit->pItemData) {
        return JS_NewInt32(ctx, pUnit->pItemData->dwItemLevel);
      }
      break;
    case ITEM_LEVELREQ:
      if (pUnit->dwType != UNIT_ITEM)
        break;
      return JS_NewInt32(ctx, D2COMMON_GetItemLevelRequirement(pUnit, D2CLIENT_GetPlayerUnit()));
    case UNIT_DIRECTION:

      if (pUnit->pPath && pUnit->pPath->pRoom1) {
        return JS_NewInt32(ctx, pUnit->pPath->bDirection);
      }
      break;
    case OBJECT_TYPE:
      if (pUnit->dwType == UNIT_OBJECT && pUnit->pObjectData) {
        pRoom = D2COMMON_GetRoomFromUnit(pUnit);
        if (pRoom && D2COMMON_GetLevelNoFromRoom(pRoom)) {
          return JS_NewInt32(ctx, pUnit->pObjectData->Type & 255);
        } else {
          return JS_NewInt32(ctx, pUnit->pObjectData->Type);
        }
      }
      break;
    case OBJECT_LOCKED:
      if (pUnit->dwType == UNIT_OBJECT && pUnit->pObjectData) {
        return JS_NewInt32(ctx, pUnit->pObjectData->ChestLocked);
      }
      break;
    case ME_WSWITCH:
      if (pUnit == D2CLIENT_GetPlayerUnit()) {
        return JS_NewInt32(ctx, *p_D2CLIENT_bWeapSwitch);
      }
      break;
    default:
      break;
  }

  return JS_UNDEFINED;
}

JSAPI_STRICT_PROP(unit_setProperty) {
  int32_t ival = 0;
  int32_t bval = false;

  if (JS_IsBool(val)) {
    bval = JS_ToBool(ctx, val);
  }
  if (JS_IsNumber(val)) {
    JS_ToInt32(ctx, &ival, val);
  }

  switch (magic) {
    case ME_CHICKENHP:
      if (JS_IsNumber(val))
        Vars.nChickenHP = ival;
      break;
    case ME_CHICKENMP:
      if (JS_IsNumber(val))
        Vars.nChickenMP = ival;
      break;
    case ME_QUITONHOSTILE:
      if (JS_IsBool(val))
        Vars.bQuitOnHostile = bval;
      break;
    case OOG_QUITONERROR:
      if (JS_IsBool(val))
        Vars.bQuitOnError = bval;
      break;
    case OOG_MAXGAMETIME:
      if (JS_IsNumber(val))
        Vars.dwMaxGameTime = ival;
      break;
    case ME_BLOCKKEYS:
      if (JS_IsBool(val))
        Vars.bBlockKeys = bval;
      break;
    case ME_BLOCKMOUSE:
      if (JS_IsBool(val))
        Vars.bBlockMouse = bval;
      break;
    case ME_RUNWALK:
      *p_D2CLIENT_AlwaysRun = !!ival;
      break;
    case ME_AUTOMAP:
      *p_D2CLIENT_AutomapOn = bval ? 1 : 0;
      break;
    case ME_NOPICKUP:
      *p_D2CLIENT_NoPickUp = !!ival;
      break;
  }

  return JS_UNDEFINED;
}

JSAPI_FUNC(unit_getUnit) {
  if (argc < 1)
    return JS_UNDEFINED;

  int nType = -1;
  uint32 nClassId = (uint32)-1;
  uint32 nMode = (uint32)-1;
  uint32 nUnitId = (uint32)-1;
  const char* szName = nullptr;

  if (argc > 0 && JS_IsNumber(argv[0]))
    JS_ToInt32(ctx, &nType, argv[0]);

  if (argc > 1 && JS_IsString(argv[1]))
    szName = JS_ToCString(ctx, argv[1]);

  if (argc > 1 && JS_IsNumber(argv[1]) && !JS_IsNull(argv[1]))
    JS_ToUint32(ctx, &nClassId, argv[1]);

  if (argc > 2 && JS_IsNumber(argv[2]) && !JS_IsNull(argv[2]))
    JS_ToUint32(ctx, &nMode, argv[2]);

  if (argc > 3 && JS_IsNumber(argv[3]) && !JS_IsNull(argv[3]))
    JS_ToUint32(ctx, &nUnitId, argv[3]);

  UnitAny* pUnit = NULL;

  if (nType == 100)
    pUnit = D2CLIENT_GetCursorItem();
  else if (nType == 101) {
    pUnit = D2CLIENT_GetSelectedUnit();
    if (!pUnit)
      pUnit = (*p_D2CLIENT_SelectedInvItem);
  } else
    pUnit = GetUnit(szName ? szName : "", nClassId, nType, nMode, nUnitId);

  if (!pUnit) {
    JS_FreeCString(ctx, szName);
    return JS_UNDEFINED;
  }

  myUnit* pmyUnit = new myUnit;  // leaked?

  if (!pmyUnit) {
    JS_FreeCString(ctx, szName);
    return JS_UNDEFINED;
  }

  pmyUnit->_dwPrivateType = PRIVATE_UNIT;
  pmyUnit->dwClassId = nClassId;
  pmyUnit->dwMode = nMode;
  pmyUnit->dwType = pUnit->dwType;
  pmyUnit->dwUnitId = pUnit->dwUnitId;
  strcpy_s(pmyUnit->szName, sizeof(pmyUnit->szName), szName ? szName : "");
  JS_FreeCString(ctx, szName);

  return BuildObject(ctx, unit_class_id, unit_methods, _countof(unit_methods), unit_props, _countof(unit_props), pmyUnit);
}

JSAPI_FUNC(unit_getNext) {
  Private* unit = (Private*)JS_GetPrivate(ctx, this_val);

  if (!unit)
    return JS_UNDEFINED;

  if (unit->dwPrivateType == PRIVATE_UNIT) {
    myUnit* lpUnit = (myUnit*)unit;
    UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

    if (!pUnit)
      return JS_UNDEFINED;
    if (argc > 0 && JS_IsString(argv[0])) {
      const char* szText = JS_ToCString(ctx, argv[0]);
      strcpy_s(lpUnit->szName, 128, szText);
      JS_FreeCString(ctx, szText);
    }

    if (argc > 0 && JS_IsNumber(argv[0]) && !JS_IsNull(argv[1]))
      JS_ToUint32(ctx, (uint32*)&(lpUnit->dwClassId), argv[0]);

    if (argc > 1 && JS_IsNumber(argv[1]) && !JS_IsNull(argv[2]))
      JS_ToUint32(ctx, (uint32*)&(lpUnit->dwMode), argv[1]);

    pUnit = GetNextUnit(pUnit, lpUnit->szName, lpUnit->dwClassId, lpUnit->dwType, lpUnit->dwMode);
    if (!pUnit) {
      // Same thing as bobode's fix for finalize
      /*JSObject* obj = JS_THIS_OBJECT(cx, vp);
      //JS_ClearScope(cx, obj);
      if(JS_ValueToObject(cx, JSVAL_NULL, &obj) == JS_FALSE)
              return JS_TRUE;*/
      return JS_FALSE;
    } else {
      lpUnit->dwUnitId = pUnit->dwUnitId;
      JS_SetPrivate(ctx, this_val, lpUnit);
      return JS_TRUE;
    }
  } else if (unit->dwPrivateType == PRIVATE_ITEM) {
    invUnit* pmyUnit = (invUnit*)unit;
    if (!pmyUnit)
      return JS_UNDEFINED;

    UnitAny* pUnit = D2CLIENT_FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);
    UnitAny* pOwner = D2CLIENT_FindUnit(pmyUnit->dwOwnerId, pmyUnit->dwOwnerType);
    if (!pUnit || !pOwner)
      return JS_UNDEFINED;

    if (argc > 0 && JS_IsString(argv[0])) {
      const char* szText = JS_ToCString(ctx, argv[0]);
      strcpy_s(pmyUnit->szName, 128, szText);
      JS_FreeCString(ctx, szText);
    }

    if (argc > 0 && JS_IsNumber(argv[0]) && !JS_IsNull(argv[1]))
      JS_ToUint32(ctx, (uint32*)&(pmyUnit->dwClassId), argv[0]);

    if (argc > 1 && JS_IsNumber(argv[1]) && !JS_IsNull(argv[2]))
      JS_ToUint32(ctx, (uint32*)&(pmyUnit->dwMode), argv[1]);

    UnitAny* nextItem = GetInvNextUnit(pUnit, pOwner, pmyUnit->szName, pmyUnit->dwClassId, pmyUnit->dwMode);
    if (!nextItem) {
      // set current object to null breaks the unit_finilize cleanup cycle
      /*JSObject* obj = JS_THIS_OBJECT(cx, vp);
      //JS_ClearScope(cx, obj);

      if(JS_ValueToObject(cx, JSVAL_NULL, &obj) == JS_FALSE)
              return JS_TRUE;
      */
      return JS_FALSE;
    } else {
      pmyUnit->dwUnitId = nextItem->dwUnitId;
      JS_SetPrivate(ctx, this_val, pmyUnit);
      return JS_TRUE;
    }
  }

  return JS_UNDEFINED;
}

JSAPI_FUNC(unit_cancel) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  DWORD automapOn = *p_D2CLIENT_AutomapOn;
  jsint mode = -1;

  if (argc > 0) {
    if (JS_IsNumber(argv[0]))
      JS_ToUint32(ctx, (uint32*)&mode, argv[0]);
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

  *p_D2CLIENT_AutomapOn = automapOn;
  return JS_TRUE;
}

JSAPI_FUNC(unit_repair) {
  myUnit* lpUnit = (myUnit*)JS_GetPrivate(ctx, this_val);

  if (!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
    return JS_FALSE;

  UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

  if (!pUnit)
    return JS_FALSE;

  BYTE aPacket[17] = {NULL};
  aPacket[0] = 0x35;
  *(DWORD*)&aPacket[1] = *p_D2CLIENT_RecentInteractId;
  aPacket[16] = 0x80;
  D2NET_SendPacket(17, 1, aPacket);

  // note: this crashes while minimized
  //	D2CLIENT_PerformNpcAction(pUnit,1, NULL);

  return JS_TRUE;
}

JSAPI_FUNC(unit_useMenu) {
  myUnit* lpUnit = (myUnit*)JS_GetPrivate(ctx, this_val);

  if (argc < 1 || !JS_IsNumber(argv[0]))
    return JS_FALSE;

  if (!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
    return JS_FALSE;

  UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

  if (!pUnit)
    return JS_FALSE;

  uint32_t menuId;
  JS_ToUint32(ctx, &menuId, argv[0]);
  return JS_NewBool(ctx, ClickNPCMenu(pUnit->dwTxtFileNo, menuId));
}

JSAPI_FUNC(unit_interact) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  myUnit* lpUnit = (myUnit*)JS_GetPrivate(ctx, this_val);

  if (!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
    return JS_FALSE;

  UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

  if (!pUnit || pUnit == D2CLIENT_GetPlayerUnit())
    return JS_FALSE;

  if (pUnit->dwType == UNIT_ITEM && pUnit->dwMode != ITEM_MODE_ON_GROUND && pUnit->dwMode != ITEM_MODE_BEING_DROPPED) {
    int nLocation = GetItemLocation(pUnit);

    BYTE aPacket[13] = {NULL};

    if (nLocation == LOCATION_INVENTORY || nLocation == LOCATION_STASH) {
      aPacket[0] = 0x20;
      *(DWORD*)&aPacket[1] = pUnit->dwUnitId;
      *(DWORD*)&aPacket[5] = D2CLIENT_GetPlayerUnit()->pPath->xPos;
      *(DWORD*)&aPacket[9] = D2CLIENT_GetPlayerUnit()->pPath->yPos;
      D2NET_SendPacket(13, 1, aPacket);
      return JS_FALSE;
    } else if (nLocation == LOCATION_BELT) {
      aPacket[0] = 0x26;
      *(DWORD*)&aPacket[1] = pUnit->dwUnitId;
      *(DWORD*)&aPacket[5] = 0;
      *(DWORD*)&aPacket[9] = 0;
      D2NET_SendPacket(13, 1, aPacket);
      return JS_FALSE;
    }
  }

  if (pUnit->dwType == UNIT_OBJECT && argc == 1 && JS_IsNumber(argv[0])) {
    // TODO: check the range on argv[0] to make sure it won't crash the game - Done! TechnoHunter
    jsint nWaypointID;
    if (JS_ToInt32(ctx, &nWaypointID, argv[0])) {
      return JS_FALSE;
    }
    int retVal = 0;
    if (FillBaseStat("levels", nWaypointID, "Waypoint", &retVal, sizeof(int)))
      if (retVal == 255)
        return JS_FALSE;

    D2CLIENT_TakeWaypoint(pUnit->dwUnitId, nWaypointID);
    if (!D2CLIENT_GetUIState(UI_GAME))
      D2CLIENT_CloseInteract();

    return JS_TRUE;
  }
  //	else if(pUnit->dwType == UNIT_PLAYER && argc == 1 && JSVAL_IS_INT(argv[0]) && JSVAL_TO_INT(argv[0]) == 1)
  //	{
  // Accept Trade
  //	}

  ClickMap(0, D2CLIENT_GetUnitX(pUnit), D2CLIENT_GetUnitY(pUnit), FALSE, pUnit);
  // D2CLIENT_Interact(pUnit, 0x45);
  return JS_TRUE;
}

void InsertStatsToGenericObject(UnitAny* pUnit, StatList* pStatList, JSContext* pJSContext, JSValue pGenericObject);
void InsertStatsNow(Stat* pStat, int nStat, JSContext* cx, JSValue pArray);

JSAPI_FUNC(unit_getStat) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  myUnit* lpUnit = (myUnit*)JS_GetPrivate(ctx, this_val);

  if (!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
    return JS_FALSE;

  UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

  if (!pUnit)
    return JS_FALSE;

  jsint nStat = 0;
  jsint nSubIndex = 0;
  JS_ToInt32(ctx, &nStat, argv[0]);
  if (JS_IsNumber(argv[1])) {
    JS_ToInt32(ctx, &nSubIndex, argv[1]);
  }

  JSValue rval = JS_FALSE;
  if (nStat >= STAT_HP && nStat <= STAT_MAXSTAMINA)
    rval = JS_NewInt32(ctx, D2COMMON_GetUnitStat(pUnit, nStat, nSubIndex) >> 8);
  else if (nStat == STAT_EXP || nStat == STAT_LASTEXP || nStat == STAT_NEXTEXP) {
    rval = JS_NewInt32(ctx, (unsigned int)D2COMMON_GetUnitStat(pUnit, nStat, nSubIndex));
  } else if (nStat == STAT_ITEMLEVELREQ)
    rval = JS_NewInt32(ctx, D2COMMON_GetItemLevelRequirement(pUnit, D2CLIENT_GetPlayerUnit()));
  else if (nStat == -1) {
    Stat aStatList[256] = {NULL};
    StatList* pStatList = D2COMMON_GetStatList(pUnit, NULL, 0x40);

    if (pStatList) {
      DWORD dwStats = D2COMMON_CopyStatList(pStatList, (Stat*)aStatList, 256);

      rval = JS_NewArray(ctx);
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
        JSValue pArrayInsert = JS_NewArray(ctx);
        if (!pArrayInsert)
          continue;

        JSValue nIndex = JS_NewInt32(ctx, aStatList[i].wStatIndex);
        JSValue _nSubIndex = JS_NewInt32(ctx, aStatList[i].wSubIndex);
        JSValue nValue = JS_NewInt32(ctx, aStatList[i].dwStatValue);

        JS_SetPropertyUint32(ctx, pArrayInsert, 0, nIndex);
        JS_SetPropertyUint32(ctx, pArrayInsert, 1, _nSubIndex);
        JS_SetPropertyUint32(ctx, pArrayInsert, 2, nValue);

        JS_SetPropertyUint32(ctx, rval, i, pArrayInsert);
      }
    }
  } else if (nStat == -2) {
    rval = JS_NewArray(ctx);

    InsertStatsToGenericObject(pUnit, pUnit->pStats, ctx, rval);
    InsertStatsToGenericObject(pUnit, D2COMMON_GetStatList(pUnit, NULL, 0x40), ctx, rval);
    // InsertStatsToGenericObject(pUnit, pUnit->pStats->pNext, cx, pArray);  // only check the current unit stats!
    //	InsertStatsToGenericObject(pUnit, pUnit->pStats->pSetList, cx, pArray);
  } else {
    long result = D2COMMON_GetUnitStat(pUnit, nStat, nSubIndex);
    if (result == 0)  // if stat isnt found look up preset list
    {
      StatList* pStatList = D2COMMON_GetStatList(pUnit, NULL, 0x40);
      Stat aStatList[256] = {NULL};
      if (pStatList) {
        DWORD dwStats = D2COMMON_CopyStatList(pStatList, (Stat*)aStatList, 256);
        for (UINT i = 0; i < dwStats; i++) {
          if (nStat == aStatList[i].wStatIndex && nSubIndex == aStatList[i].wSubIndex)
            result = (aStatList[i].dwStatValue);
        }
      }
    }
    rval = JS_NewFloat64(ctx, result);
  }
  return rval;
}

void InsertStatsToGenericObject(UnitAny* pUnit, StatList* pStatList, JSContext* cx, JSValue pArray) {
  Stat* pStat = NULL;
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

void InsertStatsNow(Stat* pStat, int nStat, JSContext* cx, JSValue pArray) {
  if (pStat[nStat].wSubIndex > 0x200) {
    // subindex is the skill id and level
    int skill = pStat[nStat].wSubIndex >> 6, level = pStat[nStat].wSubIndex & 0x3F, charges = 0, maxcharges = 0;
    if (pStat[nStat].dwStatValue > 0x200) {
      charges = pStat[nStat].dwStatValue & 0xFF;
      maxcharges = pStat[nStat].dwStatValue >> 8;
    }
    JSValue val = JS_NewObject(cx);
    JSValue jsskill = JS_NewInt32(cx, skill), jslevel = JS_NewInt32(cx, level), jscharges = JS_NewInt32(cx, charges), jsmaxcharges = JS_NewInt32(cx, maxcharges);
    // val is an anonymous object that holds properties
    JS_SetPropertyStr(cx, val, "skill", jsskill);
    JS_SetPropertyStr(cx, val, "level", jslevel);
    if (maxcharges > 0) {
      JS_SetPropertyStr(cx, val, "charges", jscharges);
      JS_SetPropertyStr(cx, val, "maxcharges", jsmaxcharges);
    }
    // find where we should put it
    JSValue index = JS_UNDEFINED, obj = val;
    index = JS_GetPropertyUint32(cx, pArray, pStat[nStat].wStatIndex);
    if (index != JS_UNDEFINED) {
      // modify the existing object by stuffing it into an array
      if (!JS_IsArray(cx, index)) {
        // it's not an array, build one
        JSValue arr = JS_NewArray(cx);
        JS_SetPropertyUint32(cx, arr, 0, index);
        JS_SetPropertyUint32(cx, arr, 1, obj);
        JS_SetPropertyUint32(cx, pArray, pStat[nStat].wStatIndex, arr);
      } else {
        // it is an array, append the new value
        uint32_t len = 0;
        if (!JS_GetArrayBuffer(cx, &len, index)) {
          return;
        }
        len++;
        JS_SetPropertyUint32(cx, index, len, val);
      }
    } else {
      JS_SetPropertyUint32(cx, pArray, pStat[nStat].wStatIndex, obj);
    }
  } else {
    // Make sure to bit shift life, mana and stamina properly!
    int value = pStat[nStat].dwStatValue;
    if (pStat[nStat].wStatIndex >= 6 && pStat[nStat].wStatIndex <= 11)
      value = value >> 8;

    JSValue index = JS_GetPropertyUint32(cx, pArray, pStat[nStat].wStatIndex);
    if (JS_IsUndefined(index)) {
      // the array index doesn't exist, make it
      index = JS_NewArray(cx);
      JS_SetPropertyUint32(cx, pArray, pStat[nStat].wStatIndex, index);
    }

    // index now points to the correct array index
    JS_SetPropertyUint32(cx, index, pStat[nStat].wSubIndex, JS_NewInt32(cx, value));
  }
}

JSAPI_FUNC(unit_getState) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  myUnit* lpUnit = (myUnit*)JS_GetPrivate(ctx, this_val);

  if (!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
    return JS_FALSE;

  UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

  if (!pUnit || !JS_IsNumber(argv[0]))
    return JS_FALSE;

  jsint nState;
  if (JS_ToInt32(ctx, &nState, argv[0])) {
    return JS_FALSE;
  }

  // TODO: make these constants so we know what we're checking here
  if (nState > 183 || nState < 0)
    return JS_FALSE;

  return JS_NewBool(ctx, D2COMMON_GetUnitState(pUnit, nState));
}

JSAPI_FUNC(item_getFlags) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  myUnit* lpUnit = (myUnit*)JS_GetPrivate(ctx, this_val);

  if (!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
    return JS_UNDEFINED;

  UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

  if (!pUnit || pUnit->dwType != UNIT_ITEM)
    return JS_UNDEFINED;

  return JS_NewInt32(ctx, pUnit->pItemData->dwFlags);
}

JSAPI_FUNC(item_getFlag) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  if (argc < 1 || !JS_IsNumber(argv[0]))
    return JS_UNDEFINED;

  myUnit* lpUnit = (myUnit*)JS_GetPrivate(ctx, this_val);

  if (!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
    return JS_UNDEFINED;

  UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

  if (!pUnit || pUnit->dwType != UNIT_ITEM)
    return JS_UNDEFINED;

  jsint nFlag;
  JS_ToInt32(ctx, &nFlag, argv[0]);

  return JS_NewBool(ctx, !!(nFlag & pUnit->pItemData->dwFlags));
}

JSAPI_FUNC(item_getItemCost) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  jsint nMode;
  UnitAny* npc = D2CLIENT_GetCurrentInteractingNPC();
  jsint nNpcClassId = (npc ? npc->dwTxtFileNo : 0x9A);  // defaults to Charsi's NPC id
  jsint nDifficulty = D2CLIENT_GetDifficulty();

  if (argc < 1 || !JS_IsNumber(argv[0]))
    return JS_UNDEFINED;

  myUnit* lpUnit = (myUnit*)JS_GetPrivate(ctx, this_val);

  if (!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
    return JS_UNDEFINED;

  UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

  if (!pUnit || pUnit->dwType != UNIT_ITEM)
    return JS_UNDEFINED;

  JS_ToInt32(ctx, &nMode, argv[0]);

  if (argc > 1) {
    if (JS_IsObject(argv[1])) {
      myUnit* pmyNpc = (myUnit*)JS_GetPrivate(ctx, argv[1]);

      if (!pmyNpc || (pmyNpc->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
        return JS_UNDEFINED;

      UnitAny* pNpc = D2CLIENT_FindUnit(pmyNpc->dwUnitId, pmyNpc->dwType);

      if (!pNpc)
        return JS_UNDEFINED;
      nNpcClassId = pNpc->dwTxtFileNo;
    } else if (JS_IsNumber(argv[1]) && !JS_IsNull(argv[1])) {
      if (JS_ToInt32(ctx, &nNpcClassId, argv[1])) {
        return JS_UNDEFINED;
      }
    }
    // TODO:: validate the base stat table sizes to make sure the game doesn't crash with checking values past the end of the table
    int retVal = 0;
    if (FillBaseStat("monstats", nNpcClassId, "inventory", &retVal, sizeof(int)) && retVal == 0)
      nNpcClassId = 0x9A;  // invalid npcid incoming! default to charsi to allow the game to continue
  }

  if (argc > 2 && JS_IsNumber(argv[2]))
    JS_ToInt32(ctx, &nDifficulty, argv[2]);

  switch (nMode) {
    case 0:  // Buy
    case 1:  // Sell
      return JS_NewInt32(ctx, D2COMMON_GetItemPrice(D2CLIENT_GetPlayerUnit(), pUnit, nDifficulty, *p_D2CLIENT_ItemPriceList, nNpcClassId, nMode));
      break;
    case 2:  // Repair
      return JS_NewInt32(ctx, D2COMMON_GetItemPrice(D2CLIENT_GetPlayerUnit(), pUnit, nDifficulty, *p_D2CLIENT_ItemPriceList, nNpcClassId, 3));
      break;
    default:
      break;
  }

  return JS_UNDEFINED;
}

JSAPI_FUNC(unit_getItems) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  myUnit* lpUnit = (myUnit*)JS_GetPrivate(ctx, this_val);

  if (!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
    return JS_UNDEFINED;

  UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

  if (!pUnit || !pUnit->pInventory || !pUnit->pInventory->pFirstItem)
    return JS_UNDEFINED;

  JSValue pReturnArray = JS_NewArray(ctx);
  if (JS_IsException(pReturnArray)) {
    return JS_UNDEFINED;
  }

  DWORD dwArrayCount = 0;

  for (UnitAny* pItem = pUnit->pInventory->pFirstItem; pItem; pItem = D2COMMON_GetNextItemFromInventory(pItem), dwArrayCount++) {
    invUnit* pmyUnit = new invUnit;

    if (!pmyUnit)
      continue;

    pmyUnit->_dwPrivateType = PRIVATE_UNIT;
    pmyUnit->szName[0] = NULL;
    pmyUnit->dwMode = pItem->dwMode;
    pmyUnit->dwClassId = pItem->dwTxtFileNo;
    pmyUnit->dwUnitId = pItem->dwUnitId;
    pmyUnit->dwType = UNIT_ITEM;
    pmyUnit->dwOwnerId = pUnit->dwUnitId;
    pmyUnit->dwOwnerType = pUnit->dwType;

    JSValue jsunit = BuildObject(ctx, unit_class_id, unit_methods, _countof(unit_methods), unit_props, _countof(unit_props), pmyUnit);
    if (JS_IsException(jsunit)) {
      THROW_ERROR(ctx, "Failed to build item array");
    }
    JS_SetPropertyUint32(ctx, pReturnArray, dwArrayCount, jsunit);
  }

  return pReturnArray;
}

JSAPI_FUNC(unit_getSkill) {
  if (argc == NULL || argc > 3)
    return JS_UNDEFINED;

  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  bool nCharge = false;
  jsint nSkillId = NULL;
  jsint nExt = NULL;

  myUnit* pmyUnit = (myUnit*)JS_GetPrivate(ctx, this_val);
  if (!pmyUnit)
    return JS_UNDEFINED;

  UnitAny* pUnit = D2CLIENT_FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);
  if (!pUnit)
    return JS_UNDEFINED;

  if (argc >= 1) {
    if (!JS_IsNumber(argv[0]))
      return JS_UNDEFINED;

    JS_ToInt32(ctx, &nSkillId, argv[0]);
  }

  if (argc >= 2) {
    if (!JS_IsNumber(argv[1]))
      return JS_UNDEFINED;

    JS_ToInt32(ctx, &nExt, argv[1]);
  }

  if (argc == 3) {
    if (!JS_IsBool(argv[2]))
      return JS_UNDEFINED;

    nCharge = !!JS_ToBool(ctx, argv[2]);
  }

  if (argc == 1) {
    WORD wLeftSkillId = pUnit->pInfo->pLeftSkill->pSkillInfo->wSkillId;
    WORD wRightSkillId = pUnit->pInfo->pRightSkill->pSkillInfo->wSkillId;
    switch (nSkillId) {
      case 0: {
        int row = 0;
        if (FillBaseStat("skills", wRightSkillId, "skilldesc", &row, sizeof(int)))
          if (FillBaseStat("skilldesc", row, "str name", &row, sizeof(int))) {
            wchar_t* szName = D2LANG_GetLocaleText((WORD)row);
            return JS_NewString(ctx, szName);
          }
      } break;
      case 1: {
        int row = 0;
        if (FillBaseStat("skills", wLeftSkillId, "skilldesc", &row, sizeof(int)))
          if (FillBaseStat("skilldesc", row, "str name", &row, sizeof(int))) {
            wchar_t* szName = D2LANG_GetLocaleText((WORD)row);
            return JS_NewString(ctx, szName);
          }
      } break;

      case 2:
        return JS_NewInt32(ctx, wRightSkillId);
        break;
      case 3:
        return JS_NewInt32(ctx, wLeftSkillId);
        break;
      case 4: {
        JSValue pReturnArray = JS_NewArray(ctx);
        int i = 0;
        for (Skill* pSkill = pUnit->pInfo->pFirstSkill; pSkill; pSkill = pSkill->pNextSkill) {
          JSValue pArrayInsert = JS_NewArray(ctx);
          if (JS_IsException(pArrayInsert))
            continue;

          JS_SetPropertyUint32(ctx, pArrayInsert, 0, JS_NewInt32(ctx, pSkill->pSkillInfo->wSkillId));
          JS_SetPropertyUint32(ctx, pArrayInsert, 1, JS_NewInt32(ctx, pSkill->dwSkillLevel));
          JS_SetPropertyUint32(ctx, pArrayInsert, 2, JS_NewInt32(ctx, D2COMMON_GetSkillLevel(pUnit, pSkill, 1)));

          JS_SetPropertyUint32(ctx, pReturnArray, i, pArrayInsert);
          i++;
        }
        return pReturnArray;
        break;
      }
      default:
        break;
    }
    return JS_FALSE;
  }

  if (pUnit && pUnit->pInfo && pUnit->pInfo->pFirstSkill) {
    for (Skill* pSkill = pUnit->pInfo->pFirstSkill; pSkill; pSkill = pSkill->pNextSkill) {
      if (pSkill->pSkillInfo && pSkill->pSkillInfo->wSkillId == nSkillId) {
        if ((argc == 2 && pSkill->IsCharge == 0) || (argc == 3 && (nCharge == false || pSkill->IsCharge == 1))) {
          return JS_NewInt32(ctx, D2COMMON_GetSkillLevel(pUnit, pSkill, nExt));
        }
      }
    }
  }

  return JS_FALSE;
}

JSAPI_FUNC(item_shop) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  AutoCriticalRoom* cRoom = new AutoCriticalRoom;

  if (*p_D2CLIENT_TransactionDialog != 0 || *p_D2CLIENT_TransactionDialogs != 0 || *p_D2CLIENT_TransactionDialogs_2 != 0) {
    delete cRoom;
    return JS_FALSE;
  }

  myUnit* lpItem = (myUnit*)JS_GetPrivate(ctx, this_val);

  if (!lpItem || (lpItem->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
    delete cRoom;
    return JS_FALSE;
  }

  UnitAny* pItem = D2CLIENT_FindUnit(lpItem->dwUnitId, lpItem->dwType);

  if (!pItem || pItem->dwType != UNIT_ITEM) {
    delete cRoom;
    return JS_FALSE;
  }

  if (!D2CLIENT_GetUIState(UI_NPCSHOP)) {
    delete cRoom;
    return JS_FALSE;
  }

  UnitAny* pNPC = D2CLIENT_GetCurrentInteractingNPC();
  uint32_t dwMode;
  JS_ToUint32(ctx, &dwMode, argv[argc - 1]);

  // Check if we are interacted.
  if (!pNPC) {
    delete cRoom;
    return JS_FALSE;
  }

  // Check for proper mode.
  if ((dwMode != 1) && (dwMode != 2) && (dwMode != 6)) {
    delete cRoom;
    return JS_FALSE;
  }

  // Selling an Item
  if (dwMode == 1) {
    // Check if we own the item!
    if (pItem->pItemData->pOwnerInventory->pOwner->dwUnitId != D2CLIENT_GetPlayerUnit()->dwUnitId) {
      delete cRoom;
      return JS_FALSE;
    }

    D2CLIENT_ShopAction(pNPC, pItem, 1, 0, 0, 1, 1, NULL);
  } else {
    // Make sure the item is owned by the NPC interacted with.
    if (pItem->pItemData->pOwnerInventory->pOwner->dwUnitId != pNPC->dwUnitId) {
      delete cRoom;
      return JS_FALSE;
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

  delete cRoom;
  return JS_TRUE;
}

JSAPI_FUNC(unit_getParent) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  myUnit* lpUnit = (myUnit*)JS_GetPrivate(ctx, this_val);

  if (!lpUnit || (lpUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
    return JS_UNDEFINED;

  UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);

  if (!pUnit)
    return JS_UNDEFINED;

  if (pUnit->dwType == UNIT_MONSTER) {
    DWORD dwOwnerId = D2CLIENT_GetMonsterOwner(pUnit->dwUnitId);
    if (dwOwnerId == -1)
      return JS_UNDEFINED;

    UnitAny* pMonster = GetUnit(NULL, (DWORD)-1, (DWORD)-1, (DWORD)-1, dwOwnerId);
    if (!pMonster)
      return JS_UNDEFINED;

    myUnit* pmyUnit = new myUnit;
    if (!pmyUnit)
      return JS_UNDEFINED;

    pmyUnit->_dwPrivateType = PRIVATE_UNIT;
    pmyUnit->dwUnitId = pMonster->dwUnitId;
    pmyUnit->dwClassId = pMonster->dwTxtFileNo;
    pmyUnit->dwMode = pMonster->dwMode;
    pmyUnit->dwType = pMonster->dwType;
    pmyUnit->szName[0] = NULL;

    return BuildObject(ctx, unit_class_id, unit_methods, _countof(unit_methods), unit_props, _countof(unit_props), pmyUnit);
  } else if (pUnit->dwType == UNIT_OBJECT) {
    if (pUnit->pObjectData) {
      char szBuffer[128] = "";
      strcpy_s(szBuffer, sizeof(szBuffer), pUnit->pObjectData->szOwner);
      return JS_NewString(ctx, szBuffer);
    }
  } else if (pUnit->dwType == UNIT_ITEM) {
    if (pUnit->pItemData && pUnit->pItemData->pOwnerInventory && pUnit->pItemData->pOwnerInventory->pOwner) {
      myUnit* pmyUnit = new myUnit;  // leaks

      if (!pmyUnit)
        return JS_UNDEFINED;

      pmyUnit->_dwPrivateType = PRIVATE_UNIT;
      pmyUnit->dwUnitId = pUnit->pItemData->pOwnerInventory->pOwner->dwUnitId;
      pmyUnit->dwClassId = pUnit->pItemData->pOwnerInventory->pOwner->dwTxtFileNo;
      pmyUnit->dwMode = pUnit->pItemData->pOwnerInventory->pOwner->dwMode;
      pmyUnit->dwType = pUnit->pItemData->pOwnerInventory->pOwner->dwType;
      pmyUnit->szName[0] = NULL;
      return BuildObject(ctx, unit_class_id, unit_methods, _countof(unit_methods), unit_props, _countof(unit_props), pmyUnit);
    }
  } else if (pUnit->dwType == UNIT_MISSILE) {
    auto* pmyUnit = new myUnit;
    if (!pmyUnit)
      return JS_UNDEFINED;

    UnitAny* pOwner = D2COMMON_GetMissileOwnerUnit(pUnit);
    if (!pOwner)
      return JS_UNDEFINED;

    pmyUnit->_dwPrivateType = PRIVATE_UNIT;
    pmyUnit->dwUnitId = pOwner->dwUnitId;
    pmyUnit->dwClassId = pOwner->dwTxtFileNo;
    pmyUnit->dwMode = pOwner->dwMode;
    pmyUnit->dwType = pOwner->dwType;
    pmyUnit->szName[0] = NULL;

    return BuildObject(ctx, unit_class_id, unit_methods, _countof(unit_methods), unit_props, _countof(unit_props), pmyUnit);
  }

  return JS_UNDEFINED;
}

// Works only on players sinces monsters _CANT_ have mercs!
JSAPI_FUNC(unit_getMerc) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  myUnit* lpUnit = (myUnit*)JS_GetPrivate(ctx, this_val);

  if (lpUnit && (lpUnit->_dwPrivateType & PRIVATE_UNIT) == PRIVATE_UNIT) {
    UnitAny* pUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);
    if (pUnit && pUnit->dwType == UNIT_PLAYER) {
      UnitAny* pMerc = GetMercUnit(pUnit);
      if (pMerc) {
        myUnit* pmyUnit = new myUnit;

        pmyUnit->_dwPrivateType = PRIVATE_UNIT;
        pmyUnit->dwUnitId = pMerc->dwUnitId;
        pmyUnit->dwClassId = pMerc->dwTxtFileNo;
        pmyUnit->dwMode = NULL;
        pmyUnit->dwType = UNIT_MONSTER;
        pmyUnit->szName[0] = NULL;

        return BuildObject(ctx, unit_class_id, unit_methods, _countof(unit_methods), unit_props, _countof(unit_props), pmyUnit);
      }
    }
  }
  return JS_NULL;
}

JSAPI_FUNC(unit_getMercHP) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");
  JSValue* rest = 0;
  myUnit* lpUnit = (myUnit*)JS_GetInstancePrivate(ctx, this_val, unit_class_id, rest);
  // myUnit* lpUnit = (myUnit*)JS_GetPrivate(cx, test);

  UnitAny* pUnit = lpUnit ? D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType) : D2CLIENT_GetPlayerUnit();

  if (pUnit) {
    UnitAny* pMerc = GetMercUnit(pUnit);
    if (pMerc) {
      return (pUnit->dwMode == 12 ? JS_NewInt32(ctx, 0) : JS_NewInt32(ctx, D2CLIENT_GetUnitHPPercent(pMerc->dwUnitId)));
    }
  }

  return JS_UNDEFINED;
}

// unit.setSkill( int skillId OR String skillName, int hand [, int itemGlobalId] );
JSAPI_FUNC(unit_setskill) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  uint32_t nSkillId = (WORD)-1;
  BOOL nHand = FALSE;
  DWORD itemId = (DWORD)-1;

  if (argc < 1)
    return JS_FALSE;

  if (JS_IsString(argv[0])) {
    const char* name = JS_ToCString(ctx, argv[0]);
    nSkillId = GetSkillByName(name);
    JS_FreeCString(ctx, name);
  } else if (JS_IsNumber(argv[0]))
    JS_ToUint32(ctx, &nSkillId, argv[0]);
  else
    return JS_FALSE;

  if (JS_IsNumber(argv[1]))
    JS_ToInt32(ctx, &nHand, argv[1]);
  else
    return JS_FALSE;

  if (argc == 3 && JS_IsObject(argv[2])) {
    if (JS_IsInstanceOf(ctx, argv[2], this_val)) {
      myUnit* unit = (myUnit*)JS_GetPrivate(ctx, argv[2]);
      if (unit->dwType == UNIT_ITEM)
        itemId = unit->dwUnitId;
    }
  }

  if (SetSkill(ctx, static_cast<WORD>(nSkillId), nHand, itemId)) {
    return JS_TRUE;
  }

  return JS_FALSE;
}

JSAPI_FUNC(my_overhead) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  myUnit* pmyUnit = (myUnit*)JS_GetPrivate(ctx, this_val);

  if (!pmyUnit || (pmyUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
    return JS_FALSE;

  UnitAny* pUnit = D2CLIENT_FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

  if (!pUnit)
    return JS_FALSE;

  if (!JS_IsNull(argv[0]) && !JS_IsUndefined(argv[0])) {
    const char* lpszText = JS_ToCString(ctx, argv[0]);
    if (!lpszText) {
      return JS_EXCEPTION;
    }
    std::string ansi = UTF8ToANSI(lpszText);
    OverheadMsg* pMsg = D2COMMON_GenerateOverheadMsg(NULL, ansi.c_str(), *p_D2CLIENT_OverheadTrigger);
    if (pMsg) {
      pUnit->pOMsg = pMsg;
    }
    JS_FreeCString(ctx, lpszText);
  }

  return JS_TRUE;
}

JSAPI_FUNC(my_revive) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  BYTE pPacket[] = {0x41};
  D2NET_SendPacket(1, 1, pPacket);
  return JS_UNDEFINED;
}

JSAPI_FUNC(unit_getItem) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  myUnit* pmyUnit = (myUnit*)JS_GetPrivate(ctx, this_val);

  if (!pmyUnit || (pmyUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
    return JS_UNDEFINED;

  UnitAny* pUnit = D2CLIENT_FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

  if (!pUnit || !pUnit->pInventory)
    return JS_UNDEFINED;

  uint32 nClassId = (uint32)-1;
  uint32 nMode = (uint32)-1;
  uint32 nUnitId = (uint32)-1;
  char szName[128] = "";

  if (argc > 0 && JS_IsString(argv[0])) {
    const char* szText = JS_ToCString(ctx, argv[0]);
    strcpy_s(szName, sizeof(szName), szText);
    JS_FreeCString(ctx, szText);
  }

  if (argc > 0 && JS_IsNumber(argv[0]) && !JS_IsNull(argv[0]))
    JS_ToUint32(ctx, &nClassId, argv[0]);

  if (argc > 1 && JS_IsNumber(argv[1]) && !JS_IsNull(argv[1]))
    JS_ToUint32(ctx, &nMode, argv[1]);

  if (argc > 2 && JS_IsNumber(argv[2]) && !JS_IsNull(argv[2]))
    JS_ToUint32(ctx, &nUnitId, argv[2]);

  UnitAny* pItem = GetInvUnit(pUnit, szName, nClassId, nMode, nUnitId);

  if (!pItem)
    return JS_UNDEFINED;

  invUnit* pmyItem = new invUnit;  // leaked?

  if (!pmyItem)
    return JS_UNDEFINED;

  pmyItem->_dwPrivateType = PRIVATE_ITEM;
  pmyItem->dwClassId = nClassId;
  pmyItem->dwMode = nMode;
  pmyItem->dwType = pItem->dwType;
  pmyItem->dwUnitId = pItem->dwUnitId;
  pmyItem->dwOwnerId = pmyUnit->dwUnitId;
  pmyItem->dwOwnerType = pmyUnit->dwType;
  strcpy_s(pmyItem->szName, sizeof(pmyItem->szName), szName);

  return BuildObject(ctx, unit_class_id, unit_methods, _countof(unit_methods), unit_props, _countof(unit_props), pmyItem);
}

JSAPI_FUNC(unit_move) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  myUnit* pmyUnit = (myUnit*)JS_GetPrivate(ctx, this_val);

  if (!pmyUnit || (pmyUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
    return JS_UNDEFINED;

  UnitAny* pUnit = D2CLIENT_FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

  UnitAny* pPlayer = D2CLIENT_GetPlayerUnit();

  if (!pPlayer || !pUnit)
    return JS_UNDEFINED;

  int32 x, y;

  if (pUnit == pPlayer) {
    if (argc < 2)
      return JS_UNDEFINED;

    if (JS_ToInt32(ctx, &x, argv[0]))
      return JS_UNDEFINED;
    if (JS_ToInt32(ctx, &y, argv[1]) == JS_FALSE)
      return JS_UNDEFINED;
  } else {
    x = D2CLIENT_GetUnitX(pUnit);
    y = D2CLIENT_GetUnitY(pUnit);
  }

  ClickMap(0, (WORD)x, (WORD)y, FALSE, NULL);
  Sleep(50);
  ClickMap(2, (WORD)x, (WORD)y, FALSE, NULL);
  //	D2CLIENT_Move((WORD)x, (WORD)y);
  return JS_UNDEFINED;
}

JSAPI_FUNC(unit_getEnchant) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  if (argc < 1 || !JS_IsNumber(argv[0]))
    return JS_UNDEFINED;

  myUnit* pmyUnit = (myUnit*)JS_GetPrivate(ctx, this_val);

  if (!pmyUnit || (pmyUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
    return JS_UNDEFINED;

  UnitAny* pUnit = D2CLIENT_FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

  if (!pUnit || pUnit->dwType != UNIT_MONSTER)
    return JS_UNDEFINED;

  int nEnchant;
  JS_ToInt32(ctx, &nEnchant, argv[0]);
  for (int i = 0; i < 9; i++)
    if (pUnit->pMonsterData->anEnchants[i] == nEnchant) {
      return JS_TRUE;
    }

  return JS_NewInt32(ctx, 0);
}

JSAPI_FUNC(unit_getQuest) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  if (argc < 2 || !JS_IsNumber(argv[0]) || !JS_IsNumber(argv[1]))
    return JS_UNDEFINED;

  jsint nAct;
  jsint nQuest;
  JS_ToInt32(ctx, &nAct, argv[0]);
  JS_ToInt32(ctx, &nQuest, argv[1]);

  return JS_NewInt32(ctx, D2COMMON_GetQuestFlag(D2CLIENT_GetQuestInfo(), nAct, nQuest));
}

JSAPI_FUNC(unit_getMinionCount) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  if (argc < 1 || !JS_IsNumber(argv[0]))
    return JS_UNDEFINED;

  jsint nType;
  JS_ToInt32(ctx, &nType, argv[0]);

  myUnit* pmyUnit = (myUnit*)JS_GetPrivate(ctx, this_val);

  if (!pmyUnit || (pmyUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
    return JS_UNDEFINED;

  UnitAny* pUnit = D2CLIENT_FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

  if (!pUnit || (pUnit->dwType != UNIT_MONSTER && pUnit->dwType != UNIT_PLAYER))
    return JS_UNDEFINED;

  return JS_NewInt32(ctx, D2CLIENT_GetMinionCount(pUnit, (DWORD)nType));
}

JSAPI_FUNC(me_getRepairCost) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  UnitAny* npc = D2CLIENT_GetCurrentInteractingNPC();
  jsint nNpcClassId = (npc ? npc->dwTxtFileNo : 0x9A);

  if (argc > 0 && JS_IsNumber(argv[0]))
    JS_ToInt32(ctx, &nNpcClassId, argv[0]);

  return JS_NewInt32(ctx, D2COMMON_GetRepairCost(NULL, D2CLIENT_GetPlayerUnit(), nNpcClassId, D2CLIENT_GetDifficulty(), *p_D2CLIENT_ItemPriceList, 0));
}
