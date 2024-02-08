#include "JSScreenHook.h"
#include "Engine.h"
#include "ScreenHook.h"
#include "Script.h"
#include "File.h"
#include <Helpers.h>

#include "Bindings.h"

JSValue FrameWrap::Instantiate(JSContext* ctx, JSValue new_target, FrameHook* frame) {
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

  FrameWrap* wrap = new FrameWrap(ctx, frame);
  if (!wrap) {
    JS_FreeValue(ctx, obj);
    return JS_ThrowOutOfMemory(ctx);
  }
  JS_SetOpaque(obj, wrap);

  return obj;
}

void FrameWrap::Initialize(JSContext* ctx, JSValue target) {
  JSClassDef def{};
  def.class_name = "Frame";
  def.finalizer = [](JSRuntime* /*rt*/, JSValue val) {
    FrameWrap* wrap = static_cast<FrameWrap*>(JS_GetOpaque(val, m_class_id));
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

  JSValue obj = JS_NewCFunction2(ctx, New, "Frame", 0, JS_CFUNC_constructor, 0);
  JS_SetConstructor(ctx, obj, proto);

  JS_SetClassProto(ctx, m_class_id, proto);
  JS_SetPropertyStr(ctx, target, "Frame", obj);
}

FrameWrap::FrameWrap(JSContext* /*ctx*/, FrameHook* frame) : pFrame(frame) {
}

FrameWrap::~FrameWrap() {
  Genhook::EnterGlobalSection();
  delete pFrame;
  Genhook::LeaveGlobalSection();
}

JSValue FrameWrap::New(JSContext* ctx, JSValue new_target, int argc, JSValue* argv) {
  Script* script = (Script*)JS_GetContextOpaque(ctx);

  uint32_t x = 0, y = 0, x2 = 0, y2 = 0;
  Align align = Left;
  bool automap = false;
  JSValue click = JS_UNDEFINED, hover = JS_UNDEFINED;

  if (argc > 0 && JS_IsNumber(argv[0]))
    JS_ToUint32(ctx, &x, argv[0]);
  if (argc > 1 && JS_IsNumber(argv[1]))
    JS_ToUint32(ctx, &y, argv[1]);
  if (argc > 2 && JS_IsNumber(argv[2]))
    JS_ToUint32(ctx, &x2, argv[2]);
  if (argc > 3 && JS_IsNumber(argv[3]))
    JS_ToUint32(ctx, &y2, argv[3]);
  if (argc > 4 && JS_IsNumber(argv[4]))
    JS_ToUint32(ctx, (uint32_t*)(&align), argv[4]);
  if (argc > 5 && JS_IsBool(argv[5]))
    automap = !!JS_ToBool(ctx, argv[5]);
  if (argc > 6 && JS_IsFunction(ctx, argv[6]))
    click = argv[6];
  if (argc > 7 && JS_IsFunction(ctx, argv[7]))
    hover = argv[7];

  // framehooks don't work out of game -- they just crash
  FrameHook* pFrameHook = new FrameHook(script, JS_UNDEFINED, x, y, x2, y2, automap, align, IG);
  if (!pFrameHook)
    THROW_ERROR(ctx, "Failed to create framehook");

  JSValue hook = FrameWrap::Instantiate(ctx, new_target, pFrameHook);
  if (JS_IsException(hook))
    THROW_ERROR(ctx, "Failed to create frame object");

  pFrameHook->SetClickHandler(click);
  pFrameHook->SetHoverHandler(hover);

  return hook;
}

// properties
JSValue FrameWrap::GetX(JSContext* ctx, JSValue this_val) {
  FrameWrap* wrap = static_cast<FrameWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FrameHook* pFramehook = wrap->pFrame;
  return JS_NewUint32(ctx, pFramehook->GetX());
}

JSValue FrameWrap::SetX(JSContext* ctx, JSValue this_val, JSValue val) {
  FrameWrap* wrap = static_cast<FrameWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FrameHook* pFramehook = wrap->pFrame;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pFramehook->SetX(ival);
  return JS_UNDEFINED;
}

JSValue FrameWrap::GetY(JSContext* ctx, JSValue this_val) {
  FrameWrap* wrap = static_cast<FrameWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FrameHook* pFramehook = wrap->pFrame;
  return JS_NewUint32(ctx, pFramehook->GetY());
}

JSValue FrameWrap::SetY(JSContext* ctx, JSValue this_val, JSValue val) {
  FrameWrap* wrap = static_cast<FrameWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FrameHook* pFramehook = wrap->pFrame;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pFramehook->SetY(ival);
  return JS_UNDEFINED;
}

JSValue FrameWrap::GetSizeX(JSContext* ctx, JSValue this_val) {
  FrameWrap* wrap = static_cast<FrameWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FrameHook* pFramehook = wrap->pFrame;
  return JS_NewUint32(ctx, pFramehook->GetXSize());
}

JSValue FrameWrap::SetSizeX(JSContext* ctx, JSValue this_val, JSValue val) {
  FrameWrap* wrap = static_cast<FrameWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FrameHook* pFramehook = wrap->pFrame;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pFramehook->SetXSize(ival);
  return JS_UNDEFINED;
}

JSValue FrameWrap::GetSizeY(JSContext* ctx, JSValue this_val) {
  FrameWrap* wrap = static_cast<FrameWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FrameHook* pFramehook = wrap->pFrame;
  return JS_NewUint32(ctx, pFramehook->GetYSize());
}

JSValue FrameWrap::SetSizeY(JSContext* ctx, JSValue this_val, JSValue val) {
  FrameWrap* wrap = static_cast<FrameWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FrameHook* pFramehook = wrap->pFrame;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pFramehook->SetYSize(ival);
  return JS_UNDEFINED;
}

JSValue FrameWrap::GetVisible(JSContext* ctx, JSValue this_val) {
  FrameWrap* wrap = static_cast<FrameWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FrameHook* pFramehook = wrap->pFrame;
  return JS_NewBool(ctx, pFramehook->GetIsVisible());
}

JSValue FrameWrap::SetVisible(JSContext* ctx, JSValue this_val, JSValue val) {
  FrameWrap* wrap = static_cast<FrameWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FrameHook* pFramehook = wrap->pFrame;
  if (!JS_IsBool(val)) {
    return JS_EXCEPTION;
  }
  pFramehook->SetIsVisible(JS_ToBool(ctx, val));
  return JS_UNDEFINED;
}

JSValue FrameWrap::GetAlign(JSContext* ctx, JSValue this_val) {
  FrameWrap* wrap = static_cast<FrameWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FrameHook* pFramehook = wrap->pFrame;
  return JS_NewUint32(ctx, pFramehook->GetAlign());
}

JSValue FrameWrap::SetAlign(JSContext* ctx, JSValue this_val, JSValue val) {
  FrameWrap* wrap = static_cast<FrameWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FrameHook* pFramehook = wrap->pFrame;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pFramehook->SetAlign((Align)ival);
  return JS_UNDEFINED;
}

JSValue FrameWrap::GetZOrder(JSContext* ctx, JSValue this_val) {
  FrameWrap* wrap = static_cast<FrameWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FrameHook* pFramehook = wrap->pFrame;
  return JS_NewUint32(ctx, pFramehook->GetZOrder());
}

JSValue FrameWrap::SetZOrder(JSContext* ctx, JSValue this_val, JSValue val) {
  FrameWrap* wrap = static_cast<FrameWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FrameHook* pFramehook = wrap->pFrame;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pFramehook->SetZOrder((ushort)ival);
  return JS_UNDEFINED;
}

JSValue FrameWrap::GetClick(JSContext* ctx, JSValue this_val) {
  FrameWrap* wrap = static_cast<FrameWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FrameHook* pFramehook = wrap->pFrame;
  return JS_DupValue(ctx, pFramehook->GetClickHandler());
}

JSValue FrameWrap::SetClick(JSContext* /*ctx*/, JSValue this_val, JSValue val) {
  FrameWrap* wrap = static_cast<FrameWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FrameHook* pFramehook = wrap->pFrame;
  pFramehook->SetClickHandler(val);
  return JS_UNDEFINED;
}

JSValue FrameWrap::GetHover(JSContext* ctx, JSValue this_val) {
  FrameWrap* wrap = static_cast<FrameWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FrameHook* pFramehook = wrap->pFrame;
  return JS_DupValue(ctx, pFramehook->GetHoverHandler());
}

JSValue FrameWrap::SetHover(JSContext* /*ctx*/, JSValue this_val, JSValue val) {
  FrameWrap* wrap = static_cast<FrameWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FrameHook* pFramehook = wrap->pFrame;
  pFramehook->SetHoverHandler(val);
  return JS_UNDEFINED;
}

// functions
JSValue FrameWrap::Remove(JSContext* /*ctx*/, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  FrameWrap* wrap = static_cast<FrameWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  return JS_UNDEFINED;
}

D2BS_BINDING_INTERNAL(FrameWrap, FrameWrap::Initialize)

JSValue BoxWrap::Instantiate(JSContext* ctx, JSValue new_target, BoxHook* box) {
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

  BoxWrap* wrap = new BoxWrap(ctx, box);
  if (!wrap) {
    JS_FreeValue(ctx, obj);
    return JS_ThrowOutOfMemory(ctx);
  }
  JS_SetOpaque(obj, wrap);

  return obj;
}

void BoxWrap::Initialize(JSContext* ctx, JSValue target) {
  JSClassDef def{};
  def.class_name = "Box";
  def.finalizer = [](JSRuntime* /*rt*/, JSValue val) {
    BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(val, m_class_id));
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

  JSValue obj = JS_NewCFunction2(ctx, New, "Box", 0, JS_CFUNC_constructor, 0);
  JS_SetConstructor(ctx, obj, proto);

  JS_SetClassProto(ctx, m_class_id, proto);
  JS_SetPropertyStr(ctx, target, "Box", obj);
}

BoxWrap::BoxWrap(JSContext* /*ctx*/, BoxHook* box) : pBox(box) {
}

BoxWrap::~BoxWrap() {
  Genhook::EnterGlobalSection();
  delete pBox;
  Genhook::LeaveGlobalSection();
}

JSValue BoxWrap::New(JSContext* ctx, JSValue new_target, int argc, JSValue* argv) {
  Script* script = (Script*)JS_GetContextOpaque(ctx);

  ScreenhookState state = (script->GetMode() == kScriptModeMenu) ? OOG : IG;
  uint32_t x = 0, y = 0, x2 = 0, y2 = 0, color = 0, opacity = 0, align = Left;
  bool automap = false;
  JSValue click = JS_UNDEFINED, hover = JS_UNDEFINED;

  if (argc > 0 && JS_IsNumber(argv[0]))
    JS_ToUint32(ctx, &x, argv[0]);
  if (argc > 1 && JS_IsNumber(argv[1]))
    JS_ToUint32(ctx, &y, argv[1]);
  if (argc > 2 && JS_IsNumber(argv[2]))
    JS_ToUint32(ctx, &x2, argv[2]);
  if (argc > 3 && JS_IsNumber(argv[3]))
    JS_ToUint32(ctx, &y2, argv[3]);
  if (argc > 4 && JS_IsNumber(argv[4]))
    JS_ToUint32(ctx, &color, argv[4]);
  if (argc > 5 && JS_IsNumber(argv[5]))
    JS_ToUint32(ctx, &opacity, argv[5]);
  if (argc > 6 && JS_IsNumber(argv[6]))
    JS_ToUint32(ctx, &align, argv[6]);
  if (argc > 7 && JS_IsBool(argv[7]))
    automap = !!JS_ToBool(ctx, argv[7]);
  if (argc > 8 && JS_IsFunction(ctx, argv[8]))
    click = argv[8];
  if (argc > 9 && JS_IsFunction(ctx, argv[9]))
    hover = argv[9];

  BoxHook* pBoxHook = new BoxHook(script, JS_UNDEFINED, x, y, x2, y2, (ushort)color, (ushort)opacity, automap, (Align)align, state);
  if (!pBoxHook)
    THROW_ERROR(ctx, "Unable to initalize a box class.");

  JSValue hook = BoxWrap::Instantiate(ctx, new_target, pBoxHook);

  pBoxHook->SetClickHandler(click);
  pBoxHook->SetHoverHandler(hover);

  return hook;
}

// properties
JSValue BoxWrap::GetX(JSContext* ctx, JSValue this_val) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  BoxHook* pBox = wrap->pBox;
  return JS_NewUint32(ctx, pBox->GetX());
}

JSValue BoxWrap::SetX(JSContext* ctx, JSValue this_val, JSValue val) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  BoxHook* pBox = wrap->pBox;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pBox->SetX(ival);
  return JS_UNDEFINED;
}

JSValue BoxWrap::GetY(JSContext* ctx, JSValue this_val) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  BoxHook* pBox = wrap->pBox;
  return JS_NewUint32(ctx, pBox->GetY());
}

JSValue BoxWrap::SetY(JSContext* ctx, JSValue this_val, JSValue val) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  BoxHook* pBox = wrap->pBox;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pBox->SetY(ival);
  return JS_UNDEFINED;
}

JSValue BoxWrap::GetSizeX(JSContext* ctx, JSValue this_val) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  BoxHook* pBox = wrap->pBox;
  return JS_NewUint32(ctx, pBox->GetXSize());
}

JSValue BoxWrap::SetSizeX(JSContext* ctx, JSValue this_val, JSValue val) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  BoxHook* pBox = wrap->pBox;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pBox->SetXSize(ival);
  return JS_UNDEFINED;
}

JSValue BoxWrap::GetSizeY(JSContext* ctx, JSValue this_val) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  BoxHook* pBox = wrap->pBox;
  return JS_NewUint32(ctx, pBox->GetYSize());
}

JSValue BoxWrap::SetSizeY(JSContext* ctx, JSValue this_val, JSValue val) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  BoxHook* pBox = wrap->pBox;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pBox->SetYSize(ival);
  return JS_UNDEFINED;
}

JSValue BoxWrap::GetVisible(JSContext* ctx, JSValue this_val) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  BoxHook* pBox = wrap->pBox;
  return JS_NewBool(ctx, pBox->GetIsVisible());
}

JSValue BoxWrap::SetVisible(JSContext* ctx, JSValue this_val, JSValue val) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  BoxHook* pBox = wrap->pBox;
  if (!JS_IsBool(val)) {
    return JS_EXCEPTION;
  }
  pBox->SetIsVisible(JS_ToBool(ctx, val));
  return JS_UNDEFINED;
}

JSValue BoxWrap::GetAlign(JSContext* ctx, JSValue this_val) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  BoxHook* pBox = wrap->pBox;
  return JS_NewUint32(ctx, pBox->GetAlign());
}

JSValue BoxWrap::SetAlign(JSContext* ctx, JSValue this_val, JSValue val) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  BoxHook* pBox = wrap->pBox;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pBox->SetAlign((Align)ival);
  return JS_UNDEFINED;
}

JSValue BoxWrap::GetZOrder(JSContext* ctx, JSValue this_val) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  BoxHook* pBox = wrap->pBox;
  return JS_NewUint32(ctx, pBox->GetZOrder());
}

JSValue BoxWrap::SetZOrder(JSContext* ctx, JSValue this_val, JSValue val) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  BoxHook* pBox = wrap->pBox;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pBox->SetZOrder((ushort)ival);
  return JS_UNDEFINED;
}

JSValue BoxWrap::GetClick(JSContext* ctx, JSValue this_val) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  BoxHook* pBox = wrap->pBox;
  return JS_DupValue(ctx, pBox->GetClickHandler());
}

JSValue BoxWrap::SetClick(JSContext* /*ctx*/, JSValue this_val, JSValue val) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  BoxHook* pBox = wrap->pBox;
  pBox->SetClickHandler(val);
  return JS_UNDEFINED;
}

JSValue BoxWrap::GetHover(JSContext* ctx, JSValue this_val) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  BoxHook* pBox = wrap->pBox;
  return JS_DupValue(ctx, pBox->GetHoverHandler());
}

JSValue BoxWrap::SetHover(JSContext* /*ctx*/, JSValue this_val, JSValue val) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  BoxHook* pBox = wrap->pBox;
  pBox->SetHoverHandler(val);
  return JS_UNDEFINED;
}

JSValue BoxWrap::GetColor(JSContext* ctx, JSValue this_val) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  BoxHook* pBox = wrap->pBox;
  return JS_NewUint32(ctx, pBox->GetColor());
}

JSValue BoxWrap::SetColor(JSContext* ctx, JSValue this_val, JSValue val) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  BoxHook* pBox = wrap->pBox;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pBox->SetColor((ushort)ival);
  return JS_UNDEFINED;
}

JSValue BoxWrap::GetOpacity(JSContext* ctx, JSValue this_val) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  BoxHook* pBox = wrap->pBox;
  return JS_NewUint32(ctx, pBox->GetOpacity());
}

JSValue BoxWrap::SetOpacity(JSContext* ctx, JSValue this_val, JSValue val) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  BoxHook* pBox = wrap->pBox;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pBox->SetOpacity((ushort)ival);
  return JS_UNDEFINED;
}

// functions
JSValue BoxWrap::Remove(JSContext* /*ctx*/, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  BoxWrap* wrap = static_cast<BoxWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  return JS_UNDEFINED;
}

D2BS_BINDING_INTERNAL(BoxWrap, BoxWrap::Initialize)

// Line functions

JSValue LineWrap::Instantiate(JSContext* ctx, JSValue new_target, LineHook* line) {
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

  LineWrap* wrap = new LineWrap(ctx, line);
  if (!wrap) {
    JS_FreeValue(ctx, obj);
    return JS_ThrowOutOfMemory(ctx);
  }
  JS_SetOpaque(obj, wrap);

  return obj;
}

void LineWrap::Initialize(JSContext* ctx, JSValue target) {
  JSClassDef def{};
  def.class_name = "Line";
  def.finalizer = [](JSRuntime* /*rt*/, JSValue val) {
    LineWrap* wrap = static_cast<LineWrap*>(JS_GetOpaque(val, m_class_id));
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

  JSValue obj = JS_NewCFunction2(ctx, New, "Line", 0, JS_CFUNC_constructor, 0);
  JS_SetConstructor(ctx, obj, proto);

  JS_SetClassProto(ctx, m_class_id, proto);
  JS_SetPropertyStr(ctx, target, "Line", obj);
}

LineWrap::LineWrap(JSContext* /*ctx*/, LineHook* line) : pLine(line) {
}

LineWrap::~LineWrap() {
  Genhook::EnterGlobalSection();
  delete pLine;
  Genhook::LeaveGlobalSection();
}

JSValue LineWrap::New(JSContext* ctx, JSValue new_target, int argc, JSValue* argv) {
  Script* script = (Script*)JS_GetContextOpaque(ctx);

  ScreenhookState state = (script->GetMode() == kScriptModeMenu) ? OOG : IG;
  uint32_t x = 0, y = 0, x2 = 0, y2 = 0, color = 0;
  bool automap = false;
  JSValue click = JS_UNDEFINED, hover = JS_UNDEFINED;

  if (argc > 0 && JS_IsNumber(argv[0]))
    JS_ToUint32(ctx, &x, argv[0]);
  if (argc > 1 && JS_IsNumber(argv[1]))
    JS_ToUint32(ctx, &y, argv[1]);
  if (argc > 2 && JS_IsNumber(argv[2]))
    JS_ToUint32(ctx, &x2, argv[2]);
  if (argc > 3 && JS_IsNumber(argv[3]))
    JS_ToUint32(ctx, &y2, argv[3]);
  if (argc > 4 && JS_IsNumber(argv[4]))
    JS_ToUint32(ctx, &color, argv[4]);
  if (argc > 5 && JS_IsBool(argv[5]))
    automap = !!JS_ToBool(ctx, argv[5]);
  if (argc > 6 && JS_IsFunction(ctx, argv[6]))
    click = argv[6];
  if (argc > 7 && JS_IsFunction(ctx, argv[7]))
    hover = argv[7];

  LineHook* pLineHook = new LineHook(script, JS_UNDEFINED, x, y, x2, y2, static_cast<ushort>(color), automap, Left, state);
  if (!pLineHook) {
    THROW_ERROR(ctx, "Unable to initalize a line class.");
  }

  JSValue hook = LineWrap::Instantiate(ctx, new_target, pLineHook);

  pLineHook->SetClickHandler(click);
  pLineHook->SetHoverHandler(hover);

  return hook;
}

// properties
JSValue LineWrap::GetX(JSContext* ctx, JSValue this_val) {
  LineWrap* wrap = static_cast<LineWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  LineHook* pLine = wrap->pLine;
  return JS_NewUint32(ctx, pLine->GetX());
}

JSValue LineWrap::SetX(JSContext* ctx, JSValue this_val, JSValue val) {
  LineWrap* wrap = static_cast<LineWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  LineHook* pLine = wrap->pLine;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pLine->SetX(ival);
  return JS_UNDEFINED;
}

JSValue LineWrap::GetY(JSContext* ctx, JSValue this_val) {
  LineWrap* wrap = static_cast<LineWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  LineHook* pLine = wrap->pLine;
  return JS_NewUint32(ctx, pLine->GetY());
}

JSValue LineWrap::SetY(JSContext* ctx, JSValue this_val, JSValue val) {
  LineWrap* wrap = static_cast<LineWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  LineHook* pLine = wrap->pLine;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pLine->SetY(ival);
  return JS_UNDEFINED;
}

JSValue LineWrap::GetX2(JSContext* ctx, JSValue this_val) {
  LineWrap* wrap = static_cast<LineWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  LineHook* pLine = wrap->pLine;
  return JS_NewUint32(ctx, pLine->GetX2());
}

JSValue LineWrap::SetX2(JSContext* ctx, JSValue this_val, JSValue val) {
  LineWrap* wrap = static_cast<LineWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  LineHook* pLine = wrap->pLine;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pLine->SetX2(ival);
  return JS_UNDEFINED;
}

JSValue LineWrap::GetY2(JSContext* ctx, JSValue this_val) {
  LineWrap* wrap = static_cast<LineWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  LineHook* pLine = wrap->pLine;
  return JS_NewUint32(ctx, pLine->GetY2());
}

JSValue LineWrap::SetY2(JSContext* ctx, JSValue this_val, JSValue val) {
  LineWrap* wrap = static_cast<LineWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  LineHook* pLine = wrap->pLine;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pLine->SetY2(ival);
  return JS_UNDEFINED;
}

JSValue LineWrap::GetVisible(JSContext* ctx, JSValue this_val) {
  LineWrap* wrap = static_cast<LineWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  LineHook* pLine = wrap->pLine;
  return JS_NewBool(ctx, pLine->GetIsVisible());
}

JSValue LineWrap::SetVisible(JSContext* ctx, JSValue this_val, JSValue val) {
  LineWrap* wrap = static_cast<LineWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  LineHook* pLine = wrap->pLine;
  if (!JS_IsBool(val)) {
    return JS_EXCEPTION;
  }
  pLine->SetIsVisible(JS_ToBool(ctx, val));
  return JS_UNDEFINED;
}

JSValue LineWrap::GetColor(JSContext* ctx, JSValue this_val) {
  LineWrap* wrap = static_cast<LineWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  LineHook* pLine = wrap->pLine;
  return JS_NewUint32(ctx, pLine->GetColor());
}

JSValue LineWrap::SetColor(JSContext* ctx, JSValue this_val, JSValue val) {
  LineWrap* wrap = static_cast<LineWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  LineHook* pLine = wrap->pLine;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pLine->SetColor(static_cast<ushort>(ival));
  return JS_UNDEFINED;
}

JSValue LineWrap::GetZOrder(JSContext* ctx, JSValue this_val) {
  LineWrap* wrap = static_cast<LineWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  LineHook* pLine = wrap->pLine;
  return JS_NewUint32(ctx, pLine->GetZOrder());
}

JSValue LineWrap::SetZOrder(JSContext* ctx, JSValue this_val, JSValue val) {
  LineWrap* wrap = static_cast<LineWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  LineHook* pLine = wrap->pLine;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pLine->SetZOrder((ushort)ival);
  return JS_UNDEFINED;
}

JSValue LineWrap::GetClick(JSContext* ctx, JSValue this_val) {
  LineWrap* wrap = static_cast<LineWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  LineHook* pLine = wrap->pLine;
  return JS_DupValue(ctx, pLine->GetClickHandler());
}

JSValue LineWrap::SetClick(JSContext* /*ctx*/, JSValue this_val, JSValue val) {
  LineWrap* wrap = static_cast<LineWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  LineHook* pLine = wrap->pLine;
  pLine->SetClickHandler(val);
  return JS_UNDEFINED;
}

JSValue LineWrap::GetHover(JSContext* ctx, JSValue this_val) {
  LineWrap* wrap = static_cast<LineWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  LineHook* pLine = wrap->pLine;
  return JS_DupValue(ctx, pLine->GetHoverHandler());
}

JSValue LineWrap::SetHover(JSContext* /*ctx*/, JSValue this_val, JSValue val) {
  LineWrap* wrap = static_cast<LineWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  LineHook* pLine = wrap->pLine;
  pLine->SetHoverHandler(val);
  return JS_UNDEFINED;
}

// functions
JSValue LineWrap::Remove(JSContext* /*ctx*/, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  LineWrap* wrap = static_cast<LineWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  return JS_UNDEFINED;
}

D2BS_BINDING_INTERNAL(LineWrap, LineWrap::Initialize)

JSValue TextWrap::Instantiate(JSContext* ctx, JSValue new_target, TextHook* text) {
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

  TextWrap* wrap = new TextWrap(ctx, text);
  if (!wrap) {
    JS_FreeValue(ctx, obj);
    return JS_ThrowOutOfMemory(ctx);
  }
  JS_SetOpaque(obj, wrap);

  return obj;
}

void TextWrap::Initialize(JSContext* ctx, JSValue target) {
  JSClassDef def{};
  def.class_name = "Text";
  def.finalizer = [](JSRuntime* /*rt*/, JSValue val) {
    TextWrap* wrap = static_cast<TextWrap*>(JS_GetOpaque(val, m_class_id));
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

  JSValue obj = JS_NewCFunction2(ctx, New, "Text", 0, JS_CFUNC_constructor, 0);
  JS_SetConstructor(ctx, obj, proto);

  JS_SetClassProto(ctx, m_class_id, proto);
  JS_SetPropertyStr(ctx, target, "Text", obj);
}

TextWrap::TextWrap(JSContext* /*ctx*/, TextHook* text) : pText(text) {
}

TextWrap::~TextWrap() {
  Genhook::EnterGlobalSection();
  delete pText;
  Genhook::LeaveGlobalSection();
}

JSValue TextWrap::New(JSContext* ctx, JSValue new_target, int argc, JSValue* argv) {
  Script* script = (Script*)JS_GetContextOpaque(ctx);

  ScreenhookState state = (script->GetMode() == kScriptModeMenu) ? OOG : IG;
  uint32_t x = 0, y = 0, color = 0, font = 0, align = Left;
  bool automap = false;
  JSValue click = JS_UNDEFINED, hover = JS_UNDEFINED;
  std::wstring szText;

  if (argc > 0 && JS_IsString(argv[0])) {
    const char* str = JS_ToCString(ctx, argv[0]);
    if (!str) {
      return JS_EXCEPTION;
    }
    szText = AnsiToWide(str);
    JS_FreeCString(ctx, str);
  }
  if (argc > 1 && JS_IsNumber(argv[1]))
    JS_ToUint32(ctx, &x, argv[1]);
  if (argc > 2 && JS_IsNumber(argv[2]))
    JS_ToUint32(ctx, &y, argv[2]);
  if (argc > 3 && JS_IsNumber(argv[3]))
    JS_ToUint32(ctx, &color, argv[3]);
  if (argc > 4 && JS_IsNumber(argv[4]))
    JS_ToUint32(ctx, &font, argv[4]);
  if (argc > 5 && JS_IsNumber(argv[5]))
    JS_ToUint32(ctx, &align, argv[5]);
  if (argc > 6 && JS_IsBool(argv[6]))
    automap = !!JS_ToBool(ctx, argv[6]);
  if (argc > 7 && JS_IsFunction(ctx, argv[7]))
    click = argv[7];
  if (argc > 8 && JS_IsFunction(ctx, argv[8]))
    hover = argv[8];

  TextHook* pTextHook = new TextHook(script, JS_UNDEFINED, szText.c_str(), x, y, static_cast<ushort>(font), static_cast<ushort>(color), automap, (Align)align, state);
  if (!pTextHook) {
    THROW_ERROR(ctx, "Failed to create texthook");
  }

  JSValue hook = TextWrap::Instantiate(ctx, new_target, pTextHook);

  pTextHook->SetClickHandler(click);
  pTextHook->SetHoverHandler(hover);
  return hook;
}

// properties
JSValue TextWrap::GetX(JSContext* ctx, JSValue this_val) {
  TextWrap* wrap = static_cast<TextWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  TextHook* pText = wrap->pText;
  return JS_NewUint32(ctx, pText->GetX());
}

JSValue TextWrap::SetX(JSContext* ctx, JSValue this_val, JSValue val) {
  TextWrap* wrap = static_cast<TextWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  TextHook* pText = wrap->pText;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pText->SetX(ival);
  return JS_UNDEFINED;
}

JSValue TextWrap::GetY(JSContext* ctx, JSValue this_val) {
  TextWrap* wrap = static_cast<TextWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  TextHook* pText = wrap->pText;
  return JS_NewUint32(ctx, pText->GetY());
}

JSValue TextWrap::SetY(JSContext* ctx, JSValue this_val, JSValue val) {
  TextWrap* wrap = static_cast<TextWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  TextHook* pText = wrap->pText;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pText->SetY(ival);
  return JS_UNDEFINED;
}

JSValue TextWrap::GetColor(JSContext* ctx, JSValue this_val) {
  TextWrap* wrap = static_cast<TextWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  TextHook* pText = wrap->pText;
  return JS_NewUint32(ctx, pText->GetColor());
}

JSValue TextWrap::SetColor(JSContext* ctx, JSValue this_val, JSValue val) {
  TextWrap* wrap = static_cast<TextWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  TextHook* pText = wrap->pText;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pText->SetColor((ushort)ival);
  return JS_UNDEFINED;
}

JSValue TextWrap::GetVisible(JSContext* ctx, JSValue this_val) {
  TextWrap* wrap = static_cast<TextWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  TextHook* pText = wrap->pText;
  return JS_NewBool(ctx, pText->GetIsVisible());
}

JSValue TextWrap::SetVisible(JSContext* ctx, JSValue this_val, JSValue val) {
  TextWrap* wrap = static_cast<TextWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  TextHook* pText = wrap->pText;
  if (!JS_IsBool(val)) {
    return JS_EXCEPTION;
  }
  pText->SetIsVisible(JS_ToBool(ctx, val));
  return JS_UNDEFINED;
}

JSValue TextWrap::GetAlign(JSContext* ctx, JSValue this_val) {
  TextWrap* wrap = static_cast<TextWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  TextHook* pText = wrap->pText;
  return JS_NewUint32(ctx, pText->GetAlign());
}

JSValue TextWrap::SetAlign(JSContext* ctx, JSValue this_val, JSValue val) {
  TextWrap* wrap = static_cast<TextWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  TextHook* pText = wrap->pText;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pText->SetAlign((Align)ival);
  return JS_UNDEFINED;
}

JSValue TextWrap::GetZOrder(JSContext* ctx, JSValue this_val) {
  TextWrap* wrap = static_cast<TextWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  TextHook* pText = wrap->pText;
  return JS_NewUint32(ctx, pText->GetZOrder());
}

JSValue TextWrap::SetZOrder(JSContext* ctx, JSValue this_val, JSValue val) {
  TextWrap* wrap = static_cast<TextWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  TextHook* pText = wrap->pText;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pText->SetZOrder((ushort)ival);
  return JS_UNDEFINED;
}

JSValue TextWrap::GetClick(JSContext* ctx, JSValue this_val) {
  TextWrap* wrap = static_cast<TextWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  TextHook* pText = wrap->pText;
  return JS_DupValue(ctx, pText->GetClickHandler());
}

JSValue TextWrap::SetClick(JSContext* /*ctx*/, JSValue this_val, JSValue val) {
  TextWrap* wrap = static_cast<TextWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  TextHook* pText = wrap->pText;
  pText->SetClickHandler(val);
  return JS_UNDEFINED;
}

JSValue TextWrap::GetHover(JSContext* ctx, JSValue this_val) {
  TextWrap* wrap = static_cast<TextWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  TextHook* pText = wrap->pText;
  return JS_DupValue(ctx, pText->GetHoverHandler());
}

JSValue TextWrap::SetHover(JSContext* /*ctx*/, JSValue this_val, JSValue val) {
  TextWrap* wrap = static_cast<TextWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  TextHook* pText = wrap->pText;
  pText->SetHoverHandler(val);
  return JS_UNDEFINED;
}

JSValue TextWrap::GetFont(JSContext* ctx, JSValue this_val) {
  TextWrap* wrap = static_cast<TextWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  TextHook* pText = wrap->pText;
  return JS_NewUint32(ctx, pText->GetFont());
}

JSValue TextWrap::SetFont(JSContext* ctx, JSValue this_val, JSValue val) {
  TextWrap* wrap = static_cast<TextWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  TextHook* pText = wrap->pText;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pText->SetFont((ushort)ival);
  return JS_UNDEFINED;
}

JSValue TextWrap::GetText(JSContext* ctx, JSValue this_val) {
  TextWrap* wrap = static_cast<TextWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  TextHook* pText = wrap->pText;
  return JS_NewString(ctx, pText->GetText());
}

JSValue TextWrap::SetText(JSContext* ctx, JSValue this_val, JSValue val) {
  TextWrap* wrap = static_cast<TextWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  TextHook* pTextHook = wrap->pText;
  if (JS_IsString(val)) {
    const char* szText = JS_ToCString(ctx, val);
    if (!szText) {
      return JS_EXCEPTION;
    }
    std::wstring pText = AnsiToWide(szText);
    pTextHook->SetText(pText.c_str());
    JS_FreeCString(ctx, szText);
  }
  return JS_UNDEFINED;
}

// functions
JSValue TextWrap::Remove(JSContext* /*ctx*/, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  TextWrap* wrap = static_cast<TextWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  return JS_UNDEFINED;
}

D2BS_BINDING_INTERNAL(TextWrap, TextWrap::Initialize)

// Function to create a image which gets called on a "new Image ()"

JSValue ImageWrap::Instantiate(JSContext* ctx, JSValue new_target, ImageHook* image) {
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

  ImageWrap* wrap = new ImageWrap(ctx, image);
  if (!wrap) {
    JS_FreeValue(ctx, obj);
    return JS_ThrowOutOfMemory(ctx);
  }
  JS_SetOpaque(obj, wrap);

  return obj;
}

void ImageWrap::Initialize(JSContext* ctx, JSValue target) {
  JSClassDef def{};
  def.class_name = "Image";
  def.finalizer = [](JSRuntime* /*rt*/, JSValue val) {
    ImageWrap* wrap = static_cast<ImageWrap*>(JS_GetOpaque(val, m_class_id));
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

  JSValue obj = JS_NewCFunction2(ctx, New, "Image", 0, JS_CFUNC_constructor, 0);
  JS_SetConstructor(ctx, obj, proto);

  JS_SetClassProto(ctx, m_class_id, proto);
  JS_SetPropertyStr(ctx, target, "Image", obj);
}

ImageWrap::ImageWrap(JSContext* /*ctx*/, ImageHook* image) : pImage(image) {
}

ImageWrap::~ImageWrap() {
  Genhook::EnterGlobalSection();
  delete pImage;
  Genhook::LeaveGlobalSection();
}

JSValue ImageWrap::New(JSContext* ctx, JSValue new_target, int argc, JSValue* argv) {
  Script* script = (Script*)JS_GetContextOpaque(ctx);

  ScreenhookState state = (script->GetMode() == kScriptModeMenu) ? OOG : IG;
  uint32_t x = 0, y = 0, color = 0, align = Left;
  bool automap = false;
  JSValue click = JS_UNDEFINED, hover = JS_UNDEFINED;
  const char* szText = nullptr;
  wchar_t path[_MAX_FNAME + _MAX_PATH];

  if (argc > 0 && JS_IsString(argv[0])) {
    szText = JS_ToCString(ctx, argv[0]);
    if (!szText) {
      return JS_EXCEPTION;
    }
  }
  if (argc > 1 && JS_IsNumber(argv[1]))
    JS_ToUint32(ctx, &x, argv[1]);
  if (argc > 2 && JS_IsNumber(argv[2]))
    JS_ToUint32(ctx, &y, argv[2]);
  if (argc > 3 && JS_IsNumber(argv[3]))
    JS_ToUint32(ctx, &color, argv[3]);
  if (argc > 4 && JS_IsNumber(argv[4]))
    JS_ToUint32(ctx, &align, argv[4]);
  if (argc > 5 && JS_IsBool(argv[5]))
    automap = !!JS_ToBool(ctx, argv[5]);
  if (argc > 6 && JS_IsFunction(ctx, argv[6]))
    click = argv[6];
  if (argc > 7 && JS_IsFunction(ctx, argv[7]))
    hover = argv[7];

  if (isValidPath(szText)) {
    swprintf_s(path, _countof(path), L"%S", szText);
    JS_FreeCString(ctx, szText);
  } else {
    JS_FreeCString(ctx, szText);
    THROW_ERROR(ctx, "Invalid image file path");
  }

  ImageHook* pImageHook = new ImageHook(script, JS_UNDEFINED, path, x, y, static_cast<ushort>(color), automap, (Align)align, state, true);
  if (!pImageHook)
    THROW_ERROR(ctx, "Failed to create ImageHook");

  JSValue hook = ImageWrap::Instantiate(ctx, new_target, pImageHook);

  pImageHook->SetClickHandler(click);
  pImageHook->SetHoverHandler(hover);

  return hook;
}

// properties
JSValue ImageWrap::GetX(JSContext* ctx, JSValue this_val) {
  ImageWrap* wrap = static_cast<ImageWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  ImageHook* pImageHook = wrap->pImage;
  return JS_NewUint32(ctx, pImageHook->GetX());
}

JSValue ImageWrap::SetX(JSContext* ctx, JSValue this_val, JSValue val) {
  ImageWrap* wrap = static_cast<ImageWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  ImageHook* pImageHook = wrap->pImage;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pImageHook->SetX(ival);
  return JS_UNDEFINED;
}

JSValue ImageWrap::GetY(JSContext* ctx, JSValue this_val) {
  ImageWrap* wrap = static_cast<ImageWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  ImageHook* pImageHook = wrap->pImage;
  return JS_NewUint32(ctx, pImageHook->GetY());
}

JSValue ImageWrap::SetY(JSContext* ctx, JSValue this_val, JSValue val) {
  ImageWrap* wrap = static_cast<ImageWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  ImageHook* pImageHook = wrap->pImage;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pImageHook->SetY(ival);
  return JS_UNDEFINED;
}

JSValue ImageWrap::GetVisible(JSContext* ctx, JSValue this_val) {
  ImageWrap* wrap = static_cast<ImageWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  ImageHook* pImageHook = wrap->pImage;
  return JS_NewBool(ctx, pImageHook->GetIsVisible());
}

JSValue ImageWrap::GetLocation(JSContext* ctx, JSValue this_val) {
  ImageWrap* wrap = static_cast<ImageWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  ImageHook* pImageHook = wrap->pImage;
  return JS_NewString(ctx, pImageHook->GetImage());
}

JSValue ImageWrap::SetLocation(JSContext* ctx, JSValue this_val, JSValue val) {
  ImageWrap* wrap = static_cast<ImageWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  ImageHook* pImageHook = wrap->pImage;
  if (JS_IsString(val)) {
    const char* szText = JS_ToCString(ctx, val);
    if (!szText) {
      return JS_EXCEPTION;
    }
    std::wstring pText = AnsiToWide(szText);
    pImageHook->SetImage(pText.c_str());
    JS_FreeCString(ctx, szText);
  }
  return JS_UNDEFINED;
}

JSValue ImageWrap::SetVisible(JSContext* ctx, JSValue this_val, JSValue val) {
  ImageWrap* wrap = static_cast<ImageWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  ImageHook* pImageHook = wrap->pImage;
  if (!JS_IsBool(val)) {
    return JS_EXCEPTION;
  }
  pImageHook->SetIsVisible(JS_ToBool(ctx, val));
  return JS_UNDEFINED;
}

JSValue ImageWrap::GetAlign(JSContext* ctx, JSValue this_val) {
  ImageWrap* wrap = static_cast<ImageWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  ImageHook* pImageHook = wrap->pImage;
  return JS_NewUint32(ctx, pImageHook->GetAlign());
}

JSValue ImageWrap::SetAlign(JSContext* ctx, JSValue this_val, JSValue val) {
  ImageWrap* wrap = static_cast<ImageWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  ImageHook* pImageHook = wrap->pImage;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pImageHook->SetAlign((Align)ival);
  return JS_UNDEFINED;
}

JSValue ImageWrap::GetZOrder(JSContext* ctx, JSValue this_val) {
  ImageWrap* wrap = static_cast<ImageWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  ImageHook* pImageHook = wrap->pImage;
  return JS_NewUint32(ctx, pImageHook->GetZOrder());
}

JSValue ImageWrap::SetZOrder(JSContext* ctx, JSValue this_val, JSValue val) {
  ImageWrap* wrap = static_cast<ImageWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  ImageHook* pImageHook = wrap->pImage;
  uint32_t ival = 0;
  if (!JS_IsNumber(val) || JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }
  pImageHook->SetZOrder((ushort)ival);
  return JS_UNDEFINED;
}

JSValue ImageWrap::GetClick(JSContext* ctx, JSValue this_val) {
  ImageWrap* wrap = static_cast<ImageWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  ImageHook* pImageHook = wrap->pImage;
  return JS_DupValue(ctx, pImageHook->GetClickHandler());
}

JSValue ImageWrap::SetClick(JSContext* /*ctx*/, JSValue this_val, JSValue val) {
  ImageWrap* wrap = static_cast<ImageWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  ImageHook* pImageHook = wrap->pImage;
  pImageHook->SetClickHandler(val);
  return JS_UNDEFINED;
}

JSValue ImageWrap::GetHover(JSContext* ctx, JSValue this_val) {
  ImageWrap* wrap = static_cast<ImageWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  ImageHook* pImageHook = wrap->pImage;
  return JS_DupValue(ctx, pImageHook->GetHoverHandler());
}

JSValue ImageWrap::SetHover(JSContext* /*ctx*/, JSValue this_val, JSValue val) {
  ImageWrap* wrap = static_cast<ImageWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  ImageHook* pImageHook = wrap->pImage;
  pImageHook->SetHoverHandler(val);
  return JS_UNDEFINED;
}

// functions
JSValue ImageWrap::Remove(JSContext* /*ctx*/, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  ImageWrap* wrap = static_cast<ImageWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  return JS_UNDEFINED;
}

D2BS_BINDING_INTERNAL(ImageWrap, ImageWrap::Initialize)