#include "JSGame.h"

// #include "MPQStats.h"
// #include "Core.h"
// #include "Helpers.h"
// #include "Game.h"
// #include "JSArea.h"
// #include "JSGlobalClasses.h"
// #include "JSRoom.h"
//
// #include "Game/D2Roster.h"
//
// #include <cassert>
// #include <cmath>

#include "Core.h"
#include "CriticalSections.h"
#include "D2Helpers.h"
#include "Game/D2Quests.h"
#include "Game.h"
#include "Globals.h"
#include "JSParty.h"
#include "JSUnit.h"
#include "MapHeader.h"
#include "MPQStats.h"
#include "StringWrap.h"

bool my_copyUnit(JSContext* ctx, JS::CallArgs& args) {
  if (!args.requireAtLeast(ctx, "copyUnit", 1)) {
    return false;
  }
  if (!args[0].isObject()) {
    args.rval().setUndefined();
    return true;
  }

  UnitWrap* wrap_old;
  UNWRAP_OR_RETURN(ctx, &wrap_old, args[0]);
  UnitWrap::UnitData* lpOldUnit = wrap_old->GetData();
  UnitWrap::UnitData* lpNewUnit = nullptr;

  if (!lpOldUnit) {
    args.rval().setUndefined();
    return true;
  }

  if (lpOldUnit->dwPrivateType == PRIVATE_UNIT) {
    lpNewUnit = new UnitWrap::UnitData;
    if (!lpNewUnit) {
      args.rval().setUndefined();
      return true;
    }
    memcpy(lpNewUnit, lpOldUnit, sizeof(UnitWrap::UnitData));
  } else if (lpOldUnit->dwPrivateType == PRIVATE_ITEM) {
    lpNewUnit = new UnitWrap::ItemData;
    if (!lpNewUnit) {
      args.rval().setUndefined();
      return true;
    }
    memcpy(lpNewUnit, lpOldUnit, sizeof(UnitWrap::ItemData));
  }
  if (!lpNewUnit) {
    // this can only be reached if dwPrivateType was invalid
    args.rval().setUndefined();
    return true;
  }

  JS::RootedObject jsunit(ctx, UnitWrap::Instantiate(ctx, lpNewUnit));
  if (!jsunit) {
    delete lpNewUnit;
    lpNewUnit = NULL;
    JS_ReportErrorASCII(ctx, "Couldn't copy unit");
    return false;
  }
  args.rval().setObject(*jsunit);
  return true;
}

bool my_clickMap(JSContext* ctx, JS::CallArgs& args) {
  if (!args.requireAtLeast(ctx, "clickMap", 3)) {
    return false;
  }
  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  uint32_t nClickType = NULL, nShift = NULL, nX = NULL, nY = NULL;

  if (args[0].isNumber())
    JS::ToUint32(ctx, args[0], &nClickType);
  if (args[1].isNumber() || args[1].isBoolean())
    JS::ToUint32(ctx, args[1], &nShift);
  if (args[2].isNumber())
    JS::ToUint32(ctx, args[2], &nX);
  if (args.hasDefined(3) && args[3].isNumber())
    JS::ToUint32(ctx, args[3], &nY);

  if (args.length() == 3 && args[0].isNumber() && (args[1].isNumber() || args[1].isBoolean()) && args[2].isObject() && !args[2].isNull()) {
    UnitWrap* wrap;
    UNWRAP_OR_RETURN(ctx, &wrap, args[2]);
    UnitWrap::UnitData* mypUnit = wrap->GetData();
    if (!mypUnit || (mypUnit->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
      args.rval().setBoolean(false);
      return true;
    }

    D2UnitStrc* pUnit = D2UnitStrc::FindUnit(mypUnit->dwUnitId, mypUnit->dwType);

    if (!pUnit) {
      args.rval().setBoolean(false);
      return true;
    }

    Vars.dwSelectedUnitId = NULL;
    Vars.dwSelectedUnitType = NULL;
    args.rval().setBoolean(ClickMap(nClickType, nX, nY, nShift, pUnit));
    return true;
  } else if (args.length() > 3 && args[0].isNumber() && (args[1].isNumber() || args[1].isBoolean()) && args[2].isNumber() && args[3].isNumber()) {
    args.rval().setBoolean(ClickMap(nClickType, nX, nY, nShift, NULL));
  }

  args.rval().setBoolean(false);
  return true;
}

bool my_acceptTrade(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  // TODO: Fix this nonsense.

  if (args.length() > 0) {
    int32_t test = args[0].toInt32();
    if (test == 1) {  // Called with a '1' it will return if we already accepted it or not
      args.rval().setBoolean(*D2CLIENT_bTradeAccepted);
      return true;
    } else if (test == 2) {  // Called with a '2' it will return the trade flag
      args.rval().setBoolean(*D2CLIENT_RecentTradeId);
      return true;
    } else if (test == 3) {  // Called with a '3' it will return if the 'check' is red or not
      args.rval().setBoolean(*D2CLIENT_bTradeBlock);
      return true;
    }
  }

  AutoCriticalRoom cRoom;

  if ((*D2CLIENT_RecentTradeId) == 3 || (*D2CLIENT_RecentTradeId) == 5 || (*D2CLIENT_RecentTradeId) == 7) {
    if ((*D2CLIENT_bTradeBlock)) {
      // Don't operate if we can't trade anyway ...
      args.rval().setBoolean(false);
      return true;
    } else if ((*D2CLIENT_bTradeAccepted)) {
      (*D2CLIENT_bTradeAccepted) = FALSE;
      D2CLIENT_CancelTrade();
      args.rval().setBoolean(true);
      return true;
    } else {
      (*D2CLIENT_bTradeAccepted) = TRUE;
      D2CLIENT_AcceptTrade();
      args.rval().setBoolean(true);
      return true;
    }
    // NOTE: never reaches this point
  }
  JS_ReportErrorASCII(ctx, "Invalid parameter passed to acceptTrade!");
  return false;
}

bool my_tradeOk(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  AutoCriticalRoom cRoom;
  D2NPCMessageTableStrc* pTdi = *D2CLIENT_pTransactionDialogsInfo;
  unsigned int i;

  if (pTdi != NULL) {
    for (i = 0; i < pTdi->numLines; ++i) {
      // Not sure if *p_D2CLIENT_TransactionDialogs == 1 necessary if it's in
      // the dialog list, but if it's not 1, a crash is guaranteed. (CrazyCasta)
      if (pTdi->dialogLines[i].handler == D2CLIENT_TradeOK && *D2CLIENT_TransactionDialogs == 1) {
        D2CLIENT_TradeOK();
        args.rval().setBoolean(true);
        return true;
      }
    }
  }
  JS_ReportErrorASCII(ctx, "Not in proper state to click ok to trade.");
  return false;
}

bool my_getDialogLines(JSContext* ctx, JS::CallArgs& args) {
  D2NPCMessageTableStrc* pTdi = *D2CLIENT_pTransactionDialogsInfo;
  unsigned int i;

  AutoCriticalRoom cRoom;

  if (pTdi == NULL) {
    args.rval().setUndefined();
    return true;
  }

  JS::RootedObject pReturnArray(ctx, JS::NewArrayObject(ctx, 0));
  for (i = 0; i < pTdi->numLines; ++i) {
    JS::RootedObject line(ctx, JS_NewPlainObject(ctx));
    if (!line) {
      THROW_ERROR(ctx, "failed to instantiate dialog line");
    }

    JS::RootedValue text_val(ctx, JS::StringValue(JS_NewUCStringCopyZ(ctx, reinterpret_cast<const char16_t*>(pTdi->dialogLines[i].text))));
    JS_DefineProperty(ctx, line, "index", i, JSPROP_ENUMERATE);
    JS_DefineProperty(ctx, line, "text", text_val, JSPROP_ENUMERATE);
    JS_DefineProperty(ctx, line, "selectable", static_cast<bool>(pTdi->dialogLines[i].bMaybeSelectable), JSPROP_ENUMERATE);
    JS_DefineFunction(ctx, line, "handler", trampoline<my_clickDialog>, 0, JSPROP_ENUMERATE);
    JS_SetElement(ctx, pReturnArray, i, line);
  }
  args.rval().setObject(*pReturnArray);
  return true;
}

bool my_clickDialog(JSContext* ctx, JS::CallArgs& args) {
  D2NPCMessageTableStrc* pTdi = *D2CLIENT_pTransactionDialogsInfo;
  D2NPCMessageStrc* tdl;

  JS::RootedObject obj(ctx, args.thisv().toObjectOrNull());
  JS::RootedValue index_val(ctx);
  if (!JS_GetProperty(ctx, obj, "index", &index_val) || !index_val.isNumber()) {
    args.rval().setBoolean(false);
    return true;
  }
  uint32_t index;
  JS::ToUint32(ctx, index_val, &index);
  if (index >= pTdi->numLines) {
    args.rval().setBoolean(false);
    return true;
  }

  tdl = &pTdi->dialogLines[index];
  if (tdl != NULL && tdl->bMaybeSelectable) {
    tdl->handler();
  } else {
    JS_ReportErrorASCII(ctx, "That dialog is not currently clickable.");
    return false;
  }

  args.rval().setBoolean(true);
  return true;
}

bool my_getPath(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  if (args.length() < 5) {
    JS_ReportErrorASCII(ctx, "Not enough parameters were passed to getPath!");
    return false;
  }

  uint32_t lvl = 0, x = 0, y = 0, dx = 0, dy = 0, reductionType = 0, radius = 20;
  uint32_t* tmp[] = {&lvl, &x, &y, &dx, &dy, &reductionType, &radius};
  for (uint32_t i = 0; i < args.length() && i < _countof(tmp); ++i) {
    if (args[i].isNumber() && !JS::ToUint32(ctx, args[i], tmp[i])) {
      THROW_ERROR(ctx, "failed to convert argument");
    }
  }

  if (reductionType == 3 && args.length() == 9 &&
      !(JS::IsCallable(args[7].toObjectOrNull()) && JS::IsCallable(args[8].toObjectOrNull()) && JS::IsCallable(args[9].toObjectOrNull()))) {
    JS_ReportErrorASCII(ctx, "Invalid function values for reduction type");
    return false;
  }
  if (lvl == 0) {
    JS_ReportErrorASCII(ctx, "Invalid level passed to getPath");
    return false;
  }

  D2DrlgLevelStrc* level = D2DrlgLevelStrc::FindLevelFromLevelId(lvl);
  if (!level) {
    THROW_ERROR(ctx, "failed to find level");
  }

  ActMap* map = ActMap::GetMap(level);

  Point start(x, y), end(dx, dy);

  PathReducer* reducer = NULL;
  switch (reductionType) {
    case 0:
      reducer = new WalkPathReducer(map, DiagonalShortcut, radius);
      break;
    case 1:
      reducer = new TeleportPathReducer(map, DiagonalShortcut, radius);
      break;
    case 2:
      reducer = new NoPathReducer(map);
      break;
    case 3:
      // reducer = new JSPathReducer(map, ctx, this_val, argv[7], argv[8], argv[9]);
      break;
    default:
      JS_ReportErrorASCII(ctx, "Invalid path reducer value!");
      return false;
      break;
  }

  PointList list;
  AStarPath<> path(map, reducer);

  // box18jsrefcount depth = JS_SuspendRequest(cx);

  path.GetPath(start, end, list, true);
  map->CleanUp();
  // box18JS_ResumeRequest(cx, depth);

  int count = list.size();

  JS::RootedObject arr(ctx, JS::NewArrayObject(ctx, count));
  for (int i = 0; i < count; i++) {
    JS::RootedObject point(ctx, JS_NewPlainObject(ctx));
    JS_DefineProperty(ctx, point, "x", list[i].first, JSPROP_ENUMERATE);
    JS_DefineProperty(ctx, point, "y", list[i].second, JSPROP_ENUMERATE);
    JS_SetElement(ctx, arr, i, point);
  }
  args.rval().setObject(*arr);

  delete reducer;
  map->CleanUp();
  return true;
}

bool my_getCollision(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  uint32_t nLevelId, nX, nY;
  uint32_t* tmp[] = {&nLevelId, &nX, &nY};
  for (uint32_t i = 0; i < args.length() && i < _countof(tmp); ++i) {
    if (args[i].isNumber() && !JS::ToUint32(ctx, args[i], tmp[i])) {
      THROW_ERROR(ctx, "failed to convert argument");
    }
  }

  Point point(nX, nY);
  D2DrlgLevelStrc* level = D2DrlgLevelStrc::FindLevelFromLevelId(nLevelId);
  if (!level) {
    JS_ReportErrorASCII(ctx, "Level Not loaded");
    return false;
  }

  ActMap* map = ActMap::GetMap(level);
  // if(!map->IsValidPoint(point))  //return avoid instead and make it not lvl depenant
  //	{ map->CleanUp(); THROW_ERROR(cx, "Invalid point!");}

  args.rval().setInt32(map->GetMapData(point, true));
  map->CleanUp();
  return true;
}

bool my_clickItem(JSContext* ctx, JS::CallArgs& args) {
  args.rval().setNull();
  typedef void __fastcall clickequip(D2UnitStrc * pPlayer, D2InventoryStrc * pIventory, int loc);

  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  AutoCriticalRoom cRoom;

  if (*D2CLIENT_TransactionDialog != 0 || *D2CLIENT_TransactionDialogs != 0 || *D2CLIENT_TransactionDialogs_2 != 0) {
    args.rval().setBoolean(false);
    return true;
  }

  D2UnitStrc* pUnit = NULL;

  // int ScreenSize = D2GFX_GetScreenSize();

  POINT Belt[] = {
      {0, 0},  // 0
      {1, 0},  // 1
      {2, 0},  // 2
      {3, 0},  // 3

      {0, 1},  // 4
      {1, 1},  // 5
      {2, 1},  // 6
      {3, 1},  // 7

      {0, 2},  // 8
      {1, 2},  // 9
      {2, 2},  // 10
      {3, 2},  // 11

      {0, 3},  // 12
      {1, 3},  // 13
      {2, 3},  // 14
      {3, 3},  // 15
  };

  *D2CLIENT_CursorHoverX = 0xFFFFFFFF;
  *D2CLIENT_CursorHoverY = 0xFFFFFFFF;

  if (args.length() == 1 && args[0].isObject()) {
    UnitWrap* wrap;
    UNWRAP_OR_RETURN(ctx, &wrap, args[0]);
    UnitWrap::UnitData* pmyUnit = wrap->GetData();

    if (!pmyUnit || (pmyUnit->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
      return true;
    }

    pUnit = D2UnitStrc::FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

    if (!pUnit) {
      return true;
    }

    clickequip* click = (clickequip*)*(DWORD*)(D2CLIENT_BodyClickTable + (4 * pUnit->pItemData->BodyLocation));

    if (!click) {
      return true;
    }

    click(D2CLIENT_GetPlayerUnit(), D2CLIENT_GetPlayerUnit()->pInventory, pUnit->pItemData->BodyLocation);
    return true;
  } else if (args.length() == 2 && args[0].isInt32() && args[1].isInt32()) {
    int32_t nClickType = args[0].toInt32();
    int32_t nBodyLoc = args[1].toInt32();

    if (nClickType == NULL) {
      clickequip* click = (clickequip*)*(DWORD*)(D2CLIENT_BodyClickTable + (4 * nBodyLoc));

      if (!click) {
        return true;
      }

      click(D2CLIENT_GetPlayerUnit(), D2CLIENT_GetPlayerUnit()->pInventory, nBodyLoc);

    }
    // Click Merc Gear
    else if (nClickType == 4) {
      if (nBodyLoc == 1 || nBodyLoc == 3 || nBodyLoc == 4) {
        D2UnitStrc* pMerc = D2CLIENT_GetPlayerUnit()->FindMercUnit();
        if (pMerc) {
          D2CLIENT_MercItemAction(0x61, nBodyLoc);
          args.rval().setBoolean(true);
        }
      }
    }
    return true;
  } else if (args.length() == 2 && args[0].isInt32() && args[1].isObject()) {
    UnitWrap* wrap;
    UNWRAP_OR_RETURN(ctx, &wrap, args[1]);
    UnitWrap::UnitData* pmyUnit = wrap->GetData();

    if (!pmyUnit || (pmyUnit->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
      return true;
    }

    pUnit = D2UnitStrc::FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

    int32_t nClickType = args[0].toInt32();

    if (!pUnit || !(pUnit->dwType == UNIT_ITEM) || !pUnit->pItemData) {
      JS_ReportErrorASCII(ctx, "Object is not an item!");
      return false;
    }

    int InventoryLocation = pUnit->GetItemLocation();
    int ClickLocation = LOCATION_NULL;

    int x = pUnit->pItemPath->dwPosX;
    int y = pUnit->pItemPath->dwPosY;

    *D2CLIENT_CursorHoverX = x;
    *D2CLIENT_CursorHoverY = y;

    InventoryLayout* pLayout = NULL;

    if (nClickType == 4) {
      D2UnitStrc* pMerc = D2CLIENT_GetPlayerUnit()->FindMercUnit();

      if (pMerc)
        if (pUnit->pItemData && pUnit->pItemData->pOwner)
          if (pUnit->pItemData->pOwner->dwUnitId == pMerc->dwUnitId) {
            args.rval().setBoolean(true);
            D2CLIENT_MercItemAction(0x61, pUnit->pItemData->BodyLocation);
          }
      return true;
    } else if (InventoryLocation == LOCATION_INVENTORY || InventoryLocation == LOCATION_STASH || InventoryLocation == LOCATION_CUBE) {
      switch (InventoryLocation) {
        case LOCATION_INVENTORY:
          pLayout = (InventoryLayout*)D2CLIENT_InventoryLayout;
          ClickLocation = CLICKTARGET_INVENTORY;
          break;
        case LOCATION_STASH:
          pLayout = (InventoryLayout*)D2CLIENT_StashLayout;
          ClickLocation = CLICKTARGET_STASH;
          break;
        case LOCATION_CUBE:
          pLayout = (InventoryLayout*)D2CLIENT_CubeLayout;
          ClickLocation = CLICKTARGET_CUBE;
          break;
      }

      x = pLayout->Left + x * pLayout->SlotPixelWidth + 10;
      y = pLayout->Top + y * pLayout->SlotPixelHeight + 10;

      if (nClickType == NULL)
        D2CLIENT_LeftClickItem(ClickLocation, D2CLIENT_GetPlayerUnit(), D2CLIENT_GetPlayerUnit()->pInventory, x, y, nClickType, pLayout);
      // D2CLIENT_LeftClickItem(D2CLIENT_GetPlayerUnit(), D2CLIENT_GetPlayerUnit()->pInventory, x, y, nClickType, pLayout,
      // pUnit->pItemData->ItemLocation);
      else
        D2CLIENT_RightClickItem(x, y, ClickLocation, D2CLIENT_GetPlayerUnit(), D2CLIENT_GetPlayerUnit()->pInventory);
      // D2CLIENT_RightClickItem(x,y, pUnit->pItemData->ItemLocation, D2CLIENT_GetPlayerUnit(), D2CLIENT_GetPlayerUnit()->pInventory);

    } else if (InventoryLocation == LOCATION_BELT) {
      int i = x;

      if (i < 0 || i > 0x0F) {
        return true;
      }

      if (D2GFX_GetScreenSize() == 2) {
        x = 440 + (Belt[i].x * 29);
        y = 580 - (Belt[i].y * 29);
      } else {
        x = 360 + (Belt[i].x * 29);
        y = 460 - (Belt[i].y * 29);
      }
      if (nClickType == NULL)
        D2CLIENT_ClickBelt(x, y, D2CLIENT_GetPlayerUnit()->pInventory);
      else
        D2CLIENT_ClickBeltRight(D2CLIENT_GetPlayerUnit()->pInventory, D2CLIENT_GetPlayerUnit(), nClickType == 1 ? FALSE : TRUE, i);
    } else if (D2CLIENT_GetCursorItem() == pUnit) {
      if (nClickType < 1 || nClickType > 12) {
        return true;
      }

      clickequip* click = (clickequip*)*(DWORD*)(D2CLIENT_BodyClickTable + (4 * nClickType));

      if (!click) {
        return true;
      }

      click(D2CLIENT_GetPlayerUnit(), D2CLIENT_GetPlayerUnit()->pInventory, nClickType);
    }
  } else if (args.length() == 4) {
    if (args[0].isInt32() && args[1].isInt32() && args[2].isInt32() && args[3].isInt32()) {
      int32_t nButton = args[0].toInt32();
      int32_t nX = args[1].toInt32();
      int32_t nY = args[2].toInt32();
      int32_t nLoc = args[3].toInt32();

      int clickTarget = LOCATION_NULL;
      InventoryLayout* pLayout = NULL;

      *D2CLIENT_CursorHoverX = nX;
      *D2CLIENT_CursorHoverY = nY;

      // Fixing the x/y click spot for items taking more than one inventory square- so Diablo can handle it!
      if (nLoc != LOCATION_BELT) {
        D2UnitStrc* pItem = D2CLIENT_GetCursorItem();
        if (pItem) {
          D2ItemsTxt* pTxt = D2COMMON_GetItemText(pItem->dwTxtFileNo);
          if (pTxt) {
            if (pTxt->ySize > 1)
              nY += 1;

            if (pTxt->xSize > 1)
              nX += 1;
          }
        }
      }
      // nLoc is click target locations=: LOCATION_INVENTORY=inventory, LOCATION_TRADE=player trade, LOCATION_CUBE=cube, LOCATION_STASH=stash,
      // LOCATION_BELT=belt
      if (nLoc == LOCATION_INVENTORY || nLoc == LOCATION_TRADE || nLoc == LOCATION_CUBE || nLoc == LOCATION_STASH) {
        switch (nLoc) {
          case LOCATION_INVENTORY:
            pLayout = (InventoryLayout*)D2CLIENT_InventoryLayout;
            clickTarget = CLICKTARGET_INVENTORY;
            break;
          case LOCATION_TRADE:
            pLayout = (InventoryLayout*)D2CLIENT_TradeLayout;
            clickTarget = CLICKTARGET_TRADE;
            break;
          case LOCATION_CUBE:
            pLayout = (InventoryLayout*)D2CLIENT_CubeLayout;
            clickTarget = CLICKTARGET_CUBE;
            break;
          case LOCATION_STASH:
            pLayout = (InventoryLayout*)D2CLIENT_StashLayout;
            clickTarget = CLICKTARGET_STASH;
            break;
        }

        int x = pLayout->Left + nX * pLayout->SlotPixelWidth + 10;
        int y = pLayout->Top + nY * pLayout->SlotPixelHeight + 10;

        if (nButton == 0)  // Left Click
          D2CLIENT_LeftClickItem(clickTarget, D2CLIENT_GetPlayerUnit(), D2CLIENT_GetPlayerUnit()->pInventory, x, y, 1, pLayout);
        else if (nButton == 1)  // Right Click
          D2CLIENT_RightClickItem(x, y, clickTarget, D2CLIENT_GetPlayerUnit(), D2CLIENT_GetPlayerUnit()->pInventory);
        else if (nButton == 2)  // Shift Left Click
          D2CLIENT_LeftClickItem(clickTarget, D2CLIENT_GetPlayerUnit(), D2CLIENT_GetPlayerUnit()->pInventory, x, y, 5, pLayout);

        args.rval().setBoolean(true);
        return true;
      } else if (nLoc == LOCATION_BELT)  // Belt
      {
        int z = -1;

        for (UINT i = 0; i < _countof(Belt); i++) {
          if (Belt[i].x == nX && Belt[i].y == nY) {
            z = (int)i;
            break;
          }
        }

        if (z == -1) {
          return true;
        }

        int x = NULL;
        int y = NULL;

        if (D2GFX_GetScreenSize() == 2) {
          x = 440 + (Belt[z].x * 29);
          y = 580 - (Belt[z].y * 29);
        } else {
          x = 360 + (Belt[z].x * 29);
          y = 460 - (Belt[z].y * 29);
        }

        if (nButton == 0)
          D2CLIENT_ClickBelt(x, y, D2CLIENT_GetPlayerUnit()->pInventory);
        else if (nButton == 1)
          D2CLIENT_ClickBeltRight(D2CLIENT_GetPlayerUnit(), D2CLIENT_GetPlayerUnit()->pInventory, FALSE, z);
        else if (nButton == 2)
          D2CLIENT_ClickBeltRight(D2CLIENT_GetPlayerUnit(), D2CLIENT_GetPlayerUnit()->pInventory, TRUE, z);
        args.rval().setBoolean(true);
        return true;
      }
    }
  }
  args.rval().setBoolean(true);
  return true;
}

bool my_getLocaleString(JSContext* ctx, JS::CallArgs& args) {
  if (args.length() < 1 || !args[0].isNumber()) {
    args.rval().setUndefined();
    return true;
  }

  uint32_t localeId;
  JS::ToUint32(ctx, args[0], &localeId);
  wchar_t* wString = D2LANG_GetLocaleText(static_cast<uint16_t>(localeId));
  args.rval().setString(JS_NewUCStringCopyZ(ctx, reinterpret_cast<const char16_t*>(wString)));
  return true;
}

bool my_rand(JSContext* ctx, JS::CallArgs& args) {
  if (args.length() < 2 || !args[0].isInt32() || !args[1].isInt32()) {
    args.rval().setInt32(0);
    return true;
  }

  // only seed the rng once
  static bool seeded = false;
  if (!seeded) {
    srand(GetTickCount());
    seeded = true;
  }

  uint32_t seed = 0;
  if (ClientState() == ClientStateInGame)
    seed = D2GAME_Rand(D2CLIENT_GetPlayerUnit()->dwSeed);
  else
    seed = rand();

  int32_t low = args[0].toInt32();
  int32_t high = args[1].toInt32();

  if (high > low + 1) {
    int i = (seed % (high - low + 1)) + low;
    args.rval().setInt32(i);
    return true;
  }

  args.rval().setInt32(high);
  return true;
}

bool my_getDistance(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  // TODO: Add the type of distance to the api design
  int32_t nX1 = NULL;
  int32_t nX2 = NULL;
  int32_t nY1 = NULL;
  int32_t nY2 = NULL;

  if (args.length() == 1 && args[0].isObject()) {
    JS::RootedObject obj(ctx, args[0].toObjectOrNull());
    JS::RootedValue x1_val(ctx);
    JS::RootedValue y1_val(ctx);
    if (!JS_GetProperty(ctx, obj, "x", &x1_val)) {
      args.rval().setUndefined();
      return true;
    }
    if (!JS_GetProperty(ctx, obj, "y", &y1_val)) {
      args.rval().setUndefined();
      return true;
    }
    if (x1_val.isInt32() && y1_val.isInt32()) {
      nX1 = D2CLIENT_GetUnitX(D2CLIENT_GetPlayerUnit());
      nY1 = D2CLIENT_GetUnitY(D2CLIENT_GetPlayerUnit());
      nX2 = x1_val.toInt32();
      nY2 = y1_val.toInt32();
    }
  } else if (args.length() == 2) {
    if (args[0].isInt32() && args[1].isInt32()) {
      nX1 = D2CLIENT_GetUnitX(D2CLIENT_GetPlayerUnit());
      nY1 = D2CLIENT_GetUnitY(D2CLIENT_GetPlayerUnit());
      nX2 = args[0].toInt32();
      nY2 = args[1].toInt32();
    } else if (args[0].isObject() && args[1].isObject()) {
      // get [x, y] from arg0
      JS::RootedObject obj1(ctx, args[0].toObjectOrNull());
      JS::RootedValue x_val(ctx);
      JS::RootedValue y_val(ctx);
      if (!JS_GetProperty(ctx, obj1, "x", &x_val)) {
        args.rval().setUndefined();
        return true;
      }
      if (!JS_GetProperty(ctx, obj1, "y", &y_val)) {
        args.rval().setUndefined();
        return true;
      }

      // get [x, y] from arg1
      JS::RootedObject obj2(ctx, args[1].toObjectOrNull());
      JS::RootedValue x2_val(ctx);
      JS::RootedValue y2_val(ctx);
      if (!JS_GetProperty(ctx, obj2, "x", &x2_val)) {
        args.rval().setUndefined();
        return true;
      }
      if (!JS_GetProperty(ctx, obj2, "y", &y2_val)) {
        args.rval().setUndefined();
        return true;
      }
      if (x_val.isInt32() && y_val.isInt32() && x2_val.isInt32() && y2_val.isInt32()) {
        nX1 = x_val.toInt32();
        nY1 = y_val.toInt32();
        nX2 = x2_val.toInt32();
        nY2 = y2_val.toInt32();
      }
    }
  } else if (args.length() == 3) {
    if (args[0].isObject() && args[1].isInt32() && args[2].isInt32()) {
      UnitWrap* unit_wrap;
      UNWRAP_OR_RETURN(ctx, &unit_wrap, args[0]);
      UnitWrap::UnitData* pUnit1 = unit_wrap->GetData();
      if (!pUnit1 || (pUnit1->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
        args.rval().setUndefined();
        return true;
      }

      D2UnitStrc* pUnitA = D2UnitStrc::FindUnit(pUnit1->dwUnitId, pUnit1->dwType);

      if (!pUnitA) {
        args.rval().setUndefined();
        return true;
      }

      nX1 = D2CLIENT_GetUnitX(pUnitA);
      nY1 = D2CLIENT_GetUnitY(pUnitA);
      nX2 = args[1].toInt32();
      nY2 = args[2].toInt32();
    } else if (args[0].isInt32() && args[1].isInt32() && args[2].isObject()) {
      UnitWrap* unit_wrap;
      UNWRAP_OR_RETURN(ctx, &unit_wrap, args[2]);
      UnitWrap::UnitData* pUnit1 = unit_wrap->GetData();
      if (!pUnit1 || (pUnit1->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
        args.rval().setUndefined();
        return true;
      }

      D2UnitStrc* pUnitA = D2UnitStrc::FindUnit(pUnit1->dwUnitId, pUnit1->dwType);

      if (!pUnitA) {
        args.rval().setUndefined();
        return true;
      }
      nX1 = D2CLIENT_GetUnitX(pUnitA);
      nY1 = D2CLIENT_GetUnitY(pUnitA);
      nX2 = args[0].toInt32();
      nY2 = args[1].toInt32();
    }
  } else if (args.length() == 4) {
    if (args[0].isInt32() && args[1].isInt32() && args[2].isInt32() && args[3].isInt32()) {
      nX1 = args[0].toInt32();
      nY1 = args[1].toInt32();
      nX2 = args[2].toInt32();
      nY2 = args[3].toInt32();
    }
  }
  args.rval().setNumber(abs(GetDistance(nX1, nY1, nX2, nY2)));
  return true;
}

bool my_gold(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  int32_t nGold = NULL;
  int32_t nMode = 1;

  if (args.length() > 0 && args[0].isInt32())
    nGold = args[0].toInt32();

  if (args.length() > 1 && args[1].isInt32())
    nMode = args[1].toInt32();

  SendGold(nGold, nMode);
  args.rval().setUndefined();
  return true;
}

bool my_checkCollision(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  if (args.length() == 3 && args[0].isObject() && args[1].isObject() && args[2].isInt32()) {
    UnitWrap* unit_wrap1;
    UNWRAP_OR_RETURN(ctx, &unit_wrap1, args[0]);
    UnitWrap* unit_wrap2;
    UNWRAP_OR_RETURN(ctx, &unit_wrap2, args[1]);
    UnitWrap::UnitData* pUnitA = unit_wrap1->GetData();
    UnitWrap::UnitData* pUnitB = unit_wrap2->GetData();
    int32_t nBitMask = args[2].toInt32();

    if (!pUnitA || (pUnitA->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT || !pUnitB || (pUnitB->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
      args.rval().setUndefined();
      return true;
    }

    D2UnitStrc* pUnit1 = D2UnitStrc::FindUnit(pUnitA->dwUnitId, pUnitA->dwType);
    D2UnitStrc* pUnit2 = D2UnitStrc::FindUnit(pUnitB->dwUnitId, pUnitB->dwType);

    if (!pUnit1 || !pUnit2) {
      args.rval().setUndefined();
      return true;
    }
    args.rval().setInt32(D2COMMON_CheckUnitCollision(pUnit1, pUnit2, (WORD)nBitMask));
    return true;
  }

  args.rval().setUndefined();
  return true;
}

bool my_getCursorType(JSContext* ctx, JS::CallArgs& args) {
  int32_t nType = NULL;

  if (args.length() > 0)
    nType = args[0].toInt32();

  args.rval().setInt32(nType == 1 ? *D2CLIENT_ShopCursorType : *D2CLIENT_RegularCursorType);
  return true;
}

bool my_getSkillByName(JSContext* ctx, JS::CallArgs& args) {
  if (args.length() < 1 || !args[0].isString()) {
    args.rval().setUndefined();
    return true;
  }

  StringWrap lpszText(ctx, args[0]);
  if (!lpszText || lpszText[0]) {
    JS_ReportErrorASCII(ctx, "Could not convert string");
    return false;
  }

  D2UnitStrc* player = D2CLIENT_GetPlayerUnit();
  if (!player) {
    args.rval().setUndefined();
    return true;
  }

  D2SkillStrc* skill = player->GetSkillFromSkillName(lpszText);
  if (!skill) {
    args.rval().setUndefined();
    return true;
  }

  args.rval().setInt32(skill->pSkillInfo->wSkillId);
  return true;
}

bool my_getSkillById(JSContext* ctx, JS::CallArgs& args) {
  if (args.length() < 1 || !args[0].isInt32()) {
    args.rval().setUndefined();
    return true;
  }

  int32_t nId = args[0].toInt32();

  int row = 0;
  if (FillBaseStat("skills", nId, "skilldesc", &row, sizeof(int))) {
    if (FillBaseStat("skilldesc", row, "str name", &row, sizeof(int))) {
      wchar_t* szName = D2LANG_GetLocaleText((WORD)row);
      args.rval().setString(JS_NewUCStringCopyZ(ctx, reinterpret_cast<const char16_t*>(szName)));
      return true;
    }
  }
  args.rval().setString(JS_NewStringCopyZ(ctx, "Unknown"));
  return true;
}

bool my_getTextSize(JSContext* ctx, JS::CallArgs& args) {
  if (args.length() < 2 || !args[0].isString() || !args[1].isInt32()) {
    args.rval().setBoolean(false);
    return true;
  }

  StringWrap szString(ctx, args[0]);
  if (!szString) {
    THROW_ERROR(ctx, "failed to encode string");
  }

  int font = args[1].toInt32();
  POINT r = CalculateTextLen(szString.c_str(), font);
  JS::RootedValue x(ctx, JS::NumberValue(r.x));
  JS::RootedValue y(ctx, JS::NumberValue(r.y));

  if (args.length() > 2 && args[2].isBoolean() && args[2].toBoolean() == TRUE) {
    // return an object with a height/width rather than an array
    JS::RootedObject pObj(ctx, JS_NewPlainObject(ctx));
    JS_SetProperty(ctx, pObj, "width", x);
    JS_SetProperty(ctx, pObj, "height", y);
    args.rval().setObject(*pObj);
    return true;
  }

  JS::RootedObject pObj(ctx, JS::NewArrayObject(ctx, 2));
  JS_SetElement(ctx, pObj, 0, x);
  JS_SetElement(ctx, pObj, 1, y);
  args.rval().setObject(*pObj);
  return true;
}

bool my_getTradeInfo(JSContext* ctx, JS::CallArgs& args) {
  if (args.length() < 1) {
    args.rval().setBoolean(false);
    return true;
  }

  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  if (!args[0].isInt32()) {
    args.rval().setBoolean(false);
    return true;
  }

  int32_t nMode = args[0].toInt32();
  switch (nMode) {
    case 0:
      args.rval().setInt32(*D2CLIENT_RecentTradeId);
      return true;
    case 1:
      // FIXME
      // char* tmp = UnicodeToAnsi((wchar_t*)(*p_D2CLIENT_RecentTradeName));
      //*rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, tmp));
      // delete[] tmp;
      // Temporary return value to keep it kosher
      args.rval().setNull();
      return true;
    case 2:
      args.rval().setInt32(*D2CLIENT_RecentTradeId);
      return true;
  }

  args.rval().setBoolean(false);
  return true;
}

bool my_getUIFlag(JSContext* ctx, JS::CallArgs& args) {
  if (args.length() < 1 || !args[0].isInt32()) {
    args.rval().setUndefined();
    return true;
  }

  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  int32_t nUIId = args[0].toInt32();
  args.rval().setBoolean(D2CLIENT_GetUIState(nUIId));
  return true;
}

bool my_getWaypoint(JSContext* ctx, JS::CallArgs& args) {
  if (args.length() < 1 || !args[0].isInt32()) {
    args.rval().setBoolean(false);
    return true;
  }

  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  int32_t nWaypointId = args[0].toInt32();

  if (nWaypointId > 40)
    nWaypointId = NULL;

  args.rval().setBoolean(D2COMMON_CheckWaypoint((*D2CLIENT_WaypointTable), nWaypointId));
  return true;
}

bool my_quitGame(JSContext* ctx, JS::CallArgs& args) {
  if (ClientState() != ClientStateMenu)
    D2CLIENT_ExitGame();

  // give the core a chance to shut down
  // TODO(ejt): commented out Shutdown while refactoring the Engine
  // Shutdown();
  TerminateProcess(GetCurrentProcess(), 0);

  args.rval().setBoolean(false);
  return true;
}

bool my_quit(JSContext* ctx, JS::CallArgs& args) {
  Vars.bQuitting = true;
  if (ClientState() != ClientStateMenu)
    D2CLIENT_ExitGame();

  args.rval().setBoolean(false);
  return true;
}

bool my_playSound(JSContext* ctx, JS::CallArgs& args) {
  // I need to take a closer look at the D2CLIENT_PlaySound function
  // if (args.length() < 1 || !JS_IsNumber(argv[0])) {
  //  args.rval().setBoolean(false);
  //  return true;
  //}

  //// int32_t nSoundId = JSVAL_TO_INT(JS_ARGV(cx, vp)[0]);
  //// D2CLIENT_PlaySound(nSoundId);

  args.rval().setBoolean(true);
  return true;
}

bool my_say(JSContext* ctx, JS::CallArgs& args) {
  for (uint32_t i = 0; i < args.length(); i++) {
    StringWrap str(ctx, JS::ToString(ctx, args[i]));
    if (!str) {
      THROW_ERROR(ctx, "failed to encode string");
    }
    Say(L"%S", str.c_str());
  }
  args.rval().setBoolean(true);
  return true;
}

bool my_clickParty(JSContext* ctx, JS::CallArgs& args) {
  args.rval().setBoolean(false);

  if (args.length() < 2 || !args[0].isObject() || !args[1].isInt32())
    return true;

  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  D2UnitStrc* myUnit = D2CLIENT_GetPlayerUnit();
  PartyWrap* party_wrap;
  UNWRAP_OR_RETURN(ctx, &party_wrap, args[0]);
  D2RosterUnitStrc* pUnit = party_wrap->GetData();
  D2RosterUnitStrc* mypUnit = *D2CLIENT_PlayerUnitList;

  if (!pUnit || !mypUnit)
    return true;

  BOOL bFound = FALSE;

  for (D2RosterUnitStrc* pScan = mypUnit; pScan; pScan = pScan->pNext)
    if (pScan->dwUnitId == pUnit->dwUnitId)
      bFound = TRUE;

  if (!bFound)
    return true;

  int32_t nMode = args[1].toInt32();

  BnetData* pData = (*D2LAUNCH_BnData);

  // Attempt to loot player, check first if it's hardcore
  if (nMode == 0 && pData && !(pData->nCharFlags & PLAYER_TYPE_HARDCORE))
    return true;

  // Attempt to party a player who is already in party ^_~
  if (nMode == 2 && pUnit->wPartyId != 0xFFFF && mypUnit->wPartyId == pUnit->wPartyId && pUnit->wPartyId != 0xFFFF)
    return true;

  // don't leave a party if we are in none!
  if (nMode == 3 && pUnit->wPartyId == 0xFFFF)
    return true;
  else if (nMode == 3 && pUnit->wPartyId != 0xFFFF) {
    args.rval().setBoolean(true);
    D2CLIENT_LeaveParty();
    return true;
  }

  if (nMode < 0 || nMode > 5)
    return true;

  // Trying to click self
  if (pUnit->dwUnitId == myUnit->dwUnitId)
    return true;

  if (nMode == 0)
    D2CLIENT_HostilePartyUnit(pUnit, 2);
  else if (nMode == 1)
    D2CLIENT_HostilePartyUnit(pUnit, 1);
  else if (nMode == 4)
    D2CLIENT_HostilePartyUnit(pUnit, 3);
  else if (nMode == 5)
    D2CLIENT_HostilePartyUnit(pUnit, 4);
  else
    D2CLIENT_ClickParty(pUnit, nMode);

  args.rval().setBoolean(true);
  return true;
}

bool my_useStatPoint(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  uint32_t stat = 0;
  int32_t count = 1;
  JS::ToUint32(ctx, args[0], &stat);
  if (args.length() > 1) {
    count = args[1].toInt32();
  }

  UseStatPoint(static_cast<WORD>(stat), count);
  args.rval().setUndefined();
  return true;
}

bool my_useSkillPoint(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  uint32_t skill = 0;
  int32_t count = 1;
  JS::ToUint32(ctx, args[0], &skill);
  if (args.length() > 1) {
    count = args[1].toInt32();
  }

  UseSkillPoint(static_cast<WORD>(skill), count);
  args.rval().setUndefined();
  return true;
}

bool my_getBaseStat(JSContext* ctx, JS::CallArgs& args) {
  if (args.length() > 2) {
    StringWrap szStatName;
    StringWrap szTableName;
    int32_t nBaseStat = 0;
    int32_t nClassId = 0;
    int32_t nStat = -1;
    if (args[0].isString()) {
      szTableName = std::move(StringWrap{ctx, args[0]});
      if (!szTableName) {
        JS_ReportErrorASCII(ctx, "Invalid table value");
        return false;
      }
    } else if (args[0].isInt32())
      nBaseStat = args[0].toInt32();
    else {
      JS_ReportErrorASCII(ctx, "Invalid table value");
      return false;
    }

    if (args.hasDefined(1) && args[1].isInt32()) {
      nClassId = args[1].toInt32();
    }

    if (args[2].isString()) {
      szStatName = std::move(StringWrap{ctx, args[2]});
      if (!szStatName) {
        JS_ReportErrorASCII(ctx, "Invalid column value");
        return false;
      }
    } else if (args[2].isInt32())
      nStat = args[2].toInt32();
    else {
      JS_ReportErrorASCII(ctx, "Invalid column value");
      return false;
    }
    JS::RootedValue rval(ctx);
    FillBaseStat(ctx, &rval, nBaseStat, nClassId, nStat, szTableName, szStatName);
    args.rval().set(rval);
    return true;
  }

  args.rval().setUndefined();
  return true;
}

bool my_weaponSwitch(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  int32_t nParameter = NULL;
  if (args.length() > 0) {
    nParameter = args[0].toInt32();
  }

  if (nParameter == NULL) {
    // don't perform a weapon switch if current gametype is classic
    BnetData* pData = (*D2LAUNCH_BnData);
    if (pData) {
      if (!(pData->nCharFlags & PLAYER_TYPE_EXPAC))
        args.rval().setBoolean(false);
      return true;
    } else {
      JS_ReportErrorASCII(ctx, "Could not acquire BnData");
      return false;
    }

    BYTE aPacket[1];
    aPacket[0] = 0x60;
    D2NET_SendPacket(1, 1, aPacket);
    args.rval().setBoolean(true);
    return true;
  }
  args.rval().setInt32(*D2CLIENT_bWeapSwitch);
  return true;
}

bool my_transmute(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  bool cubeOn = !!D2CLIENT_GetUIState(UI_CUBE);
  if (!cubeOn)
    D2CLIENT_SetUIState(UI_CUBE, TRUE);

  D2CLIENT_Transmute();

  if (!cubeOn)
    D2CLIENT_SetUIState(UI_CUBE, FALSE);

  args.rval().setNull();
  return true;
}

bool my_getPlayerFlag(JSContext* ctx, JS::CallArgs& args) {
  if (args.length() != 3 || !args[0].isNumber() || !args[1].isNumber() || !args[2].isNumber()) {
    args.rval().setUndefined();
    return true;
  }

  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  uint32_t nFirstUnitId = (uint32_t)-1;
  uint32_t nSecondUnitId = (uint32_t)-1;
  uint32_t nFlag;
  JS::ToUint32(ctx, args[0], &nFirstUnitId);
  JS::ToUint32(ctx, args[1], &nSecondUnitId);
  JS::ToUint32(ctx, args[2], &nFlag);

  args.rval().setBoolean(D2CLIENT_TestPvpFlag(nFirstUnitId, nSecondUnitId, nFlag));
  return true;
}

bool my_getMouseCoords(JSContext* ctx, JS::CallArgs& args) {
  bool nFlag = false, nReturn = false;

  if (args.length() > 0 && args[0].isInt32() || args[0].isBoolean())
    nFlag = args[0].toBoolean();

  if (args.length() > 1 && args[1].isInt32() || args[1].isBoolean())
    nReturn = args[1].toBoolean();

  POINT Coords = {static_cast<LONG>(*D2CLIENT_MouseX), static_cast<LONG>(*D2CLIENT_MouseY)};

  if (nFlag) {
    Coords.x += *D2CLIENT_ViewportX;
    Coords.y += *D2CLIENT_ViewportY;

    D2COMMON_AbsScreenToMap(&Coords.x, &Coords.y);
  }

  JS::RootedValue jsX(ctx, JS::NumberValue(Coords.x));
  JS::RootedValue jsY(ctx, JS::NumberValue(Coords.y));
  if (nReturn) {
    JS::RootedObject pObj(ctx, JS_NewPlainObject(ctx));
    JS_SetProperty(ctx, pObj, "x", jsX);
    JS_SetProperty(ctx, pObj, "y", jsY);
    args.rval().setObject(*pObj);
    return true;
  } else {
    JS::RootedObject pObj(ctx, JS::NewArrayObject(ctx, 2));
    JS_SetElement(ctx, pObj, 0, jsX);
    JS_SetElement(ctx, pObj, 1, jsY);
    args.rval().setObject(*pObj);
    return true;
  }

  args.rval().setUndefined();
  return true;
}

bool my_submitItem(JSContext* ctx, JS::CallArgs& args) {
  args.rval().setNull();

  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  if (D2UnitStrc* pUnit = D2CLIENT_GetCursorItem()) {
    if (D2CLIENT_GetPlayerUnit()->dwAct == 1) {
      if (GetPlayerArea() == D2CLIENT_GetPlayerUnit()->pAct->pMisc->dwStaffTombLevel) {
        *D2CLIENT_CursorItemMode = 3;
        BYTE aPacket[17] = {NULL};
        aPacket[0] = 0x44;
        *(DWORD*)&aPacket[1] = D2CLIENT_GetPlayerUnit()->dwUnitId;
        *(DWORD*)&aPacket[5] = *D2CLIENT_OrificeId;
        *(DWORD*)&aPacket[9] = pUnit->dwUnitId;
        *(DWORD*)&aPacket[13] = 3;
        D2NET_SendPacket(17, 1, aPacket);
        args.rval().setBoolean(true);
      } else
        args.rval().setBoolean(false);
    } else if (D2CLIENT_GetPlayerUnit()->dwAct == 0 || D2CLIENT_GetPlayerUnit()->dwAct == 4)  // dwAct is 0-4, not 1-5
    {
      if (*D2CLIENT_RecentInteractId && D2COMMON_IsTownByLevelNo(GetPlayerArea())) {
        D2CLIENT_SubmitItem(pUnit->dwUnitId);
        args.rval().setBoolean(true);
      } else  // what
        args.rval().setBoolean(false);
    } else  // the
      args.rval().setBoolean(false);
  } else  // fuck
    args.rval().setBoolean(false);

  return true;
}

bool my_getIsTalkingNPC(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  args.rval().setBoolean(IsScrollingText());
  return true;
}

bool my_getInteractedNPC(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  D2UnitStrc* pNPC = D2CLIENT_GetCurrentInteractingNPC();
  if (!pNPC) {
    args.rval().setBoolean(false);
    return true;
  }

  UnitWrap::UnitData* pmyUnit = new UnitWrap::UnitData;  // leaked?
  if (!pmyUnit) {
    args.rval().setUndefined();
    return true;
  }

  char szName[256] = "";
  pmyUnit->dwPrivateType = PRIVATE_UNIT;
  pmyUnit->dwClassId = pNPC->dwTxtFileNo;
  pmyUnit->dwMode = pNPC->dwMode;
  pmyUnit->dwType = pNPC->dwType;
  pmyUnit->dwUnitId = pNPC->dwUnitId;
  strcpy_s(pmyUnit->szName, sizeof(pmyUnit->szName), szName);
  JS::RootedObject newObject(ctx, UnitWrap::Instantiate(ctx, pmyUnit));
  if (!newObject) {
    THROW_ERROR(ctx, "failed to instantiate unit");
  }
  args.rval().setObject(*newObject);
  return true;
}

bool my_takeScreenshot(JSContext* ctx, JS::CallArgs& args) {
  Vars.bTakeScreenshot = true;
  args.rval().setUndefined();
  return true;
}

bool my_moveNPC(JSContext* ctx, JS::CallArgs& args) {
  if (!args.requireAtLeast(ctx, "moveNPC", 2)) {
    return false;
  }

  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  if (!Vars.bEnableUnsupported) {
    JS::WarnUTF8(ctx, "moveNPC requires EnableUnsupported = true in d2bs.ini");
    args.rval().setBoolean(false);
    return true;
  }

  UnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args[0]);
  UnitWrap::UnitData* pNpc = wrap->GetData();
  if (!pNpc || pNpc->dwType != 1) {
    JS_ReportErrorASCII(ctx, "Invalid NPC passed to moveNPC!");
    return false;
  }

  uint32_t dwX;
  uint32_t dwY;
  JS::ToUint32(ctx, args[1], &dwX);
  JS::ToUint32(ctx, args[2], &dwY);

  if (!WaitForGameReady()) {
    JS::WarnUTF8(ctx, "Game not ready");
    args.rval().setBoolean(false);
    return true;
  }

  BYTE aPacket[17];
  aPacket[0] = 0x59;
  *(DWORD*)&aPacket[1] = pNpc->dwType;
  *(DWORD*)&aPacket[5] = pNpc->dwUnitId;
  *(DWORD*)&aPacket[9] = dwX;
  *(DWORD*)&aPacket[13] = dwY;

  D2NET_SendPacket(sizeof(aPacket), 1, aPacket);
  args.rval().setBoolean(true);
  return true;
}

bool my_revealLevel(JSContext* ctx, JS::CallArgs& args) {
  D2UnitStrc* unit = D2CLIENT_GetPlayerUnit();

  if (!unit) {
    args.rval().setUndefined();
    return true;
  }

  D2DrlgLevelStrc* level = unit->pPath->pRoom1->pRoom2->pLevel;

  if (!level) {
    args.rval().setUndefined();
    return true;
  }

  BOOL bDrawPresets = false;

  if (args.length() == 1 && args[0].isBoolean()) {
    bDrawPresets = args[0].toBoolean();
  }
  AutoCriticalRoom cRoom;
  if (!GameReady()) {
    args.rval().setUndefined();
    return true;
  }

  for (D2DrlgRoomStrc* room = level->pRoom2First; room; room = room->pRoom2Next) {
    room->Reveal(bDrawPresets);
  }
  args.rval().setUndefined();
  return true;
}

bool screenToAutomap(JSContext* ctx, JS::CallArgs& args) {
  if (!args.requireAtLeast(ctx, "screenToAutomap", 1)) {
    return false;
  }

  if (args.length() == 1) {
    // the arg must be an object with an x and a y that we can convert
    if (args[0].isObject()) {
      JS::RootedObject obj(ctx, args[0].toObjectOrNull());
      JS::RootedValue x(ctx);
      JS::RootedValue y(ctx);
      if (!JS_GetProperty(ctx, obj, "x", &x)) {
        args.rval().setUndefined();
        return true;
      }
      if (!JS_GetProperty(ctx, obj, "x", &y)) {
        args.rval().setUndefined();
        return true;
      }
      if (!x.isInt32() || !y.isInt32()) {
        JS_ReportErrorASCII(ctx, "Input has an x or y, but they aren't the correct type!");
        return false;
      }

      int32_t ix = x.toInt32();
      int32_t iy = y.toInt32();

      // convert the values
      POINT result = ScreenToAutomap(ix, iy);
      JS::RootedObject rval(ctx, JS_NewPlainObject(ctx));
      JS::RootedValue resultX(ctx, JS::Int32Value(result.x));
      JS::RootedValue resultY(ctx, JS::Int32Value(result.y));
      JS_SetProperty(ctx, rval, "x", resultX);
      JS_SetProperty(ctx, rval, "y", resultY);
      args.rval().setObject(*rval);
      return true;
    } else {
      JS_ReportErrorASCII(ctx, "Invalid object specified to screenToAutomap");
      return false;
    }
  } else if (args.length() == 2) {
    // the args must be ints
    if (args[0].isInt32() && args[1].isInt32()) {
      int32_t ix = args[0].toInt32();
      int32_t iy = args[1].toInt32();

      // convert the values
      POINT result = ScreenToAutomap(ix, iy);
      JS::RootedObject rval(ctx, JS_NewPlainObject(ctx));
      JS::RootedValue resultX(ctx, JS::Int32Value(result.x));
      JS::RootedValue resultY(ctx, JS::Int32Value(result.y));
      JS_SetProperty(ctx, rval, "x", resultX);
      JS_SetProperty(ctx, rval, "y", resultY);
      args.rval().setObject(*rval);
      return true;
    } else {
      JS_ReportErrorASCII(ctx, "screenToAutomap expects two arguments to be two integers");
      return false;
    }
  }
  JS_ReportErrorASCII(ctx, "Invalid arguments specified for screenToAutomap");
  return false;
}

// POINT result = {ix, iy};
// AutomapToScreen(&result);
bool automapToScreen(JSContext* ctx, JS::CallArgs& args) {
  if (!args.requireAtLeast(ctx, "automapToScreen", 1)) {
    return false;
  }

  if (args.length() == 1) {
    // the arg must be an object with an x and a y that we can convert
    if (args[0].isObject()) {
      JS::RootedObject obj(ctx, args[0].toObjectOrNull());
      JS::RootedValue x(ctx);
      JS::RootedValue y(ctx);
      if (!JS_GetProperty(ctx, obj, "x", &x)) {
        args.rval().setUndefined();
        return true;
      }
      if (!JS_GetProperty(ctx, obj, "x", &y)) {
        args.rval().setUndefined();
        return true;
      }
      if (!x.isInt32() || !y.isInt32()) {
        JS_ReportErrorASCII(ctx, "Input has an x or y, but they aren't the correct type!");
        return false;
      }

      int32_t ix = x.toInt32();
      int32_t iy = y.toInt32();

      // convert the values
      POINT result = {ix, iy};
      AutomapToScreen(&result);
      JS::RootedObject rval(ctx, JS_NewPlainObject(ctx));
      JS::RootedValue resultX(ctx, JS::Int32Value(result.x));
      JS::RootedValue resultY(ctx, JS::Int32Value(result.y));
      JS_SetProperty(ctx, rval, "x", resultX);
      JS_SetProperty(ctx, rval, "y", resultY);
      args.rval().setObject(*rval);
      return true;
    } else {
      JS_ReportErrorASCII(ctx, "Invalid object specified to screenToAutomap");
      return false;
    }
  } else if (args.length() == 2) {
    // the args must be ints
    // the args must be ints
    if (args[0].isInt32() && args[1].isInt32()) {
      int32_t ix = args[0].toInt32();
      int32_t iy = args[1].toInt32();

      // convert the values
      POINT result = {ix, iy};
      AutomapToScreen(&result);
      JS::RootedObject rval(ctx, JS_NewPlainObject(ctx));
      JS::RootedValue resultX(ctx, JS::Int32Value(result.x));
      JS::RootedValue resultY(ctx, JS::Int32Value(result.y));
      JS_SetProperty(ctx, rval, "x", resultX);
      JS_SetProperty(ctx, rval, "y", resultY);
      args.rval().setObject(*rval);
      return true;
    } else {
      JS_ReportErrorASCII(ctx, "automapToScreen expects two arguments to be two integers");
      return false;
    }
  }
  JS_ReportErrorASCII(ctx, "Invalid arguments specified for automapToScreen");
  return false;
}
