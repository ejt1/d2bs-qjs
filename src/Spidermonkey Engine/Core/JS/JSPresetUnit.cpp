#include "JSPresetUnit.h"

#include "Bindings.h"
#include "CriticalSections.h"  // AutoCriticalRoom
#include "D2Helpers.h"         // WaitForGameReady

JSValue PresetUnitWrap::Instantiate(JSContext* ctx, JSValue new_target, D2PresetUnitStrc* preset, D2DrlgRoomStrc* room, uint32_t level) {
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

  PresetUnitWrap* wrap = new PresetUnitWrap(ctx, preset, room, level);
  if (!wrap) {
    JS_FreeValue(ctx, obj);
    return JS_ThrowOutOfMemory(ctx);
  }
  JS_SetOpaque(obj, wrap);

  return obj;
}

void PresetUnitWrap::Initialize(JSContext* ctx, JSValue target) {
  JSClassDef def{};
  def.class_name = "PresetUnit";
  def.finalizer = [](JSRuntime* /*rt*/, JSValue val) {
    PresetUnitWrap* wrap = static_cast<PresetUnitWrap*>(JS_GetOpaque(val, m_class_id));
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

  JSValue obj = JS_NewCFunction2(ctx, New, "PresetUnit", 0, JS_CFUNC_constructor, 0);
  JS_SetConstructor(ctx, obj, proto);

  JS_SetClassProto(ctx, m_class_id, proto);
  JS_SetPropertyStr(ctx, target, "PresetUnit", obj);

  // globals
  JS_SetPropertyStr(ctx, target, "getPresetUnit", JS_NewCFunction(ctx, GetPresetUnit, "getPresetUnit", 0));
  JS_SetPropertyStr(ctx, target, "getPresetUnits", JS_NewCFunction(ctx, GetPresetUnits, "getPresetUnits", 0));
}

PresetUnitWrap::PresetUnitWrap(JSContext* /*ctx*/, D2PresetUnitStrc* preset, D2DrlgRoomStrc* room, uint32_t level)
    : dwPosX(preset->dwPosX), dwPosY(preset->dwPosY), dwRoomX(room->dwPosX), dwRoomY(room->dwPosY), dwType(preset->dwType), dwId(preset->dwTxtFileNo), dwLevel(level) {
}

JSValue PresetUnitWrap::New(JSContext* ctx, JSValue new_target, int argc, JSValue* argv) {
  // TODO(ejt): empty constructor for compatibility with kolbot
  return JS_UNDEFINED;
}

// properties
JSValue PresetUnitWrap::GetType(JSContext* ctx, JSValue this_val) {
  PresetUnitWrap* wrap = static_cast<PresetUnitWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  return JS_NewUint32(ctx, wrap->dwType);
}

JSValue PresetUnitWrap::GetRoomX(JSContext* ctx, JSValue this_val) {
  PresetUnitWrap* wrap = static_cast<PresetUnitWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  return JS_NewUint32(ctx, wrap->dwRoomX);
}

JSValue PresetUnitWrap::GetRoomY(JSContext* ctx, JSValue this_val) {
  PresetUnitWrap* wrap = static_cast<PresetUnitWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  return JS_NewUint32(ctx, wrap->dwRoomY);
}

JSValue PresetUnitWrap::GetX(JSContext* ctx, JSValue this_val) {
  PresetUnitWrap* wrap = static_cast<PresetUnitWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  return JS_NewUint32(ctx, wrap->dwPosX);
}

JSValue PresetUnitWrap::GetY(JSContext* ctx, JSValue this_val) {
  PresetUnitWrap* wrap = static_cast<PresetUnitWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  return JS_NewUint32(ctx, wrap->dwPosY);
}

JSValue PresetUnitWrap::GetId(JSContext* ctx, JSValue this_val) {
  PresetUnitWrap* wrap = static_cast<PresetUnitWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  return JS_NewUint32(ctx, wrap->dwId);
}

JSValue PresetUnitWrap::GetLevel(JSContext* ctx, JSValue this_val) {
  PresetUnitWrap* wrap = static_cast<PresetUnitWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  return JS_NewUint32(ctx, wrap->dwLevel);
}

JSValue PresetUnitWrap::GetPresetUnit(JSContext* ctx, JSValue /*this_val*/, int argc, JSValue* argv) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  if (argc < 1) {
    return JS_FALSE;
  }

  uint32_t levelId;
  JS_ToUint32(ctx, &levelId, argv[0]);

  D2DrlgLevelStrc* pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(levelId);
  if (!pLevel)
    THROW_ERROR(ctx, "getPresetUnits failed, couldn't access the level!");

  uint32_t nClassId = NULL;
  uint32_t nType = NULL;

  if (argc >= 2)
    JS_ToUint32(ctx, &nType, argv[1]);
  if (argc >= 3)
    JS_ToUint32(ctx, &nClassId, argv[2]);

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
        JSValue obj = PresetUnitWrap::Instantiate(ctx, JS_UNDEFINED, pUnit, pRoom, levelId);
        if (JS_IsException(obj)) {
          THROW_ERROR(ctx, "Failed to create presetunit object");
        }
        return obj;
      }
    }

    if (bAddedRoom) {
      D2COMMON_RemoveRoomData(D2CLIENT_GetPlayerUnit()->pAct, pLevel->dwLevelNo, pRoom->dwPosX, pRoom->dwPosY, D2CLIENT_GetPlayerUnit()->pPath->pRoom1);
      bAddedRoom = false;
    }
  }

  return JS_FALSE;
}

JSValue PresetUnitWrap::GetPresetUnits(JSContext* ctx, JSValue /*this_val*/, int argc, JSValue* argv) {
  if (!WaitForGameReady())
    THROW_WARNING(ctx, "Game not ready");

  if (argc < 1) {
    return JS_FALSE;
  }

  uint32_t levelId;
  JS_ToUint32(ctx, &levelId, argv[0]);

  D2DrlgLevelStrc* pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(levelId);
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
  uint32_t dwArrayCount = NULL;

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
        JSValue obj = PresetUnitWrap::Instantiate(ctx, JS_UNDEFINED, pUnit, pRoom, levelId);
        if (JS_IsException(obj)) {
          JS_FreeValue(ctx, pReturnArray);
          THROW_ERROR(ctx, "Failed to build object?");
        }

        JS_SetPropertyUint32(ctx, pReturnArray, dwArrayCount, obj);

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

D2BS_BINDING_INTERNAL(PresetUnitWrap, PresetUnitWrap::Initialize)