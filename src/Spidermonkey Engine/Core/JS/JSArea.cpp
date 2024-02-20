#include "JSArea.h"

#include "Bindings.h"
#include "JSExits.h"    // JSExit
#include "MapHeader.h"  // ExitArray

JSObject* AreaWrap::Instantiate(JSContext* ctx, uint32_t dwAreaId) {
  JS::RootedObject global(ctx, JS::CurrentGlobalOrNull(ctx));
  JS::RootedValue constructor_val(ctx);
  if (!JS_GetProperty(ctx, global, "Area", &constructor_val)) {
    JS_ReportErrorASCII(ctx, "Could not find constructor object for Area");
    return nullptr;
  }
  if (!constructor_val.isObject()) {
    JS_ReportErrorASCII(ctx, "Area is not a constructor");
    return nullptr;
  }
  JS::RootedObject constructor(ctx, &constructor_val.toObject());

  JS::RootedObject obj(ctx, JS_New(ctx, constructor, JS::HandleValueArray::empty()));
  if (!obj) {
    JS_ReportErrorASCII(ctx, "Calling Area constructor failed");
    return nullptr;
  }
  AreaWrap* wrap = new AreaWrap(ctx, obj, dwAreaId);
  if (!wrap) {
    JS_ReportOutOfMemory(ctx);
    return nullptr;
  }
  return obj;
}

void AreaWrap::Initialize(JSContext* ctx, JS::HandleObject target) {
  JS::RootedObject proto(ctx, JS_InitClass(ctx, target, nullptr, &m_class, trampoline<New>, 0, m_props, nullptr, nullptr, nullptr));
  if (!proto) {
    Log("failed to initialize class Area");
    return;
  }

  JS_DefineFunction(ctx, target, "getArea", trampoline<GetArea>, 0, JSPROP_ENUMERATE);
}

AreaWrap::AreaWrap(JSContext* ctx, JS::HandleObject obj, uint32_t dwAreaId) : BaseObject(ctx, obj), dwAreaId(dwAreaId), dwExits(0), arrExitArray(ctx) {
}

void AreaWrap::finalize(JSFreeOp* /*fop*/, JSObject* obj) {
  BaseObject* wrap = BaseObject::FromJSObject(obj);
  if (wrap) {
    delete wrap;
  }
}

bool AreaWrap::New(JSContext* ctx, JS::CallArgs& args) {
  JS::RootedObject newObject(ctx, JS_NewObjectForConstructor(ctx, &m_class, args));
  if (!newObject) {
    THROW_ERROR(ctx, "failed to instantiate area");
  }
  // BUG(ejt): ask ejt
  JS_DefineProperties(ctx, newObject, m_props);

  args.rval().setObject(*newObject);
  return true;
}

bool AreaWrap::GetId(JSContext* ctx, JS::CallArgs& args) {
  AreaWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgLevelStrc* pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(wrap->dwAreaId);
  if (!pLevel)
    THROW_ERROR(ctx, "failed to find level");

  args.rval().setInt32(pLevel->dwLevelNo);
  return true;
}

bool AreaWrap::GetName(JSContext* ctx, JS::CallArgs& args) {
  AreaWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2LevelsTxt* pTxt = D2COMMON_GetLevelText(wrap->dwAreaId);
  if (pTxt) {
    args.rval().setString(JS_NewStringCopyZ(ctx, pTxt->szName));
    return true;
  }
  args.rval().setUndefined();
  return true;
}

bool AreaWrap::GetExits(JSContext* ctx, JS::CallArgs& args) {
  AreaWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgLevelStrc* pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(wrap->dwAreaId);
  if (!pLevel)
    THROW_ERROR(ctx, "failed to find level");

  if (wrap->arrExitArray.isUndefined()) {
    JS::RootedObject arr(ctx, JS::NewArrayObject(ctx, 0));

    ActMap* map = ActMap::GetMap(pLevel);

    ExitArray exits;
    map->GetExits(exits);
    map->CleanUp();
    int count = exits.size();
    for (int i = 0; i < count; i++) {
      JS::RootedObject obj(ctx, ExitWrap::Instantiate(ctx, &exits[i], wrap->dwAreaId));
      if (!obj) {
        JS_ReportErrorASCII(ctx, "Failed to create exit object!");
        return false;
      }
      JS_SetElement(ctx, arr, i, obj);
    }
    wrap->arrExitArray.setObject(*arr);
  }
  args.rval().setObject(wrap->arrExitArray.toObject());
  return true;
}

bool AreaWrap::GetPosX(JSContext* ctx, JS::CallArgs& args) {
  AreaWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgLevelStrc* pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(wrap->dwAreaId);
  if (!pLevel)
    THROW_ERROR(ctx, "failed to find level");

  args.rval().setInt32(pLevel->dwPosX);
  return true;
}

bool AreaWrap::GetPosY(JSContext* ctx, JS::CallArgs& args) {
  AreaWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgLevelStrc* pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(wrap->dwAreaId);
  if (!pLevel)
    THROW_ERROR(ctx, "failed to find level");

  args.rval().setInt32(pLevel->dwPosY);
  return true;
}

bool AreaWrap::GetSizeX(JSContext* ctx, JS::CallArgs& args) {
  AreaWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgLevelStrc* pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(wrap->dwAreaId);
  if (!pLevel)
    THROW_ERROR(ctx, "failed to find level");

  args.rval().setInt32(pLevel->dwSizeX);
  return true;
}

bool AreaWrap::GetSizeY(JSContext* ctx, JS::CallArgs& args) {
  AreaWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  D2DrlgLevelStrc* pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(wrap->dwAreaId);
  if (!pLevel) {
    THROW_ERROR(ctx, "failed to find level");
  }

  args.rval().setInt32(pLevel->dwSizeY);
  return true;
}

bool AreaWrap::GetArea(JSContext* ctx, JS::CallArgs& args) {
  if (!WaitForGameReady()) {
    JS_ReportErrorASCII(ctx, "Get Area: Game not ready");
    return false;
  }

  int32_t nArea = GetPlayerArea();

  if (args.length() == 1) {
    nArea = args[0].toInt32();
  }

  if (nArea < 0) {
    JS_ReportErrorASCII(ctx, "Invalid parameter passed to getArea!");
    return false;
  }

  D2DrlgLevelStrc* pLevel = D2DrlgLevelStrc::FindLevelFromLevelId(nArea);
  if (!pLevel) {
    args.rval().setBoolean(false);
    return true;
  }

  JS::RootedObject newObject(ctx, AreaWrap::Instantiate(ctx, nArea));
  if (!newObject) {
    THROW_ERROR(ctx, "failed to instantiate area");
  }
  args.rval().setObject(*newObject);
  return true;
}

D2BS_BINDING_INTERNAL(AreaWrap, AreaWrap::Initialize)
