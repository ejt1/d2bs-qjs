#include "JSExits.h"

#include "Bindings.h"

JSValue ExitWrap::Instantiate(JSContext* ctx, JSValue new_target, Exit* exit, uint32_t level_id) {
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

  ExitWrap* wrap = new ExitWrap(ctx, exit, level_id);
  if (!wrap) {
    JS_FreeValue(ctx, obj);
    return JS_ThrowOutOfMemory(ctx);
  }
  JS_SetOpaque(obj, wrap);

  return obj;
}

void ExitWrap::Initialize(JSContext* ctx, JSValue target) {
  JSClassDef def{};
  def.class_name = "Exit";
  def.finalizer = [](JSRuntime* /*rt*/, JSValue val) {
    ExitWrap* wrap = static_cast<ExitWrap*>(JS_GetOpaque(val, m_class_id));
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

  JSValue obj = JS_NewObjectProtoClass(ctx, proto, m_class_id);
  JS_SetClassProto(ctx, m_class_id, proto);
  JS_SetPropertyStr(ctx, target, "Exit", obj);
}

ExitWrap::ExitWrap(JSContext* /*ctx*/, Exit* exit, uint32_t level_id)
    : id(exit->Target), x(exit->Position.first), y(exit->Position.second), type(exit->Type), tileid(exit->TileId), level(level_id) {
}

// properties
JSValue ExitWrap::GetX(JSContext* ctx, JSValue this_val) {
  ExitWrap* wrap = static_cast<ExitWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  return JS_NewUint32(ctx, wrap->x);
}

JSValue ExitWrap::GetY(JSContext* ctx, JSValue this_val) {
  ExitWrap* wrap = static_cast<ExitWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  return JS_NewUint32(ctx, wrap->y);
}

JSValue ExitWrap::GetTarget(JSContext* ctx, JSValue this_val) {
  ExitWrap* wrap = static_cast<ExitWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  return JS_NewUint32(ctx, wrap->id);
}

JSValue ExitWrap::GetType(JSContext* ctx, JSValue this_val) {
  ExitWrap* wrap = static_cast<ExitWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  return JS_NewUint32(ctx, wrap->type);
}

JSValue ExitWrap::GetTileId(JSContext* ctx, JSValue this_val) {
  ExitWrap* wrap = static_cast<ExitWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  return JS_NewUint32(ctx, wrap->tileid);
}

JSValue ExitWrap::GetLevelId(JSContext* ctx, JSValue this_val) {
  ExitWrap* wrap = static_cast<ExitWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  return JS_NewUint32(ctx, wrap->level);
}

D2BS_BINDING_INTERNAL(ExitWrap, ExitWrap::Initialize)