#include "JSRoom.h"
#include "CriticalSections.h"
#include "JSPresetUnit.h"
#include "D2Structs.h"
#include "D2Ptrs.h"

EMPTY_CTOR(room)

JSAPI_PROP(room_getProperty) {
  Room2* pRoom2 = (Room2*)JS_GetOpaque3(this_val);

  if (!pRoom2)
    return JS_UNDEFINED;

  switch (magic) {
    case ROOM_NUM:
      if (pRoom2->dwPresetType != 2)
        return JS_NewInt32(ctx, -1);
      else if (pRoom2->pType2Info)
        return JS_NewInt32(ctx, pRoom2->pType2Info->dwRoomNumber);
      break;
    case ROOM_XPOS:
      return JS_NewInt32(ctx, pRoom2->dwPosX);
      break;
    case ROOM_YPOS:
      return JS_NewInt32(ctx, pRoom2->dwPosY);
      break;
    case ROOM_XSIZE:
      return JS_NewInt32(ctx, pRoom2->dwSizeX * 5);
      break;
    case ROOM_YSIZE:
      return JS_NewInt32(ctx, pRoom2->dwSizeY * 5);
      break;
    case ROOM_AREA:
    case ROOM_LEVEL:
      if (pRoom2->pLevel)
        return JS_NewInt32(ctx, pRoom2->pLevel->dwLevelNo);
      break;

    case ROOM_CORRECTTOMB:
      if (pRoom2->pLevel && pRoom2->pLevel->pMisc && pRoom2->pLevel->pMisc->dwStaffTombLevel)
        return JS_NewInt32(ctx, pRoom2->pLevel->pMisc->dwStaffTombLevel);
      break;

    default:
      break;
  }

  return JS_UNDEFINED;
}

JSAPI_FUNC(room_getNext) {
  (argc);

  Room2* pRoom2 = (Room2*)JS_GetOpaque3(this_val);
  if (!pRoom2) {
    return JS_FALSE;
  }

  pRoom2 = pRoom2->pRoom2Next;
  if (!pRoom2) {
    return JS_FALSE;
  }

  JS_SetOpaque(this_val, pRoom2);
  return JS_TRUE;
}

JSAPI_FUNC(room_getPresetUnits) {
  Room2* pRoom2 = (Room2*)JS_GetOpaque3(this_val);

  uint32_t nType = NULL;
  uint32_t nClass = NULL;

  if (argc > 0 && JS_IsNumber(argv[0]))
    JS_ToUint32(ctx, &nType, argv[0]);
  if (argc > 1 && JS_IsNumber(argv[1]))
    JS_ToUint32(ctx, &nClass, argv[1]);

  bool bAdded = FALSE;
  DWORD dwArrayCount = NULL;

  AutoCriticalRoom* cRoom = new AutoCriticalRoom;
  if (!pRoom2 || !GameReady()) {
    delete cRoom;
    return JS_TRUE;
  }

  if (!pRoom2->pRoom1) {
    bAdded = TRUE;
    D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
  }

  JSValue pReturnArray = JS_NewArray(ctx);
  for (PresetUnit* pUnit = pRoom2->pPreset; pUnit; pUnit = pUnit->pPresetNext) {
    if ((pUnit->dwType == nType || nType == NULL) && (pUnit->dwTxtFileNo == nClass || nClass == NULL)) {
      myPresetUnit* mypUnit = new myPresetUnit;

      mypUnit->dwPosX = pUnit->dwPosX;
      mypUnit->dwPosY = pUnit->dwPosY;
      mypUnit->dwRoomX = pRoom2->dwPosX;
      mypUnit->dwRoomY = pRoom2->dwPosY;
      mypUnit->dwType = pUnit->dwType;
      mypUnit->dwId = pUnit->dwTxtFileNo;

      JSValue jsUnit = BuildObject(ctx, presetunit_class_id, NULL, 0, presetunit_props, _countof(presetunit_props), mypUnit);
      if (JS_IsException(jsUnit)) {
        delete cRoom;
        return JS_FALSE;
      }

      JS_SetPropertyUint32(ctx, pReturnArray, dwArrayCount, jsUnit);
      dwArrayCount++;
    }
  }

  if (bAdded)
    D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);

  delete cRoom;
  return pReturnArray;
}

JSAPI_FUNC(room_getCollisionTypeArray) {
  Room2* pRoom2 = (Room2*)JS_GetOpaque3(this_val);

  bool bAdded = FALSE;
  CollMap* pCol = NULL;

  AutoCriticalRoom* cRoom = new AutoCriticalRoom;
  if (!pRoom2 || !GameReady()) {
    delete cRoom;
    return JS_UNDEFINED;
  }

  if (!pRoom2->pRoom1) {
    bAdded = TRUE;
    D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
  }

  JSValue jsobjy = JS_NewArray(ctx);
  if (!jsobjy)
    return JS_UNDEFINED;

  if (pRoom2->pRoom1)
    pCol = pRoom2->pRoom1->Coll;

  if (!pCol) {
    if (bAdded)
      D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
    delete cRoom;
    return JS_UNDEFINED;
  }

  int x = pCol->dwPosGameX - pRoom2->pLevel->dwPosX * 5;
  int y = pCol->dwPosGameY - pRoom2->pLevel->dwPosY * 5;
  int nCx = pCol->dwSizeGameX;
  int nCy = pCol->dwSizeGameY;

  int nLimitX = x + nCx;
  int nLimitY = y + nCy;

  int nCurrentArrayY = NULL;

  WORD* p = pCol->pMapStart;
  for (int j = y; j < nLimitY; j++) {
    int nCurrentArrayX = 0;
    for (int i = x; i < nLimitX; i++) {
      JSValue nNode = JS_NewInt32(ctx, *p);
      JS_SetPropertyUint32(ctx, jsobjy, nCurrentArrayY * nCx + nCurrentArrayX, nNode);
      nCurrentArrayX++;
      p++;
    }
    nCurrentArrayY++;
  }

  if (bAdded)
    D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);

  delete cRoom;
  return jsobjy;
}

JSAPI_FUNC(room_getCollision) {
  Room2* pRoom2 = (Room2*)JS_GetOpaque3(this_val);

  bool bAdded = FALSE;
  CollMap* pCol = NULL;

  AutoCriticalRoom* cRoom = new AutoCriticalRoom;
  if (!pRoom2 || !GameReady()) {
    delete cRoom;
    return JS_UNDEFINED;
  }

  JSValue jsobjy = JS_NewArray(ctx);
  if (!jsobjy)
    return JS_UNDEFINED;

  if (!pRoom2->pRoom1) {
    bAdded = TRUE;
    D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
  }

  if (pRoom2->pRoom1)
    pCol = pRoom2->pRoom1->Coll;

  if (!pCol) {
    if (bAdded)
      D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
    delete cRoom;
    return JS_UNDEFINED;
  }

  int x = pCol->dwPosGameX - pRoom2->pLevel->dwPosX * 5;
  int y = pCol->dwPosGameY - pRoom2->pLevel->dwPosY * 5;
  int nCx = pCol->dwSizeGameX;
  int nCy = pCol->dwSizeGameY;

  int nLimitX = x + nCx;
  int nLimitY = y + nCy;

  int nCurrentArrayY = NULL;

  WORD* p = pCol->pMapStart;
  for (int j = y; j < nLimitY; j++) {
    JSValue jsobjx = JS_NewArray(ctx);

    int nCurrentArrayX = 0;
    for (int i = x; i < nLimitX; i++) {
      JSValue nNode = JS_NewInt32(ctx, *p);
      JS_SetPropertyUint32(ctx, jsobjx, nCurrentArrayX, nNode);
      nCurrentArrayX++;
      p++;
    }

    JS_SetPropertyUint32(ctx, jsobjy, nCurrentArrayY, jsobjx);
    nCurrentArrayY++;
  }

  if (bAdded)
    D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);

  delete cRoom;
  return jsobjy;
}

JSAPI_FUNC(room_getNearby) {
  (argc);

  Room2* pRoom2 = (Room2*)JS_GetOpaque3(this_val);

  JSValue jsobj = JS_NewArray(ctx);
  if (!jsobj)
    return JS_UNDEFINED;

  for (DWORD i = 0; i < pRoom2->dwRoomsNear; ++i) {
    JSValue jsroom = BuildObject(ctx, room_class_id, room_methods, _countof(room_methods), room_props, _countof(room_methods), pRoom2->pRoom2Near[i]);
    if (JS_IsException(jsroom)) {
      return JS_UNDEFINED;
    }
    JS_SetPropertyUint32(ctx, jsobj, i, jsroom);
  }

  return jsobj;
}

// Don't know whether it works or not
JSAPI_FUNC(room_getStat) {
  JSValue rval = JS_NULL;
  Room2* pRoom2 = (Room2*)JS_GetOpaque3(this_val);

  if (argc < 1 || !JS_IsNumber(argv[0]))
    return rval;

  int32_t nStat;
  JS_ToInt32(ctx, &nStat, argv[0]);

  bool bAdded = false;

  AutoCriticalRoom* cRoom = new AutoCriticalRoom;
  if (!pRoom2 || !GameReady()) {
    delete cRoom;
    return rval;
  }
  if (!pRoom2->pRoom1) {
    bAdded = true;
    D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
  }

  if (!pRoom2->pRoom1) {
    delete cRoom;
    return rval;
  }

  if (nStat == 0)  // xStart
    rval = JS_NewInt32(ctx, (pRoom2->pRoom1->dwXStart));
  else if (nStat == 1)
    rval = JS_NewInt32(ctx, (pRoom2->pRoom1->dwYStart));
  else if (nStat == 2)
    rval = JS_NewInt32(ctx, (pRoom2->pRoom1->dwXSize));
  else if (nStat == 3)
    rval = JS_NewInt32(ctx, (pRoom2->pRoom1->dwYSize));
  else if (nStat == 4)
    rval = JS_NewInt32(ctx, (pRoom2->dwPosX));
  else if (nStat == 5)
    rval = JS_NewInt32(ctx, (pRoom2->dwPosY));
  else if (nStat == 6)
    rval = JS_NewInt32(ctx, (pRoom2->dwSizeX));
  else if (nStat == 7)
    rval = JS_NewInt32(ctx, (pRoom2->dwSizeY));
  //	else if(nStat == 8)
  //		*rval = INT_TO_JSVAL(pRoom2->pRoom1->dwYStart); // God knows??!!??!?!?!?!
  else if (nStat == 9)
    rval = JS_NewInt32(ctx, (pRoom2->pRoom1->Coll->dwPosGameX));
  else if (nStat == 10)
    rval = JS_NewInt32(ctx, (pRoom2->pRoom1->Coll->dwPosGameY));
  else if (nStat == 11)
    rval = JS_NewInt32(ctx, (pRoom2->pRoom1->Coll->dwSizeGameX));
  else if (nStat == 12)
    rval = JS_NewInt32(ctx, (pRoom2->pRoom1->Coll->dwSizeGameY));
  else if (nStat == 13)
    rval = JS_NewInt32(ctx, (pRoom2->pRoom1->Coll->dwPosRoomX));
  else if (nStat == 14)
    rval = JS_NewInt32(ctx, (pRoom2->pRoom1->Coll->dwPosGameY));
  else if (nStat == 15)
    rval = JS_NewInt32(ctx, (pRoom2->pRoom1->Coll->dwSizeRoomX));
  else if (nStat == 16)
    rval = JS_NewInt32(ctx, (pRoom2->pRoom1->Coll->dwSizeRoomY));

  if (bAdded)
    D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
  delete cRoom;

  return rval;
}

JSAPI_FUNC(room_getFirst) {
  Room2* pRoom2 = (Room2*)JS_GetOpaque3(this_val);
  if (!pRoom2 || !pRoom2->pLevel || !pRoom2->pLevel->pRoom2First)
    return JS_UNDEFINED;

  return BuildObject(ctx, room_class_id, room_methods, _countof(room_methods), room_props, _countof(room_props), pRoom2->pLevel->pRoom2First);
}

JSAPI_FUNC(room_unitInRoom) {
  Room2* pRoom2 = (Room2*)JS_GetOpaque3(this_val);
  if (!pRoom2 || argc < 1 || !JS_IsObject(argv[0]))
    return JS_UNDEFINED;

  myUnit* pmyUnit = (myUnit*)JS_GetOpaque3(argv[0]);
  if (!pmyUnit || (pmyUnit->_dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
    return JS_UNDEFINED;

  UnitAny* pUnit = D2CLIENT_FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

  if (!pUnit)
    return JS_UNDEFINED;

  Room1* pRoom1 = D2COMMON_GetRoomFromUnit(pUnit);

  if (!pRoom1 || !pRoom1->pRoom2)
    return JS_UNDEFINED;

  return JS_NewBool(ctx, pRoom1->pRoom2 == pRoom2);
}

JSAPI_FUNC(room_reveal) {
  Room2* pRoom2 = (Room2*)JS_GetOpaque3(this_val);

  BOOL bDrawPresets = false;
  if (argc == 1 && JS_IsBool(argv[0]))
    bDrawPresets = !!JS_ToBool(ctx, argv[0]);

  AutoCriticalRoom* cRoom = new AutoCriticalRoom;
  if (!pRoom2 || !GameReady()) {
    delete cRoom;
    return JS_UNDEFINED;
  }

  delete cRoom;
  return JS_NewBool(ctx, RevealRoom(pRoom2, bDrawPresets));
}

JSAPI_FUNC(my_getRoom) {
  if (!WaitForGameReady())
    THROW_ERROR(ctx, "Get Room Game not ready");

  AutoCriticalRoom* cRoom = new AutoCriticalRoom;

  if (argc == 1 && JS_IsNumber(argv[0])) {
    uint32_t levelId;
    JS_ToUint32(ctx, &levelId, argv[0]);
    if (levelId != 0)  // 1 Parameter, AreaId
    {
      Level* pLevel = GetLevel(levelId);

      if (!pLevel || !pLevel->pRoom2First) {
        delete cRoom;
        return JS_UNDEFINED;
      }

      JSValue jsroom = BuildObject(ctx, room_class_id, room_methods, _countof(room_methods), room_props, _countof(room_props), pLevel->pRoom2First);
      if (!jsroom) {
        delete cRoom;
        return JS_UNDEFINED;
      }
      delete cRoom;
      return jsroom;
    } else if (levelId == 0) {
      Room1* pRoom1 = D2COMMON_GetRoomFromUnit(D2CLIENT_GetPlayerUnit());

      if (!pRoom1 || !pRoom1->pRoom2) {
        delete cRoom;
        return JS_UNDEFINED;
      }

      JSValue jsroom = BuildObject(ctx, room_class_id, room_methods, _countof(room_methods), room_props, _countof(room_props), pRoom1->pRoom2);
      delete cRoom;
      if (!jsroom)
        return JS_UNDEFINED;

      return jsroom;
    }
  } else if (argc == 3 || argc == 2)  // area ,x and y
  {
    Level* pLevel = NULL;

    uint32_t levelId;
    JS_ToUint32(ctx, &levelId, argv[0]);
    if (argc == 3)
      pLevel = GetLevel(levelId);
    else if (D2CLIENT_GetPlayerUnit() && D2CLIENT_GetPlayerUnit()->pPath && D2CLIENT_GetPlayerUnit()->pPath->pRoom1 && D2CLIENT_GetPlayerUnit()->pPath->pRoom1->pRoom2)
      pLevel = D2CLIENT_GetPlayerUnit()->pPath->pRoom1->pRoom2->pLevel;

    if (!pLevel || !pLevel->pRoom2First) {
      delete cRoom;
      return JS_UNDEFINED;
    }

    uint32_t nX = NULL;
    uint32_t nY = NULL;
    if (argc == 2) {
      JS_ToUint32(ctx, &nX, argv[0]);
      JS_ToUint32(ctx, &nY, argv[1]);
    } else if (argc == 3) {
      JS_ToUint32(ctx, &nX, argv[1]);
      JS_ToUint32(ctx, &nY, argv[2]);
    }
    if (!nX || !nY) {
      delete cRoom;
      return JS_UNDEFINED;
    }

    // Scan for the room with the matching x,y coordinates.
    for (Room2* pRoom = pLevel->pRoom2First; pRoom; pRoom = pRoom->pRoom2Next) {
      bool bAdded = FALSE;
      if (!pRoom->pRoom1) {
        D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
        bAdded = TRUE;
      }

      CollMap* map = pRoom->pRoom1->Coll;
      if (nX >= map->dwPosGameX && nY >= map->dwPosGameY && nX < (map->dwPosGameX + map->dwSizeGameX) && nY < (map->dwPosGameY + map->dwSizeGameY)) {
        if (bAdded)
          D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);

        JSValue jsroom = BuildObject(ctx, room_class_id, room_methods, _countof(room_methods), room_props, _countof(room_props), pRoom);
        delete cRoom;
        if (!jsroom)
          return JS_UNDEFINED;

        return jsroom;
      }

      if (bAdded)
        D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
    }

    JSValue jsroom = BuildObject(ctx, room_class_id, room_methods, _countof(room_methods), room_props, _countof(room_props), pLevel->pRoom2First);
    delete cRoom;
    if (!jsroom)
      return JS_UNDEFINED;

    return jsroom;
  } else {
    JSValue jsroom = BuildObject(ctx, room_class_id, room_methods, _countof(room_methods), room_props, _countof(room_props),
                                 D2CLIENT_GetPlayerUnit()->pPath->pRoom1->pRoom2->pLevel->pRoom2First);
    delete cRoom;
    if (!jsroom)
      return JS_UNDEFINED;

    return jsroom;
  }
  delete cRoom;
  return JS_UNDEFINED;
}
