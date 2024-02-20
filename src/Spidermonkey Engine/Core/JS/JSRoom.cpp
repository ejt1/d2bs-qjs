#include "JSRoom.h"

#include "Bindings.h"
#include "CriticalSections.h"
#include "D2Helpers.h"
#include "JSPresetUnit.h"
#include "JSUnit.h"

JSObject* RoomWrap::Instantiate(JSContext* ctx, D2DrlgRoomStrc* room) {
  JS::RootedObject global(ctx, JS::CurrentGlobalOrNull(ctx));
  JS::RootedValue constructor_val(ctx);
  if (!JS_GetProperty(ctx, global, "Room", &constructor_val)) {
    JS_ReportErrorASCII(ctx, "Could not find constructor object for Room");
    return nullptr;
  }
  if (!constructor_val.isObject()) {
    JS_ReportErrorASCII(ctx, "Room is not a constructor");
    return nullptr;
  }
  JS::RootedObject constructor(ctx, &constructor_val.toObject());

  JS::RootedObject obj(ctx, JS_New(ctx, constructor, JS::HandleValueArray::empty()));
  if (!obj) {
    JS_ReportErrorASCII(ctx, "Calling Room constructor failed");
    return nullptr;
  }
  RoomWrap* wrap = new RoomWrap(ctx, obj, room);
  if (!wrap) {
    JS_ReportOutOfMemory(ctx);
    return nullptr;
  }
  return obj;
}

void RoomWrap::Initialize(JSContext* ctx, JS::HandleObject target) {
  JS::RootedObject proto(ctx, JS_InitClass(ctx, target, nullptr, &m_class, trampoline<New>, 0, m_props, m_methods, nullptr, nullptr));
  if (!proto) {
    return;
  }

  // globals
  JS_DefineFunction(ctx, target, "getRoom", trampoline<GetRoom>, 0, JSPROP_ENUMERATE);
}

RoomWrap::RoomWrap(JSContext* ctx, JS::HandleObject obj, D2DrlgRoomStrc* room) : BaseObject(ctx, obj), pRoom(room) {
}

void RoomWrap::finalize(JSFreeOp* /*fop*/, JSObject* obj) {
  BaseObject* wrap = BaseObject::FromJSObject(obj);
  if (wrap) {
    delete wrap;
  }
}

bool RoomWrap::New(JSContext* ctx, JS::CallArgs& args) {
  JS::RootedObject newObject(ctx, JS_NewObjectForConstructor(ctx, &m_class, args));
  if (!newObject) {
    THROW_ERROR(ctx, "failed to instantiate room");
  }
  // BUG(ejt): ask ejt
  JS_DefineProperties(ctx, newObject, m_props);
  JS_DefineFunctions(ctx, newObject, m_methods);

  args.rval().setObject(*newObject);
  return true;
}

// properties
bool RoomWrap::GetNumber(JSContext* ctx, JS::CallArgs& args) {
  RoomWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgRoomStrc* room = wrap->pRoom;

  args.rval().setUndefined();
  if (room->dwPresetType != 2)
    args.rval().setInt32(-1);
  else if (room->pType2Info)
    args.rval().setInt32(room->pType2Info->dwRoomNumber);
  return true;
}

bool RoomWrap::GetX(JSContext* ctx, JS::CallArgs& args) {
  RoomWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgRoomStrc* room = wrap->pRoom;
  args.rval().setInt32(room->dwPosX);
  return true;
}

bool RoomWrap::GetY(JSContext* ctx, JS::CallArgs& args) {
  RoomWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgRoomStrc* room = wrap->pRoom;
  args.rval().setInt32(room->dwPosY);
  return true;
}

bool RoomWrap::GetSizeX(JSContext* ctx, JS::CallArgs& args) {
  RoomWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgRoomStrc* room = wrap->pRoom;
  args.rval().setInt32(room->dwSizeX * 5);
  return true;
}

bool RoomWrap::GetSizeY(JSContext* ctx, JS::CallArgs& args) {
  RoomWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgRoomStrc* room = wrap->pRoom;
  args.rval().setInt32(room->dwSizeY * 5);
  return true;
}

bool RoomWrap::GetArea(JSContext* ctx, JS::CallArgs& args) {
  RoomWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgRoomStrc* room = wrap->pRoom;
  if (room->pLevel) {
    args.rval().setInt32(room->pLevel->dwLevelNo);
    return true;
  }
  args.rval().setUndefined();
  return true;
}

bool RoomWrap::GetLevel(JSContext* ctx, JS::CallArgs& args) {
  RoomWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgRoomStrc* room = wrap->pRoom;
  if (room->pLevel) {
    args.rval().setInt32(room->pLevel->dwLevelNo);
    return true;
  }
  args.rval().setUndefined();
  return true;
}

bool RoomWrap::GetCorrectTomb(JSContext* ctx, JS::CallArgs& args) {
  RoomWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgRoomStrc* room = wrap->pRoom;
  if (room->pLevel && room->pLevel->pMisc && room->pLevel->pMisc->dwStaffTombLevel) {
    args.rval().setInt32(room->pLevel->pMisc->dwStaffTombLevel);
    return true;
  }
  args.rval().setUndefined();
  return true;
}

// functions
bool RoomWrap::GetFirst(JSContext* ctx, JS::CallArgs& args) {
  RoomWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgRoomStrc* room = wrap->pRoom;
  if (!room || !room->pLevel || !room->pLevel->pRoom2First) {
    args.rval().setUndefined();
    return true;
  }

  JS::RootedObject obj(ctx, RoomWrap::Instantiate(ctx, room->pLevel->pRoom2First));
  if (!obj) {
    THROW_ERROR(ctx, "failed to instantiate room");
  }
  args.rval().setObject(*obj);
  return true;
}

bool RoomWrap::GetNext(JSContext* ctx, JS::CallArgs& args) {
  RoomWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgRoomStrc* room = wrap->pRoom;
  if (!room->pRoom2Next) {
    args.rval().setBoolean(false);
    return true;
  }
  wrap->pRoom = room->pRoom2Next;
  args.rval().setBoolean(true);
  return true;
}

bool RoomWrap::Reveal(JSContext* ctx, JS::CallArgs& args) {
  RoomWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgRoomStrc* room = wrap->pRoom;
  BOOL bDrawPresets = false;
  if (args.length() == 1 && args[0].isBoolean())
    bDrawPresets = args[0].toBoolean();

  AutoCriticalRoom cRoom;
  if (!room || !GameReady()) {
    args.rval().setUndefined();
    return true;
  }
  args.rval().setBoolean(room->Reveal(bDrawPresets));
  return true;
}

bool RoomWrap::GetPresetUnits(JSContext* ctx, JS::CallArgs& args) {
  RoomWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgRoomStrc* room = wrap->pRoom;
  uint32_t nType = NULL;
  uint32_t nClass = NULL;

  if (args.length() > 0 && args[0].isNumber())
    JS::ToUint32(ctx, args[0], &nType);
  if (args.length() > 1 && args[1].isNumber())
    JS::ToUint32(ctx, args[1], &nClass);

  bool bAdded = FALSE;
  DWORD dwArrayCount = NULL;

  AutoCriticalRoom cRoom;
  if (!room || !GameReady()) {
    args.rval().setUndefined();
    return true;
  }

  if (!room->pRoom1) {
    bAdded = TRUE;
    D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, room->pLevel->dwLevelNo, room->dwPosX, room->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
  }

  JS::RootedObject pReturnArray(ctx, JS::NewArrayObject(ctx, 0));
  for (D2PresetUnitStrc* pUnit = room->pPreset; pUnit; pUnit = pUnit->pPresetNext) {
    if ((pUnit->dwType == nType || nType == NULL) && (pUnit->dwTxtFileNo == nClass || nClass == NULL)) {
      JS::RootedObject jsUnit(ctx, PresetUnitWrap::Instantiate(ctx, pUnit, room, room->pLevel ? room->pLevel->dwLevelNo : 0));
      if (!jsUnit) {
        args.rval().setBoolean(false);
        return true;
      }

      JS_SetElement(ctx, pReturnArray, dwArrayCount, jsUnit);
      dwArrayCount++;
    }
  }

  if (bAdded)
    D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, room->pLevel->dwLevelNo, room->dwPosX, room->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);

  args.rval().setObject(*pReturnArray);
  return true;
}

bool RoomWrap::GetCollision(JSContext* ctx, JS::CallArgs& args) {
  RoomWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgRoomStrc* pRoom2 = wrap->pRoom;
  bool bAdded = FALSE;
  D2DrlgCoordsStrc* pCol = NULL;

  AutoCriticalRoom cRoom;
  if (!pRoom2 || !GameReady()) {
    args.rval().setUndefined();
    return true;
  }

  JS::RootedObject jsobjy(ctx, JS::NewArrayObject(ctx, 0));
  if (!jsobjy) {
    args.rval().setUndefined();
    return true;
  }

  if (!pRoom2->pRoom1) {
    bAdded = TRUE;
    D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
  }

  if (pRoom2->pRoom1)
    pCol = pRoom2->pRoom1->pCoords;

  if (!pCol) {
    if (bAdded)
      D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
    args.rval().setUndefined();
    return true;
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
    JS::RootedObject jsobjx(ctx, JS::NewArrayObject(ctx, 0));

    int nCurrentArrayX = 0;
    for (int i = x; i < nLimitX; i++) {
      JS_SetElement(ctx, jsobjx, nCurrentArrayX, *p);
      nCurrentArrayX++;
      p++;
    }

    JS_SetElement(ctx, jsobjy, nCurrentArrayY, jsobjx);
    nCurrentArrayY++;
  }

  if (bAdded)
    D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);

  args.rval().setObject(*jsobjy);
  return true;
}

bool RoomWrap::GetCollisionA(JSContext* ctx, JS::CallArgs& args) {
  RoomWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgRoomStrc* pRoom2 = wrap->pRoom;
  bool bAdded = FALSE;
  D2DrlgCoordsStrc* pCol = NULL;

  AutoCriticalRoom cRoom;
  if (!pRoom2 || !GameReady()) {
    args.rval().setUndefined();
    return true;
  }

  if (!pRoom2->pRoom1) {
    bAdded = TRUE;
    D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
  }

  JS::RootedObject jsobjy(ctx, JS::NewArrayObject(ctx, 0));
  if (!jsobjy) {
    args.rval().setUndefined();
    return true;
  }

  if (pRoom2->pRoom1)
    pCol = pRoom2->pRoom1->pCoords;

  if (!pCol) {
    if (bAdded)
      D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
    args.rval().setUndefined();
    return true;
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
      JS_SetElement(ctx, jsobjy, nCurrentArrayY * nCx + nCurrentArrayX, *p);
      nCurrentArrayX++;
      p++;
    }
    nCurrentArrayY++;
  }

  if (bAdded)
    D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);

  args.rval().setObject(*jsobjy);
  return true;
}

bool RoomWrap::GetNearby(JSContext* ctx, JS::CallArgs& args) {
  RoomWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgRoomStrc* pRoom2 = wrap->pRoom;
  JS::RootedObject jsobj(ctx, JS::NewArrayObject(ctx, 0));
  if (!jsobj) {
    args.rval().setUndefined();
    return true;
  }

  for (DWORD i = 0; i < pRoom2->dwRoomsNear; ++i) {
    JS::RootedObject jsroom(ctx, RoomWrap::Instantiate(ctx, pRoom2->pRoom2Near[i]));
    if (!jsroom) {
      args.rval().setUndefined();
      return true;
    }
    JS_SetElement(ctx, jsobj, i, jsroom);
  }
  args.rval().setObject(*jsobj);
  return true;
}

bool RoomWrap::GetStat(JSContext* ctx, JS::CallArgs& args) {
  RoomWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgRoomStrc* pRoom2 = wrap->pRoom;
  args.rval().setNull();
  if (args.length() < 1 || !args[0].isInt32())
    return true;

  int32_t nStat = args[0].toInt32();

  bool bAdded = false;

  AutoCriticalRoom cRoom;
  if (!pRoom2 || !GameReady()) {
    return true;
  }
  if (!pRoom2->pRoom1) {
    bAdded = true;
    D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
  }

  if (!pRoom2->pRoom1) {
    return true;
  }

  if (nStat == 0)  // xStart
    args.rval().setInt32(pRoom2->pRoom1->dwXStart);
  else if (nStat == 1)
    args.rval().setInt32(pRoom2->pRoom1->dwYStart);
  else if (nStat == 2)
    args.rval().setInt32(pRoom2->pRoom1->dwXSize);
  else if (nStat == 3)
    args.rval().setInt32(pRoom2->pRoom1->dwYSize);
  else if (nStat == 4)
    args.rval().setInt32(pRoom2->dwPosX);
  else if (nStat == 5)
    args.rval().setInt32(pRoom2->dwPosY);
  else if (nStat == 6)
    args.rval().setInt32(pRoom2->dwSizeX);
  else if (nStat == 7)
    args.rval().setInt32(pRoom2->dwSizeY);
  //	else if(nStat == 8)
  //		*rval = INT_TO_JSVAL(pRoom2->pRoom1->dwYStart); // God knows??!!??!?!?!?!
  else if (nStat == 9)
    args.rval().setInt32(pRoom2->pRoom1->pCoords->dwPosGameX);
  else if (nStat == 10)
    args.rval().setInt32(pRoom2->pRoom1->pCoords->dwPosGameY);
  else if (nStat == 11)
    args.rval().setInt32(pRoom2->pRoom1->pCoords->dwSizeGameX);
  else if (nStat == 12)
    args.rval().setInt32(pRoom2->pRoom1->pCoords->dwSizeGameY);
  else if (nStat == 13)
    args.rval().setInt32(pRoom2->pRoom1->pCoords->dwPosRoomX);
  else if (nStat == 14)
    args.rval().setInt32(pRoom2->pRoom1->pCoords->dwPosGameY);
  else if (nStat == 15)
    args.rval().setInt32(pRoom2->pRoom1->pCoords->dwSizeRoomX);
  else if (nStat == 16)
    args.rval().setInt32(pRoom2->pRoom1->pCoords->dwSizeRoomY);

  if (bAdded)
    D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);

  return true;
}

bool RoomWrap::UnitInRoom(JSContext* ctx, JS::CallArgs& args) {
  RoomWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgRoomStrc* pRoom2 = wrap->pRoom;
  args.rval().setUndefined();
  if (!pRoom2 || args.length() < 1 || !args[0].isObject())
    return true;

  UnitWrap* unit_wrap;
  UNWRAP_OR_RETURN(ctx, &unit_wrap, args[0]);
  UnitWrap::UnitData* pmyUnit = unit_wrap->GetData();
  if (!pmyUnit || (pmyUnit->dwPrivateType & PRIVATE_UNIT) != PRIVATE_UNIT)
    return true;

  D2UnitStrc* pUnit = D2UnitStrc::FindUnit(pmyUnit->dwUnitId, pmyUnit->dwType);

  if (!pUnit)
    return true;

  D2ActiveRoomStrc* pRoom1 = D2COMMON_GetRoomFromUnit(pUnit);

  if (!pRoom1 || !pRoom1->pRoom2)
    return true;

  args.rval().setBoolean(pRoom1->pRoom2 == pRoom2);
  return true;
}

bool RoomWrap::GetRoom(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_ERROR(ctx, "Get Room Game not ready");

  AutoCriticalRoom cRoom;

  if (args.length() == 1 && args[0].isNumber()) {
    uint32_t levelId;
    JS::ToUint32(ctx, args[0], &levelId);
    if (levelId != 0)  // 1 Parameter, AreaId
    {
      D2DrlgLevelStrc* pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(levelId);
      if (!pLevel || !pLevel->pRoom2First) {
        args.rval().setUndefined();
        return true;
      }

      JS::RootedObject jsroom(ctx, RoomWrap::Instantiate(ctx, pLevel->pRoom2First));
      if (!jsroom) {
        args.rval().setUndefined();
        return true;
      }

      args.rval().setObject(*jsroom);
      return true;
    } else if (levelId == 0) {
      D2ActiveRoomStrc* pRoom1 = D2COMMON_GetRoomFromUnit(D2CLIENT_GetPlayerUnit());

      if (!pRoom1 || !pRoom1->pRoom2) {
        args.rval().setUndefined();
        return true;
      }

      JS::RootedObject jsroom(ctx, RoomWrap::Instantiate(ctx, pRoom1->pRoom2));
      if (!jsroom) {
        args.rval().setUndefined();
        return true;
      }

      args.rval().setObject(*jsroom);
      return true;
    }
  } else if (args.length() == 3 || args.length() == 2)  // area ,x and y
  {
    D2DrlgLevelStrc* pLevel = NULL;

    uint32_t levelId;
    JS::ToUint32(ctx, args[0], &levelId);
    if (args.length() == 3) {
      pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(levelId);
    } else if (D2CLIENT_GetPlayerUnit() && D2CLIENT_GetPlayerUnit()->pPath && D2CLIENT_GetPlayerUnit()->pPath->pRoom1 && D2CLIENT_GetPlayerUnit()->pPath->pRoom1->pRoom2)
      pLevel = D2CLIENT_GetPlayerUnit()->pPath->pRoom1->pRoom2->pLevel;

    if (!pLevel || !pLevel->pRoom2First) {
      args.rval().setUndefined();
      return true;
    }

    uint32_t nX = NULL;
    uint32_t nY = NULL;
    if (args.length() == 2) {
      JS::ToUint32(ctx, args[0], &nX);
      JS::ToUint32(ctx, args[1], &nY);
    } else if (args.length() == 3) {
      JS::ToUint32(ctx, args[1], &nX);
      JS::ToUint32(ctx, args[2], &nY);
    }
    if (!nX || !nY) {
      args.rval().setUndefined();
      return true;
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

        JS::RootedObject jsroom(ctx, RoomWrap::Instantiate(ctx, pRoom));
        if (!jsroom) {
          args.rval().setUndefined();
          return true;
        }

        args.rval().setObject(*jsroom);
        return true;
      }

      if (bAdded)
        D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
    }

    JS::RootedObject jsroom(ctx, RoomWrap::Instantiate(ctx, pLevel->pRoom2First));
    if (!jsroom) {
      args.rval().setUndefined();
      return true;
    }

    args.rval().setObject(*jsroom);
    return true;
  } else {
    JS::RootedObject jsroom(ctx, RoomWrap::Instantiate(ctx, D2CLIENT_GetPlayerUnit()->pPath->pRoom1->pRoom2->pLevel->pRoom2First));
    if (!jsroom) {
      args.rval().setUndefined();
      return true;
    }

    args.rval().setObject(*jsroom);
    return true;
  }

  args.rval().setUndefined();
  return true;
}

D2BS_BINDING_INTERNAL(RoomWrap, RoomWrap::Initialize)
