#include "JSParty.h"
#include "D2Helpers.h"
#include "D2Ptrs.h"
#include "JSUnit.h"
#include "JSGlobalClasses.h"

#include "Game/D2Roster.h"

EMPTY_CTOR(party)

JSAPI_PROP(party_getProperty) {
  D2RosterUnitStrc* pUnit = (D2RosterUnitStrc*)JS_GetOpaque3(this_val);

  if (!pUnit)
    return JS_UNDEFINED;

  switch (magic) {
    case PARTY_NAME:
      return JS_NewString(ctx, pUnit->szName);
      break;
    case PARTY_X:
      return JS_NewUint32(ctx, pUnit->Xpos);
      break;
    case PARTY_Y:
      return JS_NewUint32(ctx, pUnit->Ypos);
      break;
    case PARTY_AREA:
      return JS_NewUint32(ctx, pUnit->dwLevelId);
      break;
    case PARTY_GID:
      return JS_NewUint32(ctx, pUnit->dwUnitId);
      break;
    case PARTY_LIFE:
      return JS_NewUint32(ctx, pUnit->dwPartyLife);
      break;
    case PARTY_CLASSID:
      return JS_NewUint32(ctx, pUnit->dwClassId);
      break;
    case PARTY_LEVEL:
      return JS_NewUint32(ctx, pUnit->wLevel);
      break;
    case PARTY_FLAG:
      return JS_NewUint32(ctx, pUnit->dwPartyFlags);
      break;
    case PARTY_ID:
      return JS_NewUint32(ctx, pUnit->wPartyId);
      break;
    default:
      break;
  }

  return JS_UNDEFINED;
}

JSAPI_FUNC(party_getNext) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  D2RosterUnitStrc* pUnit = (D2RosterUnitStrc*)JS_GetOpaque3(this_val);

  if (!pUnit) {
    return JS_FALSE;
  }

  pUnit = pUnit->pNext;

  if (pUnit) {
    // BUG(ejt): possible value leak
    JS_SetOpaque(this_val, pUnit);
    return JS_DupValue(ctx, this_val);
  }

  return JS_FALSE;
}

JSAPI_FUNC(my_getParty) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  D2RosterUnitStrc* pUnit = *p_D2CLIENT_PlayerUnitList;

  if (!pUnit)
    return JS_UNDEFINED;

  if (argc == 1) {
    D2UnitStrc* inUnit = NULL;
    const char* nPlayerName = nullptr;
    uint32_t nPlayerId = NULL;

    if (JS_IsString(argv[0])) {
      nPlayerName = JS_ToCString(ctx, argv[0]);
    } else if (JS_IsNumber(argv[0]) && JS_ToUint32(ctx, &nPlayerId, argv[0])) {
      THROW_ERROR(ctx, "Unable to get ID");
    } else if (JS_IsObject(argv[0])) {
      myUnit* lpUnit = (myUnit*)JS_GetOpaque3(argv[0]);

      if (!lpUnit)
        return JS_UNDEFINED;

      inUnit = D2CLIENT_FindUnit(lpUnit->dwUnitId, lpUnit->dwType);
      if (!inUnit)
        THROW_ERROR(ctx, "Unable to get Unit");

      nPlayerId = inUnit->dwUnitId;
    }

    if (!nPlayerName && !nPlayerId)
      return JS_UNDEFINED;

    BOOL bFound = FALSE;

    for (D2RosterUnitStrc* pScan = pUnit; pScan; pScan = pScan->pNext) {
      if (nPlayerId && pScan->dwUnitId == nPlayerId) {
        bFound = TRUE;
        pUnit = pScan;
        break;
      }
      if (nPlayerName && _stricmp(pScan->szName, nPlayerName) == 0) {
        bFound = TRUE;
        pUnit = pScan;
        break;
      }
    }

    if (nPlayerName) {
      JS_FreeCString(ctx, nPlayerName);
    }

    if (!bFound)
      return JS_UNDEFINED;
  }

  return BuildObject(ctx, party_class_id, FUNCLIST(party_proto_funcs), pUnit);
}
