#include "JSExits.h"

#include "Bindings.h"

JSObject* ExitWrap::Instantiate(JSContext* ctx, Exit* exit, uint32_t level_id) {
  JS::RootedObject global(ctx, JS::CurrentGlobalOrNull(ctx));
  JS::RootedValue constructor_val(ctx);
  if (!JS_GetProperty(ctx, global, "Exit", &constructor_val)) {
    JS_ReportErrorASCII(ctx, "Could not find constructor object for Exit");
    return nullptr;
  }
  if (!constructor_val.isObject()) {
    JS_ReportErrorASCII(ctx, "Exit is not a constructor");
    return nullptr;
  }
  JS::RootedObject constructor(ctx, &constructor_val.toObject());

  JS::RootedObject obj(ctx, JS_New(ctx, constructor, JS::HandleValueArray::empty()));
  if (!obj) {
    JS_ReportErrorASCII(ctx, "Calling Exit constructor failed");
    return nullptr;
  }
  ExitWrap* wrap = new ExitWrap(ctx, obj, exit, level_id);
  if (!wrap) {
    JS_ReportOutOfMemory(ctx);
    return nullptr;
  }
  return obj;
}

void ExitWrap::Initialize(JSContext* ctx, JS::HandleObject target) {
  JS::RootedObject proto(ctx, JS_InitClass(ctx, target, nullptr, &m_class, New, 0, m_props, nullptr, nullptr, nullptr));
  if (!proto) {
    Log("failed to initialize class Exit");
    return;
  }
}

ExitWrap::ExitWrap(JSContext* ctx, JS::HandleObject obj, Exit* exit, uint32_t level_id)
    : BaseObject(ctx, obj), id(exit->Target), x(exit->Position.first), y(exit->Position.second), type(exit->Type), tileid(exit->TileId), level(level_id) {
}

void ExitWrap::finalize(JSFreeOp* fop, JSObject* obj) {
  BaseObject* wrap = BaseObject::FromJSObject(obj);
  if (wrap) {
    delete wrap;
  }
}

bool ExitWrap::New(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  JS::RootedObject newObject(ctx, JS_NewObjectForConstructor(ctx, &m_class, args));
  if (!newObject) {
    THROW_ERROR(ctx, "failed to instantiate exit");
  }
  // BUG(ejt): ask ejt
  JS_DefineProperties(ctx, newObject, m_props);

  args.rval().setObject(*newObject);
  return true;
}

// properties
bool ExitWrap::GetX(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  ExitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  args.rval().setInt32(wrap->x);
  return true;
}

bool ExitWrap::GetY(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  ExitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  args.rval().setInt32(wrap->y);
  return true;
}

bool ExitWrap::GetTarget(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  ExitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  args.rval().setInt32(wrap->id);
  return true;
}

bool ExitWrap::GetType(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  ExitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  args.rval().setInt32(wrap->type);
  return true;
}

bool ExitWrap::GetTileId(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  ExitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  args.rval().setInt32(wrap->tileid);
  return true;
}

bool ExitWrap::GetLevelId(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  ExitWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  args.rval().setInt32(wrap->level);
  return true;
}

D2BS_BINDING_INTERNAL(ExitWrap, ExitWrap::Initialize)
