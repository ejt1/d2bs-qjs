#include "JSParty.h"

#include "Bindings.h"
#include "D2Helpers.h"  // WaitForGameReady
#include "JSUnit.h"     // JSUnit

JSObject* PartyWrap::Instantiate(JSContext* ctx, D2RosterUnitStrc* unit) {
  JS::RootedObject global(ctx, JS::CurrentGlobalOrNull(ctx));
  JS::RootedValue constructor_val(ctx);
  if (!JS_GetProperty(ctx, global, "Party", &constructor_val)) {
    JS_ReportErrorASCII(ctx, "Could not find constructor object for Party");
    return nullptr;
  }
  if (!constructor_val.isObject()) {
    JS_ReportErrorASCII(ctx, "Party is not a constructor");
    return nullptr;
  }
  JS::RootedObject constructor(ctx, &constructor_val.toObject());

  JS::RootedObject obj(ctx, JS_New(ctx, constructor, JS::HandleValueArray::empty()));
  if (!obj) {
    JS_ReportErrorASCII(ctx, "Calling Party constructor failed");
    return nullptr;
  }
  PartyWrap* wrap = new PartyWrap(ctx, obj, unit);
  if (!wrap) {
    JS_ReportOutOfMemory(ctx);
    return nullptr;
  }
  return obj;
}

void PartyWrap::Initialize(JSContext* ctx, JS::HandleObject target) {
  static JSPropertySpec props[] = {
      JS_PSG("x", GetX, JSPROP_ENUMERATE),
      JS_PSG("y", GetY, JSPROP_ENUMERATE),
      JS_PSG("area", GetArea, JSPROP_ENUMERATE),
      JS_PSG("gid", GetGid, JSPROP_ENUMERATE),
      JS_PSG("life", GetLife, JSPROP_ENUMERATE),
      JS_PSG("partyflag", GetPartyFlag, JSPROP_ENUMERATE),
      JS_PSG("partyid", GetPartyId, JSPROP_ENUMERATE),
      JS_PSG("name", GetName, JSPROP_ENUMERATE),
      JS_PSG("classid", GetClassId, JSPROP_ENUMERATE),
      JS_PSG("level", GetLevel, JSPROP_ENUMERATE),
      JS_PS_END,
  };

  static JSFunctionSpec methods[] = {
      JS_FN("getNext", GetNext, 0, JSPROP_ENUMERATE),
      JS_FS_END,
  };

  JS::RootedObject proto(ctx, JS_InitClass(ctx, target, nullptr, &m_class, New, 0, props, methods, nullptr, nullptr));
  if (!proto) {
    return;
  }

  // globals
  JS_DefineFunction(ctx, target, "getParty", GetParty, 0, JSPROP_ENUMERATE);
}

D2RosterUnitStrc* PartyWrap::GetData() {
  return pPresetUnit;
}

PartyWrap::PartyWrap(JSContext* ctx, JS::HandleObject obj, D2RosterUnitStrc* unit) : BaseObject(ctx, obj), pPresetUnit(unit) {
}

void PartyWrap::finalize(JSFreeOp* fop, JSObject* obj) {
  BaseObject* wrap = BaseObject::FromJSObject(obj);
  if (wrap) {
    delete wrap;
  }
}

bool PartyWrap::New(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  JS::RootedObject newObject(ctx, JS_NewObjectForConstructor(ctx, &m_class, args));
  if (!newObject) {
    THROW_ERROR(ctx, "failed to instantiate party");
  }
  args.rval().setObject(*newObject);
  return true;
}

// properties
bool PartyWrap::GetX(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  PartyWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  D2RosterUnitStrc* unit = wrap->pPresetUnit;
  args.rval().setInt32(unit->Xpos);
  return true;
}

bool PartyWrap::GetY(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  PartyWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  D2RosterUnitStrc* unit = wrap->pPresetUnit;
  args.rval().setInt32(unit->Ypos);
  return true;
}

bool PartyWrap::GetArea(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  PartyWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  D2RosterUnitStrc* unit = wrap->pPresetUnit;
  args.rval().setInt32(unit->dwLevelId);
  return true;
}

bool PartyWrap::GetGid(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  PartyWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  D2RosterUnitStrc* unit = wrap->pPresetUnit;
  args.rval().setInt32(unit->dwUnitId);
  return true;
}

bool PartyWrap::GetLife(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  PartyWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  D2RosterUnitStrc* unit = wrap->pPresetUnit;
  args.rval().setInt32(unit->dwPartyLife);
  return true;
}

bool PartyWrap::GetPartyFlag(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  PartyWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  D2RosterUnitStrc* unit = wrap->pPresetUnit;
  args.rval().setInt32(unit->dwPartyFlags);
  return true;
}

bool PartyWrap::GetPartyId(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  PartyWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  D2RosterUnitStrc* unit = wrap->pPresetUnit;
  args.rval().setInt32(unit->wPartyId);
  return true;
}

bool PartyWrap::GetName(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  PartyWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  D2RosterUnitStrc* unit = wrap->pPresetUnit;
  args.rval().setString(JS_NewStringCopyZ(ctx, unit->szName));
  return true;
}

bool PartyWrap::GetClassId(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  PartyWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  D2RosterUnitStrc* unit = wrap->pPresetUnit;
  args.rval().setInt32(unit->dwClassId);
  return true;
}

bool PartyWrap::GetLevel(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  PartyWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  D2RosterUnitStrc* unit = wrap->pPresetUnit;
  args.rval().setInt32(unit->wLevel);
  return true;
}

// functions
bool PartyWrap::GetNext(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  PartyWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  D2RosterUnitStrc* unit = wrap->pPresetUnit;
  if (!unit || !unit->pNext) {
    args.rval().setBoolean(false);
    return true;
  }

  wrap->pPresetUnit = unit->pNext;
  args.rval().setObjectOrNull(args.thisv().toObjectOrNull());
  return true;
}

// globals
bool PartyWrap::GetParty(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  D2RosterUnitStrc* pRosterUnit = *D2CLIENT_PlayerUnitList;
  if (!pRosterUnit) {
    args.rval().setUndefined();
    return true;
  }

  if (argc == 1) {
    char* nPlayerName = nullptr;
    uint32_t nPlayerId = NULL;

    if (args.get(0).toString()) {
      nPlayerName = JS_EncodeString(ctx, args[0].toString());
    } else if (args.get(0).isNumber() && !JS::ToUint32(ctx, args[0], &nPlayerId)) {
      THROW_ERROR(ctx, "Unable to get ID");
    } else if (args.get(0).isObject()) {
      UNWRAP_UNIT_OR_ERROR(ctx, 0, args[0]);
      nPlayerId = pUnit->dwUnitId;
    }

    if (!nPlayerName && !nPlayerId) {
      args.rval().setUndefined();
      return true;
    }

    bool bFound = false;

    for (D2RosterUnitStrc* pScan = pRosterUnit; pScan; pScan = pScan->pNext) {
      if (nPlayerId && pScan->dwUnitId == nPlayerId) {
        bFound = true;
        pRosterUnit = pScan;
        break;
      }
      if (nPlayerName && _stricmp(pScan->szName, nPlayerName) == 0) {
        bFound = true;
        pRosterUnit = pScan;
        break;
      }
    }

    if (nPlayerName) {
      JS_free(ctx, nPlayerName);
    }

    if (!bFound) {
      args.rval().setUndefined();
      return true;
    }
  }

  JS::RootedObject obj(ctx, PartyWrap::Instantiate(ctx, pRosterUnit));
  if (!obj) {
    THROW_ERROR(ctx, "failed to instantiate party");
  }
  args.rval().setObject(*obj);
  return true;
}

D2BS_BINDING_INTERNAL(PartyWrap, PartyWrap::Initialize)
