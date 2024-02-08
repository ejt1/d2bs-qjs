#include "JSRoom.h"

#include "Bindings.h"
#include "CriticalSections.h"
#include "D2Helpers.h"
#include "JSPresetUnit.h"
#include "JSUnit.h"

JSValue RoomWrap::Instantiate(JSContext* ctx, JSValue new_target, D2DrlgRoomStrc* room) {
  JSValue proto;
  if (JS_IsUndefined(new_target)) {
    proto = JS_GetClassProto(ctx, m_class_id);
  } else {
    proto = JS_GetPropertyStr(ctx, new_target, "prototype");
    if (JS_IsException(proto)) {
      return JS_EXCEPTION;
    }
  }
  JSValue obj = JS_NewObjectProtoClass(ctx, proto, m_class_id);
  JS_FreeValue(ctx, proto);
  if (JS_IsException(obj)) {
    return obj;
  }

  RoomWrap* wrap = new RoomWrap(ctx, room);
  if (!wrap) {
    JS_FreeValue(ctx, obj);
    return JS_ThrowOutOfMemory(ctx);
  }
  JS_SetOpaque(obj, wrap);

  return obj;
}

void RoomWrap::Initialize(JSContext* ctx, JSValue target) {
  JSClassDef def{};
  def.class_name = "Room";
  def.finalizer = [](JSRuntime* /*rt*/, JSValue val) {
    RoomWrap* wrap = static_cast<RoomWrap*>(JS_GetOpaque(val, m_class_id));
    if (wrap) {
      delete wrap;
    }
  };

  if (m_class_id == 0) {
    JS_NewClassID(&m_class_id);
  }
  JS_NewClass(JS_GetRuntime(ctx), m_class_id, &def);

  JSValue proto = JS_NewObject(ctx);
  JS_SetPropertyFunctionList(ctx, proto, m_proto_funcs, _countof(m_proto_funcs));

  JSValue obj = JS_NewCFunction2(ctx, New, "Room", 0, JS_CFUNC_constructor, 0);
  JS_SetConstructor(ctx, obj, proto);

  JS_SetClassProto(ctx, m_class_id, proto);
  JS_SetPropertyStr(ctx, target, "Room", obj);

  // globals
  JS_SetPropertyStr(ctx, target, "getRoom", JS_NewCFunction(ctx, GetRoom, "getRoom", 0));
}

RoomWrap::RoomWrap(JSContext* /*ctx*/, D2DrlgRoomStrc* room) : pRoom(room) {
}

JSValue RoomWrap::New(JSContext* /*ctx*/, JSValue /*new_target*/, int /*argc*/, JSValue* /*argv*/) {
  // TODO(ejt): empty constructor for compatibility with kolbot
  return JS_UNDEFINED;
}

// properties
JSValue RoomWrap::GetNumber(JSContext* ctx, JSValue this_val) {
  RoomWrap* wrap = static_cast<RoomWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2DrlgRoomStrc* room = wrap->pRoom;

  if (room->dwPresetType != 2)
    return JS_NewInt32(ctx, -1);
  else if (room->pType2Info)
    return JS_NewInt32(ctx, room->pType2Info->dwRoomNumber);
  return JS_UNDEFINED;
}

JSValue RoomWrap::GetX(JSContext* ctx, JSValue this_val) {
  RoomWrap* wrap = static_cast<RoomWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2DrlgRoomStrc* room = wrap->pRoom;
  return JS_NewInt32(ctx, room->dwPosX);
}

JSValue RoomWrap::GetY(JSContext* ctx, JSValue this_val) {
  RoomWrap* wrap = static_cast<RoomWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2DrlgRoomStrc* room = wrap->pRoom;
  return JS_NewInt32(ctx, room->dwPosY);
}

JSValue RoomWrap::GetSizeX(JSContext* ctx, JSValue this_val) {
  RoomWrap* wrap = static_cast<RoomWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2DrlgRoomStrc* room = wrap->pRoom;
  return JS_NewInt32(ctx, room->dwSizeX * 5);
}

JSValue RoomWrap::GetSizeY(JSContext* ctx, JSValue this_val) {
  RoomWrap* wrap = static_cast<RoomWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2DrlgRoomStrc* room = wrap->pRoom;
  return JS_NewInt32(ctx, room->dwSizeY * 5);
}

JSValue RoomWrap::GetArea(JSContext* ctx, JSValue this_val) {
  RoomWrap* wrap = static_cast<RoomWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2DrlgRoomStrc* room = wrap->pRoom;
  if (room->pLevel)
    return JS_NewInt32(ctx, room->pLevel->dwLevelNo);
  return JS_UNDEFINED;
}

JSValue RoomWrap::GetLevel(JSContext* ctx, JSValue this_val) {
  RoomWrap* wrap = static_cast<RoomWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2DrlgRoomStrc* room = wrap->pRoom;
  if (room->pLevel)
    return JS_NewInt32(ctx, room->pLevel->dwLevelNo);
  return JS_UNDEFINED;
}

JSValue RoomWrap::GetCorrectTomb(JSContext* ctx, JSValue this_val) {
  RoomWrap* wrap = static_cast<RoomWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2DrlgRoomStrc* room = wrap->pRoom;
  if (room->pLevel && room->pLevel->pMisc && room->pLevel->pMisc->dwStaffTombLevel)
    return JS_NewInt32(ctx, room->pLevel->pMisc->dwStaffTombLevel);
  return JS_UNDEFINED;
}

// functions
JSValue RoomWrap::GetFirst(JSContext* ctx, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  RoomWrap* wrap = static_cast<RoomWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2DrlgRoomStrc* room = wrap->pRoom;

  if (!room || !room->pLevel || !room->pLevel->pRoom2First)
    return JS_UNDEFINED;

  return RoomWrap::Instantiate(ctx, JS_UNDEFINED, room->pLevel->pRoom2First);
}

JSValue RoomWrap::GetNext(JSContext* /*ctx*/, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  RoomWrap* wrap = static_cast<RoomWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2DrlgRoomStrc* room = wrap->pRoom;
  if (!room->pRoom2Next) {
    return JS_FALSE;
  }
  wrap->pRoom = room->pRoom2Next;
  return JS_TRUE;
}

JSValue RoomWrap::Reveal(JSContext* ctx, JSValue this_val, int argc, JSValue* argv) {
  RoomWrap* wrap = static_cast<RoomWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2DrlgRoomStrc* room = wrap->pRoom;
  BOOL bDrawPresets = false;
  if (argc == 1 && JS_IsBool(argv[0]))
    bDrawPresets = !!JS_ToBool(ctx, argv[0]);

  AutoCriticalRoom cRoom;
  if (!room || !GameReady()) {
    return JS_UNDEFINED;
  }

  return JS_NewBool(ctx, room->Reveal(bDrawPresets));
}

JSValue RoomWrap::GetPresetUnits(JSContext* ctx, JSValue this_val, int argc, JSValue* argv) {
  RoomWrap* wrap = static_cast<RoomWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2DrlgRoomStrc* room = wrap->pRoom;
  uint32_t nType = NULL;
  uint32_t nClass = NULL;

  if (argc > 0 && JS_IsNumber(argv[0]))
    JS_ToUint32(ctx, &nType, argv[0]);
  if (argc > 1 && JS_IsNumber(argv[1]))
    JS_ToUint32(ctx, &nClass, argv[1]);

  bool bAdded = FALSE;
  DWORD dwArrayCount = NULL;

  AutoCriticalRoom cRoom;
  if (!room || !GameReady()) {
    return JS_UNDEFINED;
  }

  if (!room->pRoom1) {
    bAdded = TRUE;
    D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, room->pLevel->dwLevelNo, room->dwPosX, room->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
  }

  JSValue pReturnArray = JS_NewArray(ctx);
  for (D2PresetUnitStrc* pUnit = room->pPreset; pUnit; pUnit = pUnit->pPresetNext) {
    if ((pUnit->dwType == nType || nType == NULL) && (pUnit->dwTxtFileNo == nClass || nClass == NULL)) {
      JSValue jsUnit = PresetUnitWrap::Instantiate(ctx, JS_UNDEFINED, pUnit, room, room->pLevel ? room->pLevel->dwLevelNo : 0);
      if (JS_IsException(jsUnit)) {
        JS_FreeValue(ctx, pReturnArray);
        return JS_FALSE;
      }

      JS_SetPropertyUint32(ctx, pReturnArray, dwArrayCount, jsUnit);
      dwArrayCount++;
    }
  }

  if (bAdded)
    D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, room->pLevel->dwLevelNo, room->dwPosX, room->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);

  return pReturnArray;
}

JSValue RoomWrap::GetCollision(JSContext* ctx, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  RoomWrap* wrap = static_cast<RoomWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2DrlgRoomStrc* pRoom2 = wrap->pRoom;
  bool bAdded = FALSE;
  D2DrlgCoordsStrc* pCol = NULL;

  AutoCriticalRoom cRoom;
  if (!pRoom2 || !GameReady()) {
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
    pCol = pRoom2->pRoom1->pCoords;

  if (!pCol) {
    if (bAdded)
      D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
    JS_FreeValue(ctx, jsobjy);
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

  return jsobjy;
}

JSValue RoomWrap::GetCollisionA(JSContext* ctx, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  RoomWrap* wrap = static_cast<RoomWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2DrlgRoomStrc* pRoom2 = wrap->pRoom;
  bool bAdded = FALSE;
  D2DrlgCoordsStrc* pCol = NULL;

  AutoCriticalRoom cRoom;
  if (!pRoom2 || !GameReady()) {
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
    pCol = pRoom2->pRoom1->pCoords;

  if (!pCol) {
    if (bAdded)
      D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
    JS_FreeValue(ctx, jsobjy);
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

  return jsobjy;
}

JSValue RoomWrap::GetNearby(JSContext* ctx, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  RoomWrap* wrap = static_cast<RoomWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2DrlgRoomStrc* pRoom2 = wrap->pRoom;
  JSValue jsobj = JS_NewArray(ctx);
  if (!jsobj)
    return JS_UNDEFINED;

  for (DWORD i = 0; i < pRoom2->dwRoomsNear; ++i) {
    JSValue jsroom = RoomWrap::Instantiate(ctx, JS_UNDEFINED, pRoom2->pRoom2Near[i]);
    if (JS_IsException(jsroom)) {
      JS_FreeValue(ctx, jsobj);
      return JS_UNDEFINED;
    }
    JS_SetPropertyUint32(ctx, jsobj, i, jsroom);
  }

  return jsobj;
}

JSValue RoomWrap::GetStat(JSContext* ctx, JSValue this_val, int argc, JSValue* argv) {
  RoomWrap* wrap = static_cast<RoomWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  JSValue rval = JS_NULL;
  D2DrlgRoomStrc* pRoom2 = wrap->pRoom;
  if (argc < 1 || !JS_IsNumber(argv[0]))
    return rval;

  int32_t nStat;
  JS_ToInt32(ctx, &nStat, argv[0]);

  bool bAdded = false;

  AutoCriticalRoom cRoom;
  if (!pRoom2 || !GameReady()) {
    return rval;
  }
  if (!pRoom2->pRoom1) {
    bAdded = true;
    D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
  }

  if (!pRoom2->pRoom1) {
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
    rval = JS_NewInt32(ctx, (pRoom2->pRoom1->pCoords->dwPosGameX));
  else if (nStat == 10)
    rval = JS_NewInt32(ctx, (pRoom2->pRoom1->pCoords->dwPosGameY));
  else if (nStat == 11)
    rval = JS_NewInt32(ctx, (pRoom2->pRoom1->pCoords->dwSizeGameX));
  else if (nStat == 12)
    rval = JS_NewInt32(ctx, (pRoom2->pRoom1->pCoords->dwSizeGameY));
  else if (nStat == 13)
    rval = JS_NewInt32(ctx, (pRoom2->pRoom1->pCoords->dwPosRoomX));
  else if (nStat == 14)
    rval = JS_NewInt32(ctx, (pRoom2->pRoom1->pCoords->dwPosGameY));
  else if (nStat == 15)
    rval = JS_NewInt32(ctx, (pRoom2->pRoom1->pCoords->dwSizeRoomX));
  else if (nStat == 16)
    rval = JS_NewInt32(ctx, (pRoom2->pRoom1->pCoords->dwSizeRoomY));

  if (bAdded)
    D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);

  return rval;
}

JSValue RoomWrap::UnitInRoom(JSContext* ctx, JSValue this_val, int argc, JSValue* argv) {
  RoomWrap* wrap = static_cast<RoomWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2DrlgRoomStrc* pRoom2 = wrap->pRoom;
  if (!pRoom2 || argc < 1 || !JS_IsObject(argv[0]))
    return JS_UNDEFINED;

  JSUnit* pmyUnit = (JSUnit*)JS_GetOpaque3(argv[0]);
  if (!pmyUnit || (pmyUnit->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
    return JS_UNDEFINED;

  D2UnitStrc* pUnit = D2UnitStrc::FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

  if (!pUnit)
    return JS_UNDEFINED;

  D2ActiveRoomStrc* pRoom1 = D2COMMON_GetRoomFromUnit(pUnit);

  if (!pRoom1 || !pRoom1->pRoom2)
    return JS_UNDEFINED;

  return JS_NewBool(ctx, pRoom1->pRoom2 == pRoom2);
}

JSValue RoomWrap::GetRoom(JSContext* ctx, JSValue /*this_val*/, int argc, JSValue* argv) {
  if (!WaitForGameReady())
    THROW_ERROR(ctx, "Get Room Game not ready");

  AutoCriticalRoom cRoom;

  if (argc == 1 && JS_IsNumber(argv[0])) {
    uint32_t levelId;
    JS_ToUint32(ctx, &levelId, argv[0]);
    if (levelId != 0)  // 1 Parameter, AreaId
    {
      D2DrlgLevelStrc* pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(levelId);
      if (!pLevel || !pLevel->pRoom2First) {
        return JS_UNDEFINED;
      }

      JSValue jsroom = RoomWrap::Instantiate(ctx, JS_UNDEFINED, pLevel->pRoom2First);
      if (!jsroom) {
        return JS_UNDEFINED;
      }
      return jsroom;
    } else if (levelId == 0) {
      D2ActiveRoomStrc* pRoom1 = D2COMMON_GetRoomFromUnit(D2CLIENT_GetPlayerUnit());

      if (!pRoom1 || !pRoom1->pRoom2) {
        return JS_UNDEFINED;
      }

      JSValue jsroom = RoomWrap::Instantiate(ctx, JS_UNDEFINED, pRoom1->pRoom2);
      if (JS_IsException(jsroom))
        return JS_UNDEFINED;

      return jsroom;
    }
  } else if (argc == 3 || argc == 2)  // area ,x and y
  {
    D2DrlgLevelStrc* pLevel = NULL;

    uint32_t levelId;
    JS_ToUint32(ctx, &levelId, argv[0]);
    if (argc == 3) {
      pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(levelId);
    } else if (D2CLIENT_GetPlayerUnit() && D2CLIENT_GetPlayerUnit()->pPath && D2CLIENT_GetPlayerUnit()->pPath->pRoom1 && D2CLIENT_GetPlayerUnit()->pPath->pRoom1->pRoom2)
      pLevel = D2CLIENT_GetPlayerUnit()->pPath->pRoom1->pRoom2->pLevel;

    if (!pLevel || !pLevel->pRoom2First) {
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
      return JS_UNDEFINED;
    }

    // Scan for the room with the matching x,y coordinates.
    for (D2DrlgRoomStrc* pRoom = pLevel->pRoom2First; pRoom; pRoom = pRoom->pRoom2Next) {
      bool bAdded = FALSE;
      if (!pRoom->pRoom1) {
        D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
        bAdded = TRUE;
      }

      D2DrlgCoordsStrc* map = pRoom->pRoom1->pCoords;
      if (nX >= map->dwPosGameX && nY >= map->dwPosGameY && nX < (map->dwPosGameX + map->dwSizeGameX) && nY < (map->dwPosGameY + map->dwSizeGameY)) {
        if (bAdded)
          D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);

        JSValue jsroom = RoomWrap::Instantiate(ctx, JS_UNDEFINED, pRoom);
        if (JS_IsException(jsroom))
          return JS_UNDEFINED;

        return jsroom;
      }

      if (bAdded)
        D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
    }

    JSValue jsroom = RoomWrap::Instantiate(ctx, JS_UNDEFINED, pLevel->pRoom2First);
    if (JS_IsException(jsroom))
      return JS_UNDEFINED;

    return jsroom;
  } else {
    JSValue jsroom = RoomWrap::Instantiate(ctx, JS_UNDEFINED, D2CLIENT_GetPlayerUnit()->pPath->pRoom1->pRoom2->pLevel->pRoom2First);
    if (JS_IsException(jsroom))
      return JS_UNDEFINED;

    return jsroom;
  }
  return JS_UNDEFINED;
}

D2BS_BINDING_INTERNAL(RoomWrap, RoomWrap::Initialize)