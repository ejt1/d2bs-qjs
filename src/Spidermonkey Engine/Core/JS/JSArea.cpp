#include "JSArea.h"
#include "JSExits.h"
#include "CriticalSections.h"
#include "MapHeader.h"
#include "JSGlobalClasses.h"

#include "Game/D2DataTbls.h"

EMPTY_CTOR(area)

CLASS_FINALIZER(area) {
  JSArea* pArea = (JSArea*)JS_GetOpaque3(val);

  if (pArea) {
    JS_SetOpaque(val, NULL);
    JS_FreeValueRT(rt, pArea->ExitArray);
    delete pArea;
  }
}

JSAPI_PROP(area_getProperty) {
  JSArea* pArea = (JSArea*)JS_GetOpaque3(this_val);

  if (!pArea)
    return JS_EXCEPTION;

  D2DrlgLevelStrc* pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(pArea->AreaId);
  if (!pLevel)
    return JS_EXCEPTION;

  switch (magic) {
    case AUNIT_EXITS: {
      if (pArea->ExitArray == JS_UNDEFINED) {
        pArea->ExitArray = JS_NewArray(ctx);

        ActMap* map = ActMap::GetMap(pLevel);

        ExitArray exits;
        map->GetExits(exits);
        map->CleanUp();
        int count = exits.size();
        for (int i = 0; i < count; i++) {
          JSExit* exit = new JSExit;
          exit->id = exits[i].Target;
          exit->x = exits[i].Position.first;
          exit->y = exits[i].Position.second;
          exit->type = exits[i].Type;
          exit->tileid = exits[i].TileId;
          exit->level = pArea->AreaId;

          JSValue pExit = BuildObject(ctx, exit_class_id, FUNCLIST(exit_proto_funcs), exit);
          if (JS_IsException(pExit)) {
            delete exit;
            THROW_ERROR(ctx, "Failed to create exit object!");
          }
          JS_SetPropertyUint32(ctx, pArea->ExitArray, i, pExit);
        }
      }
      return JS_DupValue(ctx, pArea->ExitArray);
    } break;
    case AUNIT_NAME: {
      D2LevelsTxt* pTxt = D2COMMON_GetLevelText(pArea->AreaId);
      if (pTxt) {
        return JS_NewString(ctx, pTxt->szName);
      }
    } break;
    case AUNIT_X:
      return JS_NewInt32(ctx, pLevel->dwPosX);
    case AUNIT_Y:
      return JS_NewInt32(ctx, pLevel->dwPosY);
    case AUNIT_XSIZE:
      return JS_NewInt32(ctx, pLevel->dwSizeX);
    case AUNIT_YSIZE:
      return JS_NewInt32(ctx, pLevel->dwSizeY);
    case AUNIT_ID:
      return JS_NewInt32(ctx, pLevel->dwLevelNo);
  }

  return JS_UNDEFINED;
}

JSAPI_FUNC(my_getArea) {
  if (!WaitForGameReady())
    THROW_ERROR(ctx, "Get Area: Game not ready");

  int32_t nArea = GetPlayerArea();

  if (argc == 1) {
    if (JS_ToInt32(ctx, &nArea, argv[0])) {
      return JS_EXCEPTION;
    }
  }

  if (nArea < 0) {
    THROW_ERROR(ctx, "Invalid parameter passed to getArea!");
  }

  D2DrlgLevelStrc* pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(nArea);

  if (!pLevel) {
    return JS_FALSE;
  }

  JSArea* pArea = new JSArea;
  if (!pArea) {
    return JS_FALSE;
  }

  pArea->AreaId = nArea;
  pArea->ExitArray = JS_UNDEFINED;

  JSValue unit = BuildObject(ctx, area_class_id, FUNCLIST(area_proto_funcs), pArea);
  if (JS_IsException(unit)) {
    delete pArea;
    pArea = NULL;
    THROW_ERROR(ctx, "Failed to build area unit!");
  }
  return unit;
}
