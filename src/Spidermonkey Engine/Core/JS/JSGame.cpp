#include "JSGame.h"
#include "D2Helpers.h"
#include "CriticalSections.h"
#include "D2Skills.h"
#include "MPQStats.h"
#include "Core.h"
#include "Helpers.h"
#include "Game.h"
#include "JSArea.h"
#include "JSGlobalClasses.h"
#include "MapHeader.h"
#include "JSRoom.h"

#include "Game/D2Roster.h"

#include <cassert>
#include <cmath>

JSAPI_FUNC(my_copyUnit) {
  if (argc >= 1 && JS_IsObject(argv[0]) && !JS_IsNull(argv[0])) {
    Private* myPrivate = (Private*)JS_GetOpaque3(argv[0]);

    if (!myPrivate)
      return JS_UNDEFINED;

    if (myPrivate->dwPrivateType == PRIVATE_UNIT) {
      myUnit* lpOldUnit = (myUnit*)JS_GetOpaque3(argv[0]);
      myUnit* lpUnit = new myUnit;

      if (lpUnit) {
        memcpy(lpUnit, lpOldUnit, sizeof(myUnit));
        JSValue jsunit = BuildObject(ctx, unit_class_id, lpUnit);
        if (JS_IsException(jsunit)) {
          delete lpUnit;
          lpUnit = NULL;
          THROW_ERROR(ctx, "Couldn't copy unit");
        }
        return jsunit;
      }
    } else if (myPrivate->dwPrivateType == PRIVATE_ITEM) {
      invUnit* lpOldUnit = (invUnit*)JS_GetOpaque3(argv[0]);
      invUnit* lpUnit = new invUnit;

      if (lpUnit) {
        memcpy(lpUnit, lpOldUnit, sizeof(invUnit));
        JSValue jsunit = BuildObject(ctx, unit_class_id, lpUnit);
        if (JS_IsException(jsunit)) {
          delete lpUnit;
          lpUnit = NULL;
          THROW_ERROR(ctx, "Couldn't copy unit");
        }
        return jsunit;
      }
    }
  }

  return JS_UNDEFINED;
}

JSAPI_FUNC(my_clickMap) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  uint32_t nClickType = NULL, nShift = NULL, nX = NULL, nY = NULL;

  if (argc < 3)
    return JS_FALSE;
  if (JS_IsNumber(argv[0]))
    JS_ToUint32(ctx, &nClickType, argv[0]);
  if (JS_IsNumber(argv[1]) || JS_IsBool(argv[1]))
    JS_ToUint32(ctx, &nShift, argv[1]);
  if (JS_IsNumber(argv[2]))
    JS_ToUint32(ctx, &nX, argv[2]);
  if (JS_IsNumber(argv[3]))
    JS_ToUint32(ctx, &nY, argv[3]);

  if (argc == 3 && JS_IsNumber(argv[0]) && (JS_IsNumber(argv[1]) || JS_IsBool(argv[1])) && JS_IsObject(argv[2]) && !JS_IsNull(argv[2])) {
    myUnit* mypUnit = (myUnit*)JS_GetOpaque3(argv[2]);

    if (!mypUnit || (mypUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
      return JS_FALSE;

    UnitAny* pUnit = D2CLIENT_FindUnit(mypUnit->dwUnitId, mypUnit->dwType);

    if (!pUnit)
      return JS_FALSE;

    Vars.dwSelectedUnitId = NULL;
    Vars.dwSelectedUnitType = NULL;
    return JS_NewBool(ctx, ClickMap(nClickType, nX, nY, nShift, pUnit));
  } else if (argc > 3 && JS_IsNumber(argv[0]) && (JS_IsNumber(argv[1]) || JS_IsBool(argv[1])) && JS_IsNumber(argv[2]) && JS_IsNumber(argv[3])) {
    return JS_NewBool(ctx, ClickMap(nClickType, nX, nY, nShift, NULL));
  }

  return JS_FALSE;
}

JSAPI_FUNC(my_acceptTrade) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  // TODO: Fix this nonsense.

  if (argc > 0) {
    int32_t test = 0;
    JS_ToInt32(ctx, &test, argv[0]);
    if (test == 1) {  // Called with a '1' it will return if we already accepted it or not
      return JS_NewBool(ctx, *p_D2CLIENT_bTradeAccepted);
    } else if (test == 2) {  // Called with a '2' it will return the trade flag
      return JS_NewInt32(ctx, *p_D2CLIENT_RecentTradeId);
    } else if (test == 3) {  // Called with a '3' it will return if the 'check' is red or not
      return JS_NewBool(ctx, *p_D2CLIENT_bTradeBlock);
    }
  }

  AutoCriticalRoom cRoom;

  if ((*p_D2CLIENT_RecentTradeId) == 3 || (*p_D2CLIENT_RecentTradeId) == 5 || (*p_D2CLIENT_RecentTradeId) == 7) {
    JSValue rval = JS_FALSE;
    if ((*p_D2CLIENT_bTradeBlock)) {
      // Don't operate if we can't trade anyway ...
      rval = JS_FALSE;
    } else if ((*p_D2CLIENT_bTradeAccepted)) {
      (*p_D2CLIENT_bTradeAccepted) = FALSE;
      D2CLIENT_CancelTrade();
      rval = JS_TRUE;
    } else {
      (*p_D2CLIENT_bTradeAccepted) = TRUE;
      D2CLIENT_AcceptTrade();
      rval = JS_TRUE;
    }
    return rval;
  }
  return JS_ThrowInternalError(ctx, "Invalid parameter passed to acceptTrade!");
}

JSAPI_FUNC(my_tradeOk) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  AutoCriticalRoom cRoom;
  TransactionDialogsInfo_t* pTdi = *p_D2CLIENT_pTransactionDialogsInfo;
  unsigned int i;

  if (pTdi != NULL) {
    for (i = 0; i < pTdi->numLines; ++i) {
      // Not sure if *p_D2CLIENT_TransactionDialogs == 1 necessary if it's in
      // the dialog list, but if it's not 1, a crash is guaranteed. (CrazyCasta)
      if (pTdi->dialogLines[i].handler == D2CLIENT_TradeOK && *p_D2CLIENT_TransactionDialogs == 1) {
        D2CLIENT_TradeOK();
        return JS_TRUE;
      }
    }
  }
  return JS_ThrowInternalError(ctx, "Not in proper state to click ok to trade.");
}

JSAPI_FUNC(my_getDialogLines) {
  JSValue rval = JS_UNDEFINED;
  TransactionDialogsInfo_t* pTdi = *p_D2CLIENT_pTransactionDialogsInfo;
  unsigned int i;
  JSValue pReturnArray;
  JSValue line;

  AutoCriticalRoom cRoom;

  if (pTdi != NULL) {
    pReturnArray = JS_NewArray(ctx);

    for (i = 0; i < pTdi->numLines; ++i) {
      line = BuildObject(ctx, dialogLine_class_id, &pTdi->dialogLines[i]);
      JS_SetPropertyStr(ctx, line, "text", JS_NewString(ctx, pTdi->dialogLines[i].text));
      JS_SetPropertyStr(ctx, line, "selectable", JS_NewBool(ctx, pTdi->dialogLines[i].bMaybeSelectable));

      JS_SetPropertyStr(ctx, line, "handler", JS_NewCFunction(ctx, my_clickDialog, "handler", 0));
      JS_SetPropertyUint32(ctx, pReturnArray, i, line);
    }
    rval = pReturnArray;
  }
  return rval;
}

JSAPI_FUNC(my_clickDialog) {
  TransactionDialogsLine_t* tdl;

  tdl = (TransactionDialogsLine_t*)JS_GetOpaque2(ctx, this_val, dialogLine_class_id);

  if (tdl != NULL && tdl->bMaybeSelectable)
    tdl->handler();
  else
    THROW_ERROR(ctx, "That dialog is not currently clickable.");

  return JS_TRUE;
}

JSAPI_FUNC(my_getPath) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  if (argc < 5)
    THROW_ERROR(ctx, "Not enough parameters were passed to getPath!");

  uint32_t lvl = 0, x = 0, y = 0, dx = 0, dy = 0, reductionType = 0, radius = 20;
  uint32_t* args[] = {&lvl, &x, &y, &dx, &dy, &reductionType, &radius};
  for (int i = 0; i < argc && i < _countof(args); ++i) {
    if (JS_IsNumber(argv[i]) && JS_ToUint32(ctx, args[i], argv[i])) {
      return JS_EXCEPTION;
    }
  }

  if (reductionType == 3 && !(JS_IsFunction(ctx, argv[7]) && JS_IsFunction(ctx, argv[8]) && JS_IsFunction(ctx, argv[9]))) {
    THROW_ERROR(ctx, "Invalid function values for reduction type");
  }
  if (lvl == 0) {
    THROW_ERROR(ctx, "Invalid level passed to getPath");
  }
  Level* level = GetLevel(lvl);

  if (!level)
    return JS_EXCEPTION;

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
      reducer = new JSPathReducer(map, ctx, this_val, argv[7], argv[8], argv[9]);
      break;
    default:
      THROW_ERROR(ctx, "Invalid path reducer value!");
      break;
  }

  PointList list;
  AStarPath<> path(map, reducer);

  // box18jsrefcount depth = JS_SuspendRequest(cx);

  path.GetPath(start, end, list, true);
  map->CleanUp();
  // box18JS_ResumeRequest(cx, depth);

  int count = list.size();

  JSValue arr = JS_NewArray(ctx);
  for (int i = 0; i < count; i++) {
    JSValue jx = JS_NewInt32(ctx, list[i].first), jy = JS_NewInt32(ctx, list[i].second);

    JSValue point = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, point, "x", jx);
    JS_SetPropertyStr(ctx, point, "y", jy);

    JS_SetPropertyUint32(ctx, arr, i, point);
  }

  delete reducer;
  map->CleanUp();
  return arr;
}

JSAPI_FUNC(my_getCollision) {
  if (!WaitForGameReady()) {
    THROW_WARNING(ctx, "Game not ready");
  }

  uint32_t nLevelId, nX, nY;
  uint32_t* args[] = {&nLevelId, &nX, &nY};
  for (int i = 0; i < argc && i < _countof(args); ++i) {
    if (JS_IsNumber(argv[i]) && JS_ToUint32(ctx, args[i], argv[i])) {
      return JS_EXCEPTION;
    }
  }

  Point point(nX, nY);
  Level* level = GetLevel(nLevelId);
  if (!level) {
    THROW_ERROR(ctx, "Level Not loaded");
  }

  ActMap* map = ActMap::GetMap(level);
  // if(!map->IsValidPoint(point))  //return avoid instead and make it not lvl depenant
  //	{ map->CleanUp(); THROW_ERROR(cx, "Invalid point!");}

  JSValue rval;
  rval = JS_NewInt32(ctx, map->GetMapData(point, true));
  map->CleanUp();
  return rval;
}

JSAPI_FUNC(my_clickItem) {
  JSValue rval = JS_NULL;
  typedef void __fastcall clickequip(UnitAny * pPlayer, Inventory * pIventory, int loc);

  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  AutoCriticalRoom cRoom;

  if (*p_D2CLIENT_TransactionDialog != 0 || *p_D2CLIENT_TransactionDialogs != 0 || *p_D2CLIENT_TransactionDialogs_2 != 0) {
    rval = JS_FALSE;
    return rval;
  }

  myUnit* pmyUnit = NULL;
  UnitAny* pUnit = NULL;

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

  *p_D2CLIENT_CursorHoverX = 0xFFFFFFFF;
  *p_D2CLIENT_CursorHoverY = 0xFFFFFFFF;

  if (argc == 1 && JS_IsObject(argv[0])) {
    pmyUnit = (myUnit*)JS_GetOpaque3(argv[0]);

    if (!pmyUnit || (pmyUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
      return rval;
    }

    pUnit = D2CLIENT_FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

    if (!pUnit) {
      return rval;
    }

    clickequip* click = (clickequip*)*(DWORD*)(D2CLIENT_BodyClickTable + (4 * pUnit->pItemData->BodyLocation));

    if (!click) {
      return rval;
    }

    click(D2CLIENT_GetPlayerUnit(), D2CLIENT_GetPlayerUnit()->pInventory, pUnit->pItemData->BodyLocation);
    return rval;
  } else if (argc == 2 && JS_IsNumber(argv[0]) && JS_IsNumber(argv[1])) {
    int32_t nClickType;
    int32_t nBodyLoc;
    JS_ToInt32(ctx, &nClickType, argv[0]);
    JS_ToInt32(ctx, &nBodyLoc, argv[1]);

    if (nClickType == NULL) {
      clickequip* click = (clickequip*)*(DWORD*)(D2CLIENT_BodyClickTable + (4 * nBodyLoc));

      if (!click) {
        return rval;
      }

      click(D2CLIENT_GetPlayerUnit(), D2CLIENT_GetPlayerUnit()->pInventory, nBodyLoc);

    }
    // Click Merc Gear
    else if (nClickType == 4) {
      if (nBodyLoc == 1 || nBodyLoc == 3 || nBodyLoc == 4) {
        UnitAny* pMerc = GetMercUnit(D2CLIENT_GetPlayerUnit());

        if (pMerc) {
          D2CLIENT_MercItemAction(0x61, nBodyLoc);
          rval = JS_TRUE;
        }
      }
    }
    return rval;
  } else if (argc == 2 && JS_IsNumber(argv[0]) && JS_IsObject(argv[1])) {
    pmyUnit = (myUnit*)JS_GetOpaque3(argv[1]);

    if (!pmyUnit || (pmyUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT) {
      return rval;
    }

    pUnit = D2CLIENT_FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

    int32_t nClickType;
    JS_ToInt32(ctx, &nClickType, argv[0]);

    if (!pUnit || !(pUnit->dwType == UNIT_ITEM) || !pUnit->pItemData) {
      THROW_ERROR(ctx, "Object is not an item!");
    }

    int InventoryLocation = GetItemLocation(pUnit);
    int ClickLocation = LOCATION_NULL;

    int x = pUnit->pItemPath->dwPosX;
    int y = pUnit->pItemPath->dwPosY;

    *p_D2CLIENT_CursorHoverX = x;
    *p_D2CLIENT_CursorHoverY = y;

    InventoryLayout* pLayout = NULL;

    if (nClickType == 4) {
      UnitAny* pMerc = GetMercUnit(D2CLIENT_GetPlayerUnit());

      if (pMerc)
        if (pUnit->pItemData && pUnit->pItemData->pOwner)
          if (pUnit->pItemData->pOwner->dwUnitId == pMerc->dwUnitId) {
            rval = JS_TRUE;
            D2CLIENT_MercItemAction(0x61, pUnit->pItemData->BodyLocation);
          }
      return rval;
    } else if (InventoryLocation == LOCATION_INVENTORY || InventoryLocation == LOCATION_STASH || InventoryLocation == LOCATION_CUBE) {
      switch (InventoryLocation) {
        case LOCATION_INVENTORY:
          pLayout = (InventoryLayout*)p_D2CLIENT_InventoryLayout;
          ClickLocation = CLICKTARGET_INVENTORY;
          break;
        case LOCATION_STASH:
          pLayout = (InventoryLayout*)p_D2CLIENT_StashLayout;
          ClickLocation = CLICKTARGET_STASH;
          break;
        case LOCATION_CUBE:
          pLayout = (InventoryLayout*)p_D2CLIENT_CubeLayout;
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
        return rval;
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
        return rval;
      }

      clickequip* click = (clickequip*)*(DWORD*)(D2CLIENT_BodyClickTable + (4 * nClickType));

      if (!click) {
        return rval;
      }

      click(D2CLIENT_GetPlayerUnit(), D2CLIENT_GetPlayerUnit()->pInventory, nClickType);
    }
  } else if (argc == 4) {
    if (JS_IsNumber(argv[0]) && JS_IsNumber(argv[1]) && JS_IsNumber(argv[2]) && JS_IsNumber(argv[3])) {
      int32_t nButton;
      int32_t nX;
      int32_t nY;
      int32_t nLoc;
      JS_ToInt32(ctx, &nButton, argv[0]);
      JS_ToInt32(ctx, &nX, argv[1]);
      JS_ToInt32(ctx, &nY, argv[2]);
      JS_ToInt32(ctx, &nLoc, argv[3]);

      int clickTarget = LOCATION_NULL;
      InventoryLayout* pLayout = NULL;

      *p_D2CLIENT_CursorHoverX = nX;
      *p_D2CLIENT_CursorHoverY = nY;

      // Fixing the x/y click spot for items taking more than one inventory square- so Diablo can handle it!
      if (nLoc != LOCATION_BELT) {
        UnitAny* pItem = D2CLIENT_GetCursorItem();
        if (pItem) {
          ItemTxt* pTxt = D2COMMON_GetItemText(pItem->dwTxtFileNo);
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
            pLayout = (InventoryLayout*)p_D2CLIENT_InventoryLayout;
            clickTarget = CLICKTARGET_INVENTORY;
            break;
          case LOCATION_TRADE:
            pLayout = (InventoryLayout*)p_D2CLIENT_TradeLayout;
            clickTarget = CLICKTARGET_TRADE;
            break;
          case LOCATION_CUBE:
            pLayout = (InventoryLayout*)p_D2CLIENT_CubeLayout;
            clickTarget = CLICKTARGET_CUBE;
            break;
          case LOCATION_STASH:
            pLayout = (InventoryLayout*)p_D2CLIENT_StashLayout;
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

        return JS_TRUE;
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
          return rval;
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

        return JS_TRUE;
      }
    }
  }
  return JS_TRUE;
}

JSAPI_FUNC(my_getLocaleString) {
  if (argc < 1 || !JS_IsNumber(argv[0]))
    return JS_UNDEFINED;

  uint32_t localeId;
  JS_ToUint32(ctx, &localeId, argv[0]);
  wchar_t* wString = D2LANG_GetLocaleText(static_cast<uint16_t>(localeId));
  return JS_NewString(ctx, wString);
}

JSAPI_FUNC(my_rand) {
  if (argc < 2 || !JS_IsNumber(argv[0]) || !JS_IsNumber(argv[1])) {
    return JS_NewInt32(ctx, 0);
  }

  // only seed the rng once
  static bool seeded = false;
  if (!seeded) {
    srand(GetTickCount());
    seeded = true;
  }

  long long seed = 0;
  if (ClientState() == ClientStateInGame)
    seed = D2GAME_Rand(D2CLIENT_GetPlayerUnit()->dwSeed);
  else
    seed = rand();

  int32_t high;
  int32_t low;
  JS_ToInt32(ctx, &low, argv[0]);
  JS_ToInt32(ctx, &high, argv[1]);

  if (high > low + 1) {
    int i = (seed % (high - low + 1)) + low;
    return JS_NewInt32(ctx, i);
  }

  return JS_NewInt32(ctx, high);
}

JSAPI_FUNC(my_getDistance) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  // TODO: Add the type of distance to the api design
  int32_t nX1 = NULL;
  int32_t nX2 = NULL;
  int32_t nY1 = NULL;
  int32_t nY2 = NULL;

  if (argc == 1 && JS_IsObject(argv[0])) {
    JSValue x1 = JS_GetPropertyStr(ctx, argv[0], "x");
    JSValue y1 = JS_GetPropertyStr(ctx, argv[0], "y");
    if (JS_IsNumber(x1) && JS_IsNumber(y1)) {
      nX1 = D2CLIENT_GetUnitX(D2CLIENT_GetPlayerUnit());
      nY1 = D2CLIENT_GetUnitY(D2CLIENT_GetPlayerUnit());
      JS_ToInt32(ctx, &nX2, x1);
      JS_ToInt32(ctx, &nY2, y1);
    }
  } else if (argc == 2) {
    if (JS_IsNumber(argv[0]) && JS_IsNumber(argv[1])) {
      nX1 = D2CLIENT_GetUnitX(D2CLIENT_GetPlayerUnit());
      nY1 = D2CLIENT_GetUnitY(D2CLIENT_GetPlayerUnit());
      JS_ToInt32(ctx, &nX2, argv[0]);
      JS_ToInt32(ctx, &nY2, argv[1]);
    } else if (JS_IsObject(argv[0]) && JS_IsObject(argv[1])) {
      JSValue x = JS_GetPropertyStr(ctx, argv[0], "x");
      JSValue y = JS_GetPropertyStr(ctx, argv[0], "y");
      JSValue x2 = JS_GetPropertyStr(ctx, argv[1], "x");
      JSValue y2 = JS_GetPropertyStr(ctx, argv[1], "y");
      if (JS_IsNumber(x) && JS_IsNumber(y) && JS_IsNumber(x2) && JS_IsNumber(y2)) {
        JS_ToInt32(ctx, &nX1, x);
        JS_ToInt32(ctx, &nY1, y);
        JS_ToInt32(ctx, &nX2, x2);
        JS_ToInt32(ctx, &nY2, y2);
      }
    }
  } else if (argc == 3) {
    if (JS_IsObject(argv[0]) && JS_IsNumber(argv[1]) && JS_IsNumber(argv[2])) {
      myUnit* pUnit1 = (myUnit*)JS_GetOpaque3(argv[0]);

      if (!pUnit1 || (pUnit1->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
        return JS_UNDEFINED;

      UnitAny* pUnitA = D2CLIENT_FindUnit(pUnit1->dwUnitId, pUnit1->dwType);

      if (!pUnitA)
        return JS_UNDEFINED;

      nX1 = D2CLIENT_GetUnitX(pUnitA);
      nY1 = D2CLIENT_GetUnitY(pUnitA);
      JS_ToInt32(ctx, &nX2, argv[1]);
      JS_ToInt32(ctx, &nY2, argv[2]);
    } else if (JS_IsNumber(argv[0]) && JS_IsNumber(argv[1]) && JS_IsObject(argv[2])) {
      myUnit* pUnit1 = (myUnit*)JS_GetOpaque3(argv[2]);

      if (!pUnit1 || (pUnit1->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
        return JS_UNDEFINED;

      UnitAny* pUnitA = D2CLIENT_FindUnit(pUnit1->dwUnitId, pUnit1->dwType);

      if (!pUnitA)
        return JS_UNDEFINED;

      nX1 = D2CLIENT_GetUnitX(pUnitA);
      nY1 = D2CLIENT_GetUnitY(pUnitA);
      JS_ToInt32(ctx, &nX2, argv[0]);
      JS_ToInt32(ctx, &nY2, argv[1]);
    }
  } else if (argc == 4) {
    if (JS_IsNumber(argv[0]) && JS_IsNumber(argv[1]) && JS_IsNumber(argv[2]) && JS_IsNumber(argv[3])) {
      JS_ToInt32(ctx, &nX1, argv[0]);
      JS_ToInt32(ctx, &nY1, argv[1]);
      JS_ToInt32(ctx, &nX2, argv[2]);
      JS_ToInt32(ctx, &nY2, argv[3]);
    }
  }
  return JS_NewFloat64(ctx, abs(GetDistance(nX1, nY1, nX2, nY2)));
}

JSAPI_FUNC(my_gold) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  int32_t nGold = NULL;
  int32_t nMode = 1;

  if (argc > 0 && JS_IsNumber(argv[0]))
    JS_ToInt32(ctx, &nGold, argv[0]);

  if (argc > 1 && JS_IsNumber(argv[1]))
    JS_ToInt32(ctx, &nMode, argv[1]);

  SendGold(nGold, nMode);
  return JS_UNDEFINED;
}

JSAPI_FUNC(my_checkCollision) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  if (argc == 3 && JS_IsObject(argv[0]) && JS_IsObject(argv[1]) && JS_IsNumber(argv[2])) {
    myUnit* pUnitA = (myUnit*)JS_GetOpaque3(argv[0]);
    myUnit* pUnitB = (myUnit*)JS_GetOpaque3(argv[1]);
    int32_t nBitMask;
    JS_ToInt32(ctx, &nBitMask, argv[2]);

    if (!pUnitA || (pUnitA->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT || !pUnitB || (pUnitB->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
      return JS_UNDEFINED;

    UnitAny* pUnit1 = D2CLIENT_FindUnit(pUnitA->dwUnitId, pUnitA->dwType);
    UnitAny* pUnit2 = D2CLIENT_FindUnit(pUnitB->dwUnitId, pUnitB->dwType);

    if (!pUnit1 || !pUnit2)
      return JS_UNDEFINED;

    return JS_NewInt32(ctx, D2COMMON_CheckUnitCollision(pUnit1, pUnit2, (WORD)nBitMask));
  }

  return JS_UNDEFINED;
}

JSAPI_FUNC(my_getCursorType) {
  int32_t nType = NULL;

  if (argc > 0)
    JS_ToInt32(ctx, &nType, argv[0]);

  return JS_NewInt32(ctx, nType == 1 ? *p_D2CLIENT_ShopCursorType : *p_D2CLIENT_RegularCursorType);
}

JSAPI_FUNC(my_getSkillByName) {
  if (argc < 1 || !JS_IsString(argv[0]))
    return JS_UNDEFINED;

  const char* lpszText = JS_ToCString(ctx, argv[0]);
  if (!lpszText || lpszText[0])
    THROW_ERROR(ctx, "Could not convert string");

  for (int i = 0; i < _countof(Game_Skills); i++) {
    if (!_strcmpi(Game_Skills[i].name, lpszText)) {
      JS_FreeCString(ctx, lpszText);
      return JS_NewInt32(ctx, Game_Skills[i].skillID);
    }
  }

  JS_FreeCString(ctx, lpszText);
  return JS_UNDEFINED;
}

JSAPI_FUNC(my_getSkillById) {
  if (argc < 1 || !JS_IsNumber(argv[0]))
    return JS_UNDEFINED;

  int32_t nId;
  JS_ToInt32(ctx, &nId, argv[0]);

  int row = 0;
  if (FillBaseStat("skills", nId, "skilldesc", &row, sizeof(int))) {
    if (FillBaseStat("skilldesc", row, "str name", &row, sizeof(int))) {
      wchar_t* szName = D2LANG_GetLocaleText((WORD)row);
      return JS_NewString(ctx, szName);
    }
  }

  return JS_NewString(ctx, "Unknown");
}

JSAPI_FUNC(my_getTextSize) {
  if (argc < 2 || !JS_IsString(argv[0]) || !JS_IsNumber(argv[1])) {
    return JS_FALSE;
  }

  const char* szString = JS_ToCString(ctx, argv[0]);
  if (!szString) {
    return JS_EXCEPTION;
  }

  int font;
  JS_ToInt32(ctx, &font, argv[1]);
  POINT r = CalculateTextLen(szString, font);
  JS_FreeCString(ctx, szString);
  JSValue x = JS_NewInt64(ctx, r.x);
  JSValue y = JS_NewInt64(ctx, r.y);

  JSValue pObj = NULL;
  if (argc > 2 && JS_IsBool(argv[2]) && JS_ToBool(ctx, argv[2]) == TRUE) {
    // return an object with a height/width rather than an array
    pObj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, pObj, "width", x);
    JS_SetPropertyStr(ctx, pObj, "height", y);
  } else {
    pObj = JS_NewArray(ctx);
    JS_SetPropertyUint32(ctx, pObj, 0, x);
    JS_SetPropertyUint32(ctx, pObj, 1, y);
  }
  return pObj;
}

JSAPI_FUNC(my_getTradeInfo) {
  if (argc < 1)
    return JS_FALSE;

  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  if (!JS_IsNumber(argv[0]))
    return JS_FALSE;

  int32_t nMode;
  JS_ToInt32(ctx, &nMode, argv[0]);
  switch (nMode) {
    case 0:
      return JS_NewInt32(ctx, *p_D2CLIENT_RecentTradeId);
    case 1:
      // FIXME
      // char* tmp = UnicodeToAnsi((wchar_t*)(*p_D2CLIENT_RecentTradeName));
      //*rval = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, tmp));
      // delete[] tmp;
      // Temporary return value to keep it kosher
      return JS_NULL;
    case 2:
      return JS_NewInt32(ctx, *p_D2CLIENT_RecentTradeId);
  }
  return JS_FALSE;
}

JSAPI_FUNC(my_getUIFlag) {
  if (argc < 1 || !JS_IsNumber(argv[0])) {
    return JS_UNDEFINED;
  }

  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  int32_t nUIId;
  JS_ToInt32(ctx, &nUIId, argv[0]);
  return JS_NewBool(ctx, D2CLIENT_GetUIState(nUIId));
}

JSAPI_FUNC(my_getWaypoint) {
  if (argc < 1 || !JS_IsNumber(argv[0])) {
    return JS_FALSE;
  }

  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  int32_t nWaypointId;
  JS_ToInt32(ctx, &nWaypointId, argv[0]);

  if (nWaypointId > 40)
    nWaypointId = NULL;

  return JS_NewBool(ctx, (!!D2COMMON_CheckWaypoint((*p_D2CLIENT_WaypointTable), nWaypointId)));
}

JSAPI_FUNC(my_quitGame) {
  if (ClientState() != ClientStateMenu)
    D2CLIENT_ExitGame();

  // give the core a chance to shut down
  // TODO(ejt): commented out Shutdown while refactoring the Engine
  // Shutdown();
  TerminateProcess(GetCurrentProcess(), 0);

  return JS_FALSE;
}

JSAPI_FUNC(my_quit) {
  Vars.bQuitting = true;
  if (ClientState() != ClientStateMenu)
    D2CLIENT_ExitGame();

  return JS_FALSE;
}

JSAPI_FUNC(my_playSound) {
  // I need to take a closer look at the D2CLIENT_PlaySound function
  if (argc < 1 || !JS_IsNumber(argv[0])) {
    return JS_FALSE;
  }

  // int32_t nSoundId = JSVAL_TO_INT(JS_ARGV(cx, vp)[0]);
  // D2CLIENT_PlaySound(nSoundId);

  return JS_TRUE;
}

JSAPI_FUNC(my_say) {
  for (int32_t i = 0; i < argc; i++) {
    const char* str = JS_ToCString(ctx, argv[i]);
    if (!str) {
      return JS_EXCEPTION;
    }
    Say(L"%S", str);
    JS_FreeCString(ctx, str);
  }
  return JS_TRUE;
}

JSAPI_FUNC(my_clickParty) {
  JSValue rval = JS_FALSE;

  if (argc < 2 || !JS_IsObject(argv[0]) || !JS_IsNumber(argv[1]))
    return rval;

  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  UnitAny* myUnit = D2CLIENT_GetPlayerUnit();
  RosterUnit* pUnit = (RosterUnit*)JS_GetOpaque3(argv[0]);
  RosterUnit* mypUnit = *p_D2CLIENT_PlayerUnitList;

  if (!pUnit || !mypUnit)
    return rval;

  BOOL bFound = FALSE;

  for (RosterUnit* pScan = mypUnit; pScan; pScan = pScan->pNext)
    if (pScan->dwUnitId == pUnit->dwUnitId)
      bFound = TRUE;

  if (!bFound)
    return rval;

  int32_t nMode;
  JS_ToInt32(ctx, &nMode, argv[1]);

  BnetData* pData = (*p_D2LAUNCH_BnData);

  // Attempt to loot player, check first if it's hardcore
  if (nMode == 0 && pData && !(pData->nCharFlags & PLAYER_TYPE_HARDCORE))
    return rval;

  // Attempt to party a player who is already in party ^_~
  if (nMode == 2 && pUnit->wPartyId != 0xFFFF && mypUnit->wPartyId == pUnit->wPartyId && pUnit->wPartyId != 0xFFFF)
    return rval;

  // don't leave a party if we are in none!
  if (nMode == 3 && pUnit->wPartyId == 0xFFFF)
    return rval;
  else if (nMode == 3 && pUnit->wPartyId != 0xFFFF) {
    rval = JS_TRUE;
    D2CLIENT_LeaveParty();
    return rval;
  }

  if (nMode < 0 || nMode > 5)
    return rval;

  // Trying to click self
  if (pUnit->dwUnitId == myUnit->dwUnitId)
    return rval;

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

  rval = JS_TRUE;
  return rval;
}

JSAPI_FUNC(my_useStatPoint) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  uint32_t stat = 0;
  int32_t count = 1;
  JS_ToUint32(ctx, &stat, argv[0]);
  if (argc > 1) {
    JS_ToInt32(ctx, &count, argv[1]);
  }

  UseStatPoint(static_cast<WORD>(stat), count);
  return JS_UNDEFINED;
}

JSAPI_FUNC(my_useSkillPoint) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  uint32_t skill = 0;
  int32_t count = 1;
  JS_ToUint32(ctx, &skill, argv[0]);
  if (argc > 1) {
    JS_ToInt32(ctx, &count, argv[1]);
  }

  UseSkillPoint(static_cast<WORD>(skill), count);
  return JS_UNDEFINED;
}

JSAPI_FUNC(my_getBaseStat) {
  if (argc > 2) {
    const char *szStatName = NULL, *szTableName = NULL;
    int32_t nBaseStat = 0;
    int32_t nClassId = 0;
    int32_t nStat = -1;
    if (JS_IsString(argv[0])) {
      szTableName = JS_ToCString(ctx, argv[0]);
      if (!szTableName) {
        THROW_ERROR(ctx, "Invalid table value");
      }
    } else if (JS_IsNumber(argv[0]))
      JS_ToInt32(ctx, &nBaseStat, argv[0]);
    else {
      THROW_ERROR(ctx, "Invalid table value");
    }

    JS_ToInt32(ctx, &nClassId, argv[1]);

    if (JS_IsString(argv[2])) {
      szStatName = JS_ToCString(ctx, argv[2]);
      if (!szStatName) {
        JS_FreeCString(ctx, szTableName);
        THROW_ERROR(ctx, "Invalid column value");
      }
    } else if (JS_IsNumber(argv[2]))
      JS_ToInt32(ctx, &nStat, argv[2]);
    else {
      JS_FreeCString(ctx, szTableName);
      THROW_ERROR(ctx, "Invalid column value");
    }
    JSValue rval;
    FillBaseStat(ctx, &rval, nBaseStat, nClassId, nStat, szTableName, szStatName);
    JS_FreeCString(ctx, szTableName);
    JS_FreeCString(ctx, szStatName);
    return rval;
  }

  return JS_UNDEFINED;
}

JSAPI_FUNC(my_weaponSwitch) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  int32_t nParameter = NULL;
  if (argc > 0) {
    if (JS_ToInt32(ctx, &nParameter, argv[0])) {
      THROW_ERROR(ctx, "Could not convert value");
    }
  }

  if (nParameter == NULL) {
    // don't perform a weapon switch if current gametype is classic
    BnetData* pData = (*p_D2LAUNCH_BnData);
    if (pData) {
      if (!(pData->nCharFlags & PLAYER_TYPE_EXPAC))
        return JS_FALSE;
    } else
      THROW_ERROR(ctx, "Could not acquire BnData");

    BYTE aPacket[1];
    aPacket[0] = 0x60;
    D2NET_SendPacket(1, 1, aPacket);
    return JS_TRUE;
  }
  return JS_NewInt32(ctx, *p_D2CLIENT_bWeapSwitch);
}

JSAPI_FUNC(my_transmute) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  bool cubeOn = !!D2CLIENT_GetUIState(UI_CUBE);
  if (!cubeOn)
    D2CLIENT_SetUIState(UI_CUBE, TRUE);

  D2CLIENT_Transmute();

  if (!cubeOn)
    D2CLIENT_SetUIState(UI_CUBE, FALSE);

  return JS_NULL;
}

JSAPI_FUNC(my_getPlayerFlag) {
  if (argc != 3 || !JS_IsNumber(argv[0]) || !JS_IsNumber(argv[1]) || !JS_IsNumber(argv[2]))
    return JS_UNDEFINED;

  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  uint32_t nFirstUnitId = (uint32_t)-1;
  uint32_t nSecondUnitId = (uint32_t)-1;
  uint32_t nFlag;
  JS_ToUint32(ctx, &nFirstUnitId, argv[0]);
  JS_ToUint32(ctx, &nSecondUnitId, argv[1]);
  JS_ToUint32(ctx, &nFlag, argv[2]);

  return JS_NewBool(ctx, D2CLIENT_TestPvpFlag(nFirstUnitId, nSecondUnitId, nFlag));
}

JSAPI_FUNC(my_getMouseCoords) {
  bool nFlag = false, nReturn = false;

  if (argc > 0 && JS_IsNumber(argv[0]) || JS_IsBool(argv[0]))
    nFlag = !!JS_ToBool(ctx, argv[0]);
  if (argc > 1 && JS_IsNumber(argv[1]) || JS_IsBool(argv[1]))
    nReturn = !!JS_ToBool(ctx, argv[1]);

  JSValue pObj = NULL;

  POINT Coords = {static_cast<LONG>(*p_D2CLIENT_MouseX), static_cast<LONG>(*p_D2CLIENT_MouseY)};

  if (nFlag) {
    Coords.x += *p_D2CLIENT_ViewportX;
    Coords.y += *p_D2CLIENT_ViewportY;

    D2COMMON_AbsScreenToMap(&Coords.x, &Coords.y);
  }

  JSValue jsX = JS_NewInt64(ctx, Coords.x);
  JSValue jsY = JS_NewInt64(ctx, Coords.y);

  if (nReturn) {
    pObj = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, pObj, "x", jsX);
    JS_SetPropertyStr(ctx, pObj, "y", jsY);
  } else {
    pObj = JS_NewArray(ctx);
    JS_SetPropertyUint32(ctx, pObj, 0, jsX);
    JS_SetPropertyUint32(ctx, pObj, 1, jsY);
  }

  return pObj;
}

JSAPI_FUNC(my_submitItem) {
  JSValue rval = JS_NULL;

  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  if (UnitAny* pUnit = D2CLIENT_GetCursorItem()) {
    if (D2CLIENT_GetPlayerUnit()->dwAct == 1) {
      if (GetPlayerArea() == D2CLIENT_GetPlayerUnit()->pAct->pMisc->dwStaffTombLevel) {
        *p_D2CLIENT_CursorItemMode = 3;
        BYTE aPacket[17] = {NULL};
        aPacket[0] = 0x44;
        *(DWORD*)&aPacket[1] = D2CLIENT_GetPlayerUnit()->dwUnitId;
        *(DWORD*)&aPacket[5] = *p_D2CLIENT_OrificeId;
        *(DWORD*)&aPacket[9] = pUnit->dwUnitId;
        *(DWORD*)&aPacket[13] = 3;
        D2NET_SendPacket(17, 1, aPacket);
        rval = JS_TRUE;
      } else
        rval = JS_FALSE;
    } else if (D2CLIENT_GetPlayerUnit()->dwAct == 0 || D2CLIENT_GetPlayerUnit()->dwAct == 4)  // dwAct is 0-4, not 1-5
    {
      if (*p_D2CLIENT_RecentInteractId && D2COMMON_IsTownByLevelNo(GetPlayerArea())) {
        D2CLIENT_SubmitItem(pUnit->dwUnitId);
        rval = JS_TRUE;
      } else
        rval = JS_FALSE;
    } else
      rval = JS_FALSE;
  } else
    rval = JS_FALSE;

  return rval;
}

JSAPI_FUNC(my_getIsTalkingNPC) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  return JS_NewBool(ctx, IsScrollingText());
}

JSAPI_FUNC(my_getInteractedNPC) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  UnitAny* pNPC = D2CLIENT_GetCurrentInteractingNPC();
  if (!pNPC) {
    return JS_FALSE;
  }

  myUnit* pmyUnit = new myUnit;  // leaked?
  if (!pmyUnit)
    return JS_UNDEFINED;

  char szName[256] = "";
  pmyUnit->_dwPrivateType = PRIVATE_UNIT;
  pmyUnit->dwClassId = pNPC->dwTxtFileNo;
  pmyUnit->dwMode = pNPC->dwMode;
  pmyUnit->dwType = pNPC->dwType;
  pmyUnit->dwUnitId = pNPC->dwUnitId;
  strcpy_s(pmyUnit->szName, sizeof(pmyUnit->szName), szName);

  return BuildObject(ctx, unit_class_id, pmyUnit);
}

JSAPI_FUNC(my_takeScreenshot) {
  Vars.bTakeScreenshot = true;
  return JS_UNDEFINED;
}

JSAPI_FUNC(my_moveNPC) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  if (!Vars.bEnableUnsupported) {
    THROW_WARNING(ctx, "moveNPC requires EnableUnsupported = true in d2bs.ini");
  }

  if (argc < 2)
    THROW_ERROR(ctx, "Not enough parameters were passed to moveNPC!");

  myUnit* pNpc = (myUnit*)JS_GetOpaque3(argv[0]);

  if (!pNpc || pNpc->dwType != 1)
    THROW_ERROR(ctx, "Invalid NPC passed to moveNPC!");

  uint32_t dwX;
  uint32_t dwY;
  JS_ToUint32(ctx, &dwX, argv[1]);
  JS_ToUint32(ctx, &dwY, argv[2]);

  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  BYTE aPacket[17];
  aPacket[0] = 0x59;
  *(DWORD*)&aPacket[1] = pNpc->dwType;
  *(DWORD*)&aPacket[5] = pNpc->dwUnitId;
  *(DWORD*)&aPacket[9] = dwX;
  *(DWORD*)&aPacket[13] = dwY;

  D2NET_SendPacket(sizeof(aPacket), 1, aPacket);
  return JS_TRUE;
}

JSAPI_FUNC(my_revealLevel) {
  UnitAny* unit = D2CLIENT_GetPlayerUnit();

  if (!unit) {
    return JS_UNDEFINED;
  }

  Level* level = unit->pPath->pRoom1->pRoom2->pLevel;

  if (!level) {
    return JS_UNDEFINED;
  }

  BOOL bDrawPresets = false;

  if (argc == 1 && JS_IsBool(argv[0])) {
    bDrawPresets = !!JS_ToBool(ctx, argv[0]);
  }
  AutoCriticalRoom cRoom;
  if (!GameReady()) {
    return JS_UNDEFINED;
  }

  for (Room2* room = level->pRoom2First; room; room = room->pRoom2Next) {
    RevealRoom(room, bDrawPresets);
  }
  return JS_UNDEFINED;
}