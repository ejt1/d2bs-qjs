#include "JSPresetUnit.h"

#include "CriticalSections.h"
#include "JSPresetUnit.h"
#include "D2Helpers.h"
#include "JSGlobalClasses.h"

EMPTY_CTOR(presetunit)

CLASS_FINALIZER(presetunit) {
  JSPresetUnit* pUnit = (JSPresetUnit*)JS_GetOpaque3(val);

  if (pUnit) {
    JS_SetOpaque(val, NULL);
    delete pUnit;
  }
}

JSAPI_PROP(presetunit_getProperty) {
  JSPresetUnit* pUnit = (JSPresetUnit*)JS_GetOpaque3(this_val);

  if (!pUnit)
    return JS_UNDEFINED;

  switch (magic) {
    case PUNIT_TYPE:
      return JS_NewUint32(ctx, pUnit->dwType);
      break;
    case PUNIT_ROOMX:
      return JS_NewUint32(ctx, pUnit->dwRoomX);
      break;
    case PUNIT_ROOMY:
      return JS_NewUint32(ctx, pUnit->dwRoomY);
      break;
    case PUNIT_X:
      return JS_NewUint32(ctx, pUnit->dwPosX);
      break;
    case PUNIT_Y:
      return JS_NewUint32(ctx, pUnit->dwPosY);
      break;
    case PUNIT_ID:
      return JS_NewUint32(ctx, pUnit->dwId);
      break;
    case PUINT_LEVEL:
      return JS_NewUint32(ctx, pUnit->dwLevel);
    default:
      break;
  }

  return JS_UNDEFINED;
}

JSAPI_FUNC(my_getPresetUnits) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  if (argc < 1) {
    return JS_FALSE;
  }

  uint32_t levelId;
  JS_ToUint32(ctx, &levelId, argv[0]);
  D2DrlgLevelStrc* pLevel = GetLevel(levelId);

  if (!pLevel)
    THROW_ERROR(ctx, "getPresetUnits failed, couldn't access the level!");

  uint32_t nClassId = NULL;
  uint32_t nType = NULL;

  if (argc >= 2)
    JS_ToUint32(ctx, &nType, argv[1]);
  if (argc >= 3)
    JS_ToUint32(ctx, &nClassId, argv[2]);

  AutoCriticalRoom cRoom;

  bool bAddedRoom = FALSE;
  DWORD dwArrayCount = NULL;

  JSValue pReturnArray = JS_NewArray(ctx);
  for (D2DrlgRoomStrc* pRoom = pLevel->pRoom2First; pRoom; pRoom = pRoom->pRoom2Next) {
    bAddedRoom = FALSE;

    if (!pRoom->pPreset) {
      D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
      bAddedRoom = TRUE;
    }

    for (D2PresetUnitStrc* pUnit = pRoom->pPreset; pUnit; pUnit = pUnit->pPresetNext) {
      // Does it fit?
      if ((nType == NULL || pUnit->dwType == nType) && (nClassId == NULL || pUnit->dwTxtFileNo == nClassId)) {
        JSPresetUnit* mypUnit = new JSPresetUnit;

        mypUnit->dwPosX = pUnit->dwPosX;
        mypUnit->dwPosY = pUnit->dwPosY;
        mypUnit->dwRoomX = pRoom->dwPosX;
        mypUnit->dwRoomY = pRoom->dwPosY;
        mypUnit->dwType = pUnit->dwType;
        mypUnit->dwId = pUnit->dwTxtFileNo;
        mypUnit->dwLevel = levelId;

        JSValue unit = BuildObject(ctx, presetunit_class_id, FUNCLIST(presetunit_proto_funcs), mypUnit);
        if (JS_IsException(unit)) {
          delete mypUnit;
          JS_FreeValue(ctx, pReturnArray);
          THROW_ERROR(ctx, "Failed to build object?");
        }

        JS_SetPropertyUint32(ctx, pReturnArray, dwArrayCount, unit);

        dwArrayCount++;
      }
    }

    if (bAddedRoom) {
      D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
      bAddedRoom = FALSE;
    }
  }

  return pReturnArray;
}

JSAPI_FUNC(my_getPresetUnit) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  if (argc < 1) {
    return JS_FALSE;
  }

  uint32_t levelId;
  JS_ToUint32(ctx, &levelId, argv[0]);
  D2DrlgLevelStrc* pLevel = GetLevel(levelId);

  if (!pLevel)
    THROW_ERROR(ctx, "getPresetUnits failed, couldn't access the level!");

  uint32_t nClassId = NULL;
  uint32_t nType = NULL;

  if (argc >= 2)
    JS_ToUint32(ctx, &nType, argv[1]);
  if (argc >= 3)
    JS_ToUint32(ctx, &nClassId, argv[2]);

  AutoCriticalRoom cRoom;

  bool bAddedRoom = FALSE;

  for (D2DrlgRoomStrc* pRoom = pLevel->pRoom2First; pRoom; pRoom = pRoom->pRoom2Next) {
    bAddedRoom = FALSE;

    if (!pRoom->pRoom1) {
      D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
      bAddedRoom = TRUE;
    }

    for (D2PresetUnitStrc* pUnit = pRoom->pPreset; pUnit; pUnit = pUnit->pPresetNext) {
      // Does it fit?
      if ((nType == NULL || pUnit->dwType == nType) && (nClassId == NULL || pUnit->dwTxtFileNo == nClassId)) {
        // Yes it fits! Return it
        JSPresetUnit* mypUnit = new JSPresetUnit;

        mypUnit->dwPosX = pUnit->dwPosX;
        mypUnit->dwPosY = pUnit->dwPosY;
        mypUnit->dwRoomX = pRoom->dwPosX;
        mypUnit->dwRoomY = pRoom->dwPosY;
        mypUnit->dwType = pUnit->dwType;
        mypUnit->dwId = pUnit->dwTxtFileNo;
        mypUnit->dwLevel = levelId;

        JSValue obj = BuildObject(ctx, presetunit_class_id, FUNCLIST(presetunit_proto_funcs), mypUnit);
        if (JS_IsException(obj)) {
          delete mypUnit;
          THROW_ERROR(ctx, "Failed to create presetunit object");
        }
        return obj;
      }
    }

    if (bAddedRoom) {
      D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
      bAddedRoom = FALSE;
    }
  }

  return JS_FALSE;
}
