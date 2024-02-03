#include "JSArea.h"
#include "D2Ptrs.h"
#include "JSExits.h"
#include "CriticalSections.h"
#include "MapHeader.h"

EMPTY_CTOR(area)

CLASS_FINALIZER(area) {
  myArea* pArea = (myArea*)JS_GetPrivate(val);

  if (pArea) {
    JS_SetPrivate(val, NULL);
    delete pArea;
  }
}

JSAPI_PROP(area_getProperty) {
  myArea* pArea = (myArea*)JS_GetPrivate(ctx, this_val);

  if (!pArea)
    return JS_FALSE;

  Level* pLevel = GetLevel(pArea->AreaId);
  if (!pLevel)
    return JS_FALSE;

  switch (magic) {
    case AUNIT_EXITS: {
      if (pArea->ExitArray == NULL) {
        pArea->ExitArray = JS_NewArray(ctx);

        ActMap* map = ActMap::GetMap(pLevel);

        ExitArray exits;
        map->GetExits(exits);
        map->CleanUp();
        int count = exits.size();
        for (int i = 0; i < count; i++) {
          myExit* exit = new myExit;
          exit->id = exits[i].Target;
          exit->x = exits[i].Position.first;
          exit->y = exits[i].Position.second;
          exit->type = exits[i].Type;
          exit->tileid = exits[i].TileId;
          exit->level = pArea->AreaId;

          JSValue pExit = BuildObject(ctx, exit_class_id, NULL, 0, exit_props, _countof(exit_props), exit);
          if (JS_IsException(pExit)) {
            delete exit;
            THROW_ERROR(ctx, "Failed to create exit object!");
          }
          JS_SetPropertyUint32(ctx, pArea->ExitArray, i, pExit);
        }
      }
      return pArea->ExitArray;
    } break;
    case AUNIT_NAME: {
      LevelTxt* pTxt = D2COMMON_GetLevelText(pArea->AreaId);
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

  return JS_TRUE;
}

JSAPI_FUNC(my_getArea) {
  // JS_SET_RVAL(cx, vp, JSVAL_VOID);

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

  Level* pLevel = GetLevel(nArea);

  if (!pLevel) {
    return JS_FALSE;
  }

  myArea* pArea = new myArea;
  if (!pArea) {
    return JS_FALSE;
  }

  pArea->AreaId = nArea;
  pArea->ExitArray = NULL;

  JSValue unit = BuildObject(ctx, area_class_id, NULL, 0, area_props, _countof(area_props), pArea);
  if (JS_IsException(unit)) {
    delete pArea;
    pArea = NULL;
    THROW_ERROR(ctx, "Failed to build area unit!");
  }
  return unit;
}
