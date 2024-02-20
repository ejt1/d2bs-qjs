#include "JSPresetUnit.h"

#include "Bindings.h"
#include "CriticalSections.h"  // AutoCriticalRoom
#include "D2Helpers.h"         // WaitForGameReady

JSObject* PresetUnitWrap::Instantiate(JSContext* ctx, D2PresetUnitStrc* preset, D2DrlgRoomStrc* room, uint32_t level) {
  JS::RootedObject global(ctx, JS::CurrentGlobalOrNull(ctx));
  JS::RootedValue constructor_val(ctx);
  if (!JS_GetProperty(ctx, global, "PresetUnit", &constructor_val)) {
    JS_ReportErrorASCII(ctx, "Could not find constructor object for PresetUnit");
    return nullptr;
  }
  if (!constructor_val.isObject()) {
    JS_ReportErrorASCII(ctx, "PresetUnit is not a constructor");
    return nullptr;
  }
  JS::RootedObject constructor(ctx, &constructor_val.toObject());

  JS::RootedObject obj(ctx, JS_New(ctx, constructor, JS::HandleValueArray::empty()));
  if (!obj) {
    JS_ReportErrorASCII(ctx, "Calling PresetUnit constructor failed");
    return nullptr;
  }
  PresetUnitWrap* wrap = new PresetUnitWrap(ctx, obj, preset, room, level);
  if (!wrap) {
    JS_ReportOutOfMemory(ctx);
    return nullptr;
  }
  return obj;
}

void PresetUnitWrap::Initialize(JSContext* ctx, JS::HandleObject target) {
  JS::RootedObject proto(ctx, JS_InitClass(ctx, target, nullptr, &m_class, trampoline<New>, 0, m_props, nullptr, nullptr, nullptr));
  if (!proto) {
    return;
  }

  // globals
  JS_DefineFunction(ctx, target, "getPresetUnit", trampoline<GetPresetUnit>, 0, JSPROP_ENUMERATE);
  JS_DefineFunction(ctx, target, "getPresetUnits", trampoline<GetPresetUnits>, 0, JSPROP_ENUMERATE);
}

PresetUnitWrap::PresetUnitWrap(JSContext* ctx, JS::HandleObject obj, D2PresetUnitStrc* preset, D2DrlgRoomStrc* room, uint32_t level)
    : BaseObject(ctx, obj),
      dwPosX(preset->dwPosX),
      dwPosY(preset->dwPosY),
      dwRoomX(room->dwPosX),
      dwRoomY(room->dwPosY),
      dwType(preset->dwType),
      dwId(preset->dwTxtFileNo),
      dwLevel(level) {
}

void PresetUnitWrap::finalize(JSFreeOp* /*fop*/, JSObject* obj) {
  BaseObject* wrap = BaseObject::FromJSObject(obj);
  if (wrap) {
    delete wrap;
  }
}

bool PresetUnitWrap::New(JSContext* ctx, JS::CallArgs& args) {
  JS::RootedObject newObject(ctx, JS_NewObjectForConstructor(ctx, &m_class, args));
  if (!newObject) {
    THROW_ERROR(ctx, "failed to instantiate preset");
  }
  // BUG(ejt): ask ejt
  JS_DefineProperties(ctx, newObject, m_props);

  args.rval().setObject(*newObject);
  return true;
}

// properties
bool PresetUnitWrap::GetType(JSContext* ctx, JS::CallArgs& args) {
  PresetUnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  args.rval().setInt32(wrap->dwType);
  return true;
}

bool PresetUnitWrap::GetRoomX(JSContext* ctx, JS::CallArgs& args) {
  PresetUnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  args.rval().setInt32(wrap->dwRoomX);
  return true;
}

bool PresetUnitWrap::GetRoomY(JSContext* ctx, JS::CallArgs& args) {
  PresetUnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  args.rval().setInt32(wrap->dwRoomY);
  return true;
}

bool PresetUnitWrap::GetX(JSContext* ctx, JS::CallArgs& args) {
  PresetUnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  args.rval().setInt32(wrap->dwPosX);
  return true;
}

bool PresetUnitWrap::GetY(JSContext* ctx, JS::CallArgs& args) {
  PresetUnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  args.rval().setInt32(wrap->dwPosY);
  return true;
}

bool PresetUnitWrap::GetId(JSContext* ctx, JS::CallArgs& args) {
  PresetUnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  args.rval().setInt32(wrap->dwId);
  return true;
}

bool PresetUnitWrap::GetLevel(JSContext* ctx, JS::CallArgs& args) {
  PresetUnitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  args.rval().setInt32(wrap->dwLevel);
  return true;
}

bool PresetUnitWrap::GetPresetUnit(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  if (args.length() < 1) {
    args.rval().setBoolean(false);
    return true;
  }

  uint32_t levelId;
  JS::ToUint32(ctx, args[0], &levelId);

  D2DrlgLevelStrc* pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(levelId);
  if (!pLevel)
    THROW_ERROR(ctx, "getPresetUnits failed, couldn't access the level!");

  uint32_t nClassId = NULL;
  uint32_t nType = NULL;

  if (args.length() >= 2)
    JS::ToUint32(ctx, args[1], &nType);
  if (args.length() >= 3)
    JS::ToUint32(ctx, args[2], &nClassId);

  AutoCriticalRoom cRoom;

  bool bAddedRoom = false;

  for (D2DrlgRoomStrc* pRoom = pLevel->pRoom2First; pRoom; pRoom = pRoom->pRoom2Next) {
    bAddedRoom = false;

    if (!pRoom->pRoom1) {
      D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
      bAddedRoom = true;
    }

    for (D2PresetUnitStrc* pUnit = pRoom->pPreset; pUnit; pUnit = pUnit->pPresetNext) {
      // Does it fit?
      if ((nType == NULL || pUnit->dwType == nType) && (nClassId == NULL || pUnit->dwTxtFileNo == nClassId)) {
        // Yes it fits! Return it
        JS::RootedObject obj(ctx, PresetUnitWrap::Instantiate(ctx, pUnit, pRoom, levelId));
        if (!obj) {
          THROW_ERROR(ctx, "Failed to create presetunit object");
        }
        args.rval().setObject(*obj);
        return true;
      }
    }

    if (bAddedRoom) {
      D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
      bAddedRoom = false;
    }
  }

  args.rval().setBoolean(false);
  return true;
}

bool PresetUnitWrap::GetPresetUnits(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  if (args.length() < 1) {
    args.rval().setBoolean(false);
    return true;
  }

  uint32_t levelId;
  JS::ToUint32(ctx, args[0], &levelId);

  D2DrlgLevelStrc* pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(levelId);
  if (!pLevel) {
    THROW_ERROR(ctx, "getPresetUnits failed, couldn't access the level!");
  }

  uint32_t nClassId = NULL;
  uint32_t nType = NULL;

  if (args.length() >= 2)
    JS::ToUint32(ctx, args[1], &nType);
  if (args.length() >= 3)
    JS::ToUint32(ctx, args[2], &nClassId);

  AutoCriticalRoom cRoom;

  bool bAddedRoom = FALSE;
  uint32_t dwArrayCount = NULL;

  JS::RootedObject pReturnArray(ctx, JS::NewArrayObject(ctx, 0));
  for (D2DrlgRoomStrc* pRoom = pLevel->pRoom2First; pRoom; pRoom = pRoom->pRoom2Next) {
    bAddedRoom = FALSE;

    if (!pRoom->pPreset) {
      D2COMMON_AddRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
      bAddedRoom = TRUE;
    }

    for (D2PresetUnitStrc* pUnit = pRoom->pPreset; pUnit; pUnit = pUnit->pPresetNext) {
      // Does it fit?
      if ((nType == NULL || pUnit->dwType == nType) && (nClassId == NULL || pUnit->dwTxtFileNo == nClassId)) {
        JS::RootedObject obj(ctx, PresetUnitWrap::Instantiate(ctx, pUnit, pRoom, levelId));
        if (!obj) {
          THROW_ERROR(ctx, "Failed to build object?");
        }
        JS_SetElement(ctx, pReturnArray, dwArrayCount, obj);
        dwArrayCount++;
      }
    }

    if (bAddedRoom) {
      D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
      bAddedRoom = FALSE;
    }
  }
  args.rval().setObject(*pReturnArray);
  return true;
}

D2BS_BINDING_INTERNAL(PresetUnitWrap, PresetUnitWrap::Initialize)
