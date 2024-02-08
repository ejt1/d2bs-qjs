#include "JSParty.h"

#include "Bindings.h"
#include "D2Helpers.h"  // WaitForGameReady
#include "JSUnit.h"     // JSUnit

JSValue PartyWrap::Instantiate(JSContext* ctx, JSValue new_target, D2RosterUnitStrc* unit) {
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

  PartyWrap* wrap = new PartyWrap(ctx, unit);
  if (!wrap) {
    JS_FreeValue(ctx, obj);
    return JS_ThrowOutOfMemory(ctx);
  }
  JS_SetOpaque(obj, wrap);

  return obj;
}

void PartyWrap::Initialize(JSContext* ctx, JSValue target) {
  JSClassDef def{};
  def.class_name = "Party";
  def.finalizer = [](JSRuntime* /*rt*/, JSValue val) {
    PartyWrap* wrap = static_cast<PartyWrap*>(JS_GetOpaque(val, m_class_id));
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

  JSValue obj = JS_NewCFunction2(ctx, New, "Party", 0, JS_CFUNC_constructor, 0);
  JS_SetConstructor(ctx, obj, proto);

  JS_SetClassProto(ctx, m_class_id, proto);
  JS_SetPropertyStr(ctx, target, "Party", obj);

  // globals
  JS_SetPropertyStr(ctx, target, "getParty", JS_NewCFunction(ctx, GetParty, "getParty", 0));
}

PartyWrap::PartyWrap(JSContext* /*ctx*/, D2RosterUnitStrc* unit) : pPresetUnit(unit) {
}

JSValue PartyWrap::New(JSContext* /*ctx*/, JSValue /*new_target*/, int /*argc*/, JSValue* /*argv*/) {
  // TODO(ejt): empty constructor for compatibility with kolbot
  return JS_UNDEFINED;
}

// properties
JSValue PartyWrap::GetX(JSContext* ctx, JSValue this_val) {
  PartyWrap* wrap = static_cast<PartyWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2RosterUnitStrc* unit = wrap->pPresetUnit;
  return JS_NewUint32(ctx, unit->Xpos);
}

JSValue PartyWrap::GetY(JSContext* ctx, JSValue this_val) {
  PartyWrap* wrap = static_cast<PartyWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2RosterUnitStrc* unit = wrap->pPresetUnit;
  return JS_NewUint32(ctx, unit->Ypos);
}

JSValue PartyWrap::GetArea(JSContext* ctx, JSValue this_val) {
  PartyWrap* wrap = static_cast<PartyWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2RosterUnitStrc* unit = wrap->pPresetUnit;
  return JS_NewUint32(ctx, unit->dwLevelId);
}

JSValue PartyWrap::GetGid(JSContext* ctx, JSValue this_val) {
  PartyWrap* wrap = static_cast<PartyWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2RosterUnitStrc* unit = wrap->pPresetUnit;
  return JS_NewUint32(ctx, unit->dwUnitId);
}

JSValue PartyWrap::GetLife(JSContext* ctx, JSValue this_val) {
  PartyWrap* wrap = static_cast<PartyWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2RosterUnitStrc* unit = wrap->pPresetUnit;
  return JS_NewUint32(ctx, unit->dwPartyLife);
}

JSValue PartyWrap::GetPartyFlag(JSContext* ctx, JSValue this_val) {
  PartyWrap* wrap = static_cast<PartyWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2RosterUnitStrc* unit = wrap->pPresetUnit;
  return JS_NewUint32(ctx, unit->dwPartyFlags);
}

JSValue PartyWrap::GetPartyId(JSContext* ctx, JSValue this_val) {
  PartyWrap* wrap = static_cast<PartyWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2RosterUnitStrc* unit = wrap->pPresetUnit;
  return JS_NewUint32(ctx, unit->wPartyId);
}

JSValue PartyWrap::GetName(JSContext* ctx, JSValue this_val) {
  PartyWrap* wrap = static_cast<PartyWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2RosterUnitStrc* unit = wrap->pPresetUnit;
  return JS_NewString(ctx, unit->szName);
}

JSValue PartyWrap::GetClassId(JSContext* ctx, JSValue this_val) {
  PartyWrap* wrap = static_cast<PartyWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2RosterUnitStrc* unit = wrap->pPresetUnit;
  return JS_NewUint32(ctx, unit->dwClassId);
}

JSValue PartyWrap::GetLevel(JSContext* ctx, JSValue this_val) {
  PartyWrap* wrap = static_cast<PartyWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2RosterUnitStrc* unit = wrap->pPresetUnit;
  return JS_NewUint32(ctx, unit->wLevel);
}

// functions
JSValue PartyWrap::GetNext(JSContext* ctx, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  PartyWrap* wrap = static_cast<PartyWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2RosterUnitStrc* unit = wrap->pPresetUnit;
  if (!unit || !unit->pNext) {
    return JS_FALSE;
  }

  wrap->pPresetUnit = unit->pNext;
  return JS_DupValue(ctx, this_val);
}

// globals
JSValue PartyWrap::GetParty(JSContext* ctx, JSValue /*this_val*/, int argc, JSValue* argv) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  D2RosterUnitStrc* pUnit = *D2CLIENT_PlayerUnitList;
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
      JSUnit* lpUnit = (JSUnit*)JS_GetOpaque3(argv[0]);

      if (!lpUnit)
        return JS_UNDEFINED;

      inUnit = D2UnitStrc::FindUnit(lpUnit->dwUnitId, lpUnit->dwType);
      if (!inUnit)
        THROW_ERROR(ctx, "Unable to get Unit");

      nPlayerId = inUnit->dwUnitId;
    }

    if (!nPlayerName && !nPlayerId)
      return JS_UNDEFINED;

    bool bFound = false;

    for (D2RosterUnitStrc* pScan = pUnit; pScan; pScan = pScan->pNext) {
      if (nPlayerId && pScan->dwUnitId == nPlayerId) {
        bFound = true;
        pUnit = pScan;
        break;
      }
      if (nPlayerName && _stricmp(pScan->szName, nPlayerName) == 0) {
        bFound = true;
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

  return PartyWrap::Instantiate(ctx, JS_UNDEFINED, pUnit);
}

D2BS_BINDING_INTERNAL(PartyWrap, PartyWrap::Initialize)