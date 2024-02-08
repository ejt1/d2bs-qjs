#include "JSArea.h"

#include "Bindings.h"
#include "JSExits.h" // JSExit
#include "JSGlobalClasses.h" // exit_class_id, exit_proto_funcs
#include "MapHeader.h" // ExitArray

JSValue AreaWrap::Instantiate(JSContext* ctx, JSValue new_target, uint32_t dwAreaId) {
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

  AreaWrap* wrap = new AreaWrap(ctx, dwAreaId);
  if (!wrap) {
    JS_FreeValue(ctx, obj);
    return JS_ThrowOutOfMemory(ctx);
  }
  JS_SetOpaque(obj, wrap);

  return obj;
}

void AreaWrap::Initialize(JSContext* ctx, JSValue target) {
  JSClassDef def{};
  def.class_name = "Area";
  def.finalizer = [](JSRuntime* rt, JSValue val) {
    AreaWrap* wrap = static_cast<AreaWrap*>(JS_GetOpaque(val, m_class_id));
    if (wrap) {
      JS_FreeValueRT(rt, wrap->arrExitArray);
      delete wrap;
    }
  };

  if (m_class_id == 0) {
    JS_NewClassID(&m_class_id);
  }
  JS_NewClass(JS_GetRuntime(ctx), m_class_id, &def);

  JSValue proto = JS_NewObject(ctx);
  JS_SetPropertyFunctionList(ctx, proto, m_proto_funcs, _countof(m_proto_funcs));

  JSValue obj = JS_NewObjectProtoClass(ctx, proto, m_class_id);
  JS_SetClassProto(ctx, m_class_id, proto);
  JS_SetPropertyStr(ctx, target, "Area", obj);

  // globals
  JS_SetPropertyStr(ctx, target, "getArea", JS_NewCFunction(ctx, GetArea, "getArea", 0));
}

AreaWrap::AreaWrap(JSContext* /*ctx*/, uint32_t dwAreaId) : dwAreaId(dwAreaId), dwExits(0), arrExitArray(JS_UNDEFINED) {
}

JSValue AreaWrap::GetId(JSContext* ctx, JSValue this_val) {
  AreaWrap* wrap = static_cast<AreaWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2DrlgLevelStrc* pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(wrap->dwAreaId);
  if (!pLevel)
    return JS_EXCEPTION;

  return JS_NewInt32(ctx, pLevel->dwLevelNo);
}

JSValue AreaWrap::GetName(JSContext* ctx, JSValue this_val) {
  AreaWrap* wrap = static_cast<AreaWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2LevelsTxt* pTxt = D2COMMON_GetLevelText(wrap->dwAreaId);
  if (pTxt) {
    return JS_NewString(ctx, pTxt->szName);
  }
  return JS_UNDEFINED;
}

JSValue AreaWrap::GetExits(JSContext* ctx, JSValue this_val) {
  AreaWrap* wrap = static_cast<AreaWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2DrlgLevelStrc* pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(wrap->dwAreaId);
  if (!pLevel)
    return JS_EXCEPTION;

  if (wrap->arrExitArray == JS_UNDEFINED) {
    wrap->arrExitArray = JS_NewArray(ctx);

    ActMap* map = ActMap::GetMap(pLevel);

    ExitArray exits;
    map->GetExits(exits);
    map->CleanUp();
    int count = exits.size();
    for (int i = 0; i < count; i++) {
      JSValue obj = ExitWrap::Instantiate(ctx, JS_UNDEFINED, &exits[i], wrap->dwAreaId);
      if (JS_IsException(obj)) {
        THROW_ERROR(ctx, "Failed to create exit object!");
      }
      JS_SetPropertyUint32(ctx, wrap->arrExitArray, i, obj);
    }
  }
  return JS_DupValue(ctx, wrap->arrExitArray);
}

JSValue AreaWrap::GetPosX(JSContext* ctx, JSValue this_val) {
  AreaWrap* wrap = static_cast<AreaWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2DrlgLevelStrc* pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(wrap->dwAreaId);
  if (!pLevel)
    return JS_EXCEPTION;

  return JS_NewInt32(ctx, pLevel->dwPosX);
}

JSValue AreaWrap::GetPosY(JSContext* ctx, JSValue this_val) {
  AreaWrap* wrap = static_cast<AreaWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2DrlgLevelStrc* pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(wrap->dwAreaId);
  if (!pLevel)
    return JS_EXCEPTION;

  return JS_NewInt32(ctx, pLevel->dwPosY);
}

JSValue AreaWrap::GetSizeX(JSContext* ctx, JSValue this_val) {
  AreaWrap* wrap = static_cast<AreaWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2DrlgLevelStrc* pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(wrap->dwAreaId);
  if (!pLevel)
    return JS_EXCEPTION;

  return JS_NewInt32(ctx, pLevel->dwSizeX);
}

JSValue AreaWrap::GetSizeY(JSContext* ctx, JSValue this_val) {
  AreaWrap* wrap = static_cast<AreaWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2DrlgLevelStrc* pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(wrap->dwAreaId);
  if (!pLevel)
    return JS_EXCEPTION;

  return JS_NewInt32(ctx, pLevel->dwSizeY);
}

JSValue AreaWrap::GetArea(JSContext* ctx, JSValue /*this_val*/, int argc, JSValue* argv) {
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

  return AreaWrap::Instantiate(ctx, JS_UNDEFINED, nArea);
}

D2BS_BINDING_INTERNAL(AreaWrap, AreaWrap::Initialize)