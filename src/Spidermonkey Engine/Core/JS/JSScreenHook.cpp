#include "JSScreenHook.h"
#include "Engine.h"
#include "ScreenHook.h"
#include "Script.h"
#include "File.h"
#include <Helpers.h>

#include "Bindings.h"

JSObject* FrameWrap::Instantiate(JSContext* ctx) {
  JS::RootedObject global(ctx, JS::CurrentGlobalOrNull(ctx));
  JS::RootedValue constructor_val(ctx);
  if (!JS_GetProperty(ctx, global, "Frame", &constructor_val)) {
    JS_ReportErrorASCII(ctx, "Could not find constructor object for Frame");
    return nullptr;
  }
  if (!constructor_val.isObject()) {
    JS_ReportErrorASCII(ctx, "Frame is not a constructor");
    return nullptr;
  }
  JS::RootedObject constructor(ctx, &constructor_val.toObject());

  JS::RootedObject obj(ctx, JS_New(ctx, constructor, JS::HandleValueArray::empty()));
  if (!obj) {
    JS_ReportErrorASCII(ctx, "Calling Frame constructor failed");
    return nullptr;
  }
  return obj;
}

void FrameWrap::Initialize(JSContext* ctx, JS::HandleObject target) {
  JS::RootedObject proto(ctx, JS_InitClass(ctx, target, nullptr, &m_class, trampoline<New>, 0, m_props, m_methods, nullptr, nullptr));
  if (!proto) {
    return;
  }
}

FrameWrap::FrameWrap(JSContext* ctx, JS::HandleObject obj, FrameHook* frame) : BaseObject(ctx, obj), pFrame(frame) {
}

FrameWrap::~FrameWrap() {
  Genhook::EnterGlobalSection();
  if (pFrame) {
    delete pFrame;
    pFrame = nullptr;
  }
  Genhook::LeaveGlobalSection();
}

void FrameWrap::finalize(JSFreeOp* /*fop*/, JSObject* obj) {
  BaseObject* wrap = BaseObject::FromJSObject(obj);
  if (wrap) {
    delete wrap;
  }
}

bool FrameWrap::New(JSContext* ctx, JS::CallArgs& args) {
  ThreadState* ts = (ThreadState*)JS_GetContextPrivate(ctx);
  Script* script = ts->script;

  uint32_t x = 0, y = 0, x2 = 0, y2 = 0;
  Align align = Left;
  bool automap = false;
  JS::RootedObject click(ctx);
  JS::RootedObject hover(ctx);

  if (args.length() > 0 && args[0].isNumber())
    JS::ToUint32(ctx, args[0], &x);
  if (args.length() > 1 && args[1].isNumber())
    JS::ToUint32(ctx, args[1], &y);
  if (args.length() > 2 && args[2].isNumber())
    JS::ToUint32(ctx, args[2], &x2);
  if (args.length() > 3 && args[3].isNumber())
    JS::ToUint32(ctx, args[3], &y2);
  if (args.length() > 4 && args[4].isNumber())
    JS::ToUint32(ctx, args[4], (uint32_t*)(&align));
  if (args.length() > 5 && args[5].isBoolean())
    automap = args[5].toBoolean();
  if (args.length() > 6 && args[6].isObject() && JS_ObjectIsFunction(ctx, args[6].toObjectOrNull()))
    click.set(args[6].toObjectOrNull());
  if (args.length() > 7 && args[7].isObject() && JS_ObjectIsFunction(ctx, args[7].toObjectOrNull()))
    hover.set(args[7].toObjectOrNull());

  JS::RootedObject newObject(ctx, JS_NewObjectForConstructor(ctx, &m_class, args));
  if (!newObject) {
    THROW_ERROR(ctx, "failed to instantiate frame");
  }
  // framehooks don't work out of game -- they just crash
  FrameHook* pFrameHook = new FrameHook(script, newObject, x, y, x2, y2, automap, align, IG);
  if (!pFrameHook)
    THROW_ERROR(ctx, "Failed to create framehook");

  FrameWrap* wrap = new FrameWrap(ctx, newObject, pFrameHook);
  if (!wrap) {
    JS_ReportOutOfMemory(ctx);
    return false;
  }

  // pFrameHook->SetClickHandler(click);
  // pFrameHook->SetHoverHandler(hover);
  args.rval().setObject(*newObject);
  return true;
}

// properties
bool FrameWrap::GetX(JSContext* ctx, JS::CallArgs& args) {
  FrameWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FrameHook* pFramehook = wrap->pFrame;
  args.rval().setNumber(pFramehook->GetX());
  return true;
}

bool FrameWrap::SetX(JSContext* ctx, JS::CallArgs& args) {
  FrameWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FrameHook* pFramehook = wrap->pFrame;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pFramehook->SetX(ival);
  args.rval().setUndefined();
  return true;
}

bool FrameWrap::GetY(JSContext* ctx, JS::CallArgs& args) {
  FrameWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FrameHook* pFramehook = wrap->pFrame;
  args.rval().setNumber(pFramehook->GetY());
  return true;
}

bool FrameWrap::SetY(JSContext* ctx, JS::CallArgs& args) {
  FrameWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FrameHook* pFramehook = wrap->pFrame;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pFramehook->SetY(ival);
  args.rval().setUndefined();
  return true;
}

bool FrameWrap::GetSizeX(JSContext* ctx, JS::CallArgs& args) {
  FrameWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FrameHook* pFramehook = wrap->pFrame;
  args.rval().setNumber(pFramehook->GetXSize());
  return true;
}

bool FrameWrap::SetSizeX(JSContext* ctx, JS::CallArgs& args) {
  FrameWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FrameHook* pFramehook = wrap->pFrame;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pFramehook->SetXSize(ival);
  args.rval().setUndefined();
  return true;
}

bool FrameWrap::GetSizeY(JSContext* ctx, JS::CallArgs& args) {
  FrameWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FrameHook* pFramehook = wrap->pFrame;
  args.rval().setNumber(pFramehook->GetYSize());
  return true;
}

bool FrameWrap::SetSizeY(JSContext* ctx, JS::CallArgs& args) {
  FrameWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FrameHook* pFramehook = wrap->pFrame;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pFramehook->SetYSize(ival);
  args.rval().setUndefined();
  return true;
}

bool FrameWrap::GetVisible(JSContext* ctx, JS::CallArgs& args) {
  FrameWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FrameHook* pFramehook = wrap->pFrame;
  args.rval().setBoolean(pFramehook->GetIsVisible());
  return true;
}

bool FrameWrap::SetVisible(JSContext* ctx, JS::CallArgs& args) {
  FrameWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FrameHook* pFramehook = wrap->pFrame;
  if (!args[0].isBoolean()) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pFramehook->SetIsVisible(args[0].toBoolean());
  args.rval().setUndefined();
  return true;
}

bool FrameWrap::GetAlign(JSContext* ctx, JS::CallArgs& args) {
  FrameWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FrameHook* pFramehook = wrap->pFrame;
  args.rval().setNumber(static_cast<uint32_t>(pFramehook->GetAlign()));
  return true;
}

bool FrameWrap::SetAlign(JSContext* ctx, JS::CallArgs& args) {
  FrameWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FrameHook* pFramehook = wrap->pFrame;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pFramehook->SetAlign((Align)ival);
  args.rval().setUndefined();
  return true;
}

bool FrameWrap::GetZOrder(JSContext* ctx, JS::CallArgs& args) {
  FrameWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FrameHook* pFramehook = wrap->pFrame;
  args.rval().setNumber(static_cast<uint32_t>(pFramehook->GetZOrder()));
  return true;
}

bool FrameWrap::SetZOrder(JSContext* ctx, JS::CallArgs& args) {
  FrameWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FrameHook* pFramehook = wrap->pFrame;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pFramehook->SetZOrder((ushort)ival);
  args.rval().setUndefined();
  return true;
}

bool FrameWrap::GetClick(JSContext* ctx, JS::CallArgs& args) {
  FrameWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FrameHook* pFramehook = wrap->pFrame;
  JS::RootedObject fun(ctx, pFramehook->GetClickHandler());
  args.rval().setObject(*fun);
  return true;
}

bool FrameWrap::SetClick(JSContext* ctx, JS::CallArgs& args) {
  FrameWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FrameHook* pFramehook = wrap->pFrame;
  JS::RootedObject fun(ctx, args[0].toObjectOrNull());
  pFramehook->SetClickHandler(fun);
  args.rval().setUndefined();
  return true;
}

bool FrameWrap::GetHover(JSContext* ctx, JS::CallArgs& args) {
  FrameWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FrameHook* pFramehook = wrap->pFrame;
  JS::RootedObject fun(ctx, pFramehook->GetHoverHandler());
  args.rval().setObject(*fun);
  return true;
}

bool FrameWrap::SetHover(JSContext* ctx, JS::CallArgs& args) {
  FrameWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FrameHook* pFramehook = wrap->pFrame;
  JS::RootedObject fun(ctx, args[0].toObjectOrNull());
  pFramehook->SetHoverHandler(fun);
  args.rval().setUndefined();
  return true;
}

// functions
bool FrameWrap::Remove(JSContext* ctx, JS::CallArgs& args) {
  FrameWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Genhook::EnterGlobalSection();
  FrameHook* pFramehook = wrap->pFrame;
  if (pFramehook) {
    delete pFramehook;
    wrap->pFrame = nullptr;
  }
  Genhook::LeaveGlobalSection();
  args.rval().setUndefined();
  return true;
}

D2BS_BINDING_INTERNAL(FrameWrap, FrameWrap::Initialize)

JSObject* BoxWrap::Instantiate(JSContext* ctx, BoxHook* box) {
  JS::RootedObject global(ctx, JS::CurrentGlobalOrNull(ctx));
  JS::RootedValue constructor_val(ctx);
  if (!JS_GetProperty(ctx, global, "Box", &constructor_val)) {
    JS_ReportErrorASCII(ctx, "Could not find constructor object for Box");
    return nullptr;
  }
  if (!constructor_val.isObject()) {
    JS_ReportErrorASCII(ctx, "Box is not a constructor");
    return nullptr;
  }
  JS::RootedObject constructor(ctx, &constructor_val.toObject());

  JS::RootedObject obj(ctx, JS_New(ctx, constructor, JS::HandleValueArray::empty()));
  if (!obj) {
    JS_ReportErrorASCII(ctx, "Calling Box constructor failed");
    return nullptr;
  }
  BoxWrap* wrap = new BoxWrap(ctx, obj, box);
  if (!wrap) {
    JS_ReportOutOfMemory(ctx);
    return nullptr;
  }
  return obj;
}

void BoxWrap::Initialize(JSContext* ctx, JS::HandleObject target) {
  JS::RootedObject proto(ctx, JS_InitClass(ctx, target, nullptr, &m_class, trampoline<New>, 0, m_props, m_methods, nullptr, nullptr));
  if (!proto) {
    return;
  }
}

BoxWrap::BoxWrap(JSContext* ctx, JS::HandleObject obj, BoxHook* box) : BaseObject(ctx, obj), pBox(box) {
}

BoxWrap::~BoxWrap() {
  Genhook::EnterGlobalSection();
  if (pBox) {
    delete pBox;
    pBox = nullptr;
  }
  Genhook::LeaveGlobalSection();
}

void BoxWrap::finalize(JSFreeOp* /*fop*/, JSObject* obj) {
  BaseObject* wrap = BaseObject::FromJSObject(obj);
  if (wrap) {
    delete wrap;
  }
}

bool BoxWrap::New(JSContext* ctx, JS::CallArgs& args) {
  ThreadState* ts = (ThreadState*)JS_GetContextPrivate(ctx);
  Script* script = ts->script;

  ScreenhookState state = (script->GetMode() == kScriptModeMenu) ? OOG : IG;
  uint32_t x = 0, y = 0, x2 = 0, y2 = 0, color = 0, opacity = 0, align = Left;
  bool automap = false;
  JS::RootedObject click(ctx);
  JS::RootedObject hover(ctx);

  if (args.length() > 0 && args[0].isNumber())
    JS::ToUint32(ctx, args[0], &x);
  if (args.length() > 1 && args[1].isNumber())
    JS::ToUint32(ctx, args[1], &y);
  if (args.length() > 2 && args[2].isNumber())
    JS::ToUint32(ctx, args[2], &x2);
  if (args.length() > 3 && args[3].isNumber())
    JS::ToUint32(ctx, args[3], &y2);
  if (args.length() > 4 && args[4].isNumber())
    JS::ToUint32(ctx, args[4], &color);
  if (args.length() > 5 && args[5].isNumber())
    JS::ToUint32(ctx, args[5], &opacity);
  if (args.length() > 6 && args[6].isNumber())
    JS::ToUint32(ctx, args[6], &align);
  if (args.length() > 7 && args[7].isBoolean())
    automap = args[7].toBoolean();
  if (args.length() > 8 && args[8].isObject() && JS_ObjectIsFunction(ctx, args[8].toObjectOrNull()))
    click.set(args[8].toObjectOrNull());
  if (args.length() > 9 && args[9].isObject() && JS_ObjectIsFunction(ctx, args[9].toObjectOrNull()))
    hover.set(args[9].toObjectOrNull());

  JS::RootedObject newObject(ctx, JS_NewObjectForConstructor(ctx, &m_class, args));
  if (!newObject) {
    THROW_ERROR(ctx, "failed to instantiate box");
  }
  BoxHook* pBoxHook = new BoxHook(script, newObject, x, y, x2, y2, (ushort)color, (ushort)opacity, automap, (Align)align, state);
  if (!pBoxHook)
    THROW_ERROR(ctx, "Unable to initalize a box class.");

  BoxWrap* wrap = new BoxWrap(ctx, newObject, pBoxHook);
  if (!wrap) {
    JS_ReportOutOfMemory(ctx);
    return false;
  }

  // pBoxHook->SetClickHandler(click);
  // pBoxHook->SetHoverHandler(hover);
  args.rval().setObject(*newObject);
  return true;
}

// properties
bool BoxWrap::GetX(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  BoxHook* pBox = wrap->pBox;
  args.rval().setNumber(pBox->GetX());
  return true;
}

bool BoxWrap::SetX(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  BoxHook* pBox = wrap->pBox;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pBox->SetX(ival);
  args.rval().setUndefined();
  return true;
}

bool BoxWrap::GetY(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  BoxHook* pBox = wrap->pBox;
  args.rval().setNumber(pBox->GetY());
  return true;
}

bool BoxWrap::SetY(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  BoxHook* pBox = wrap->pBox;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pBox->SetY(ival);
  args.rval().setUndefined();
  return true;
}

bool BoxWrap::GetSizeX(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  BoxHook* pBox = wrap->pBox;
  args.rval().setNumber(pBox->GetXSize());
  return true;
}

bool BoxWrap::SetSizeX(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  BoxHook* pBox = wrap->pBox;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pBox->SetXSize(ival);
  args.rval().setUndefined();
  return true;
}

bool BoxWrap::GetSizeY(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  BoxHook* pBox = wrap->pBox;
  args.rval().setNumber(pBox->GetYSize());
  return true;
}

bool BoxWrap::SetSizeY(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  BoxHook* pBox = wrap->pBox;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pBox->SetYSize(ival);
  args.rval().setUndefined();
  return true;
}

bool BoxWrap::GetVisible(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  BoxHook* pBox = wrap->pBox;
  args.rval().setBoolean(pBox->GetIsVisible());
  return true;
}

bool BoxWrap::SetVisible(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  BoxHook* pBox = wrap->pBox;
  if (!args[0].isBoolean()) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pBox->SetIsVisible(args[0].toBoolean());
  args.rval().setUndefined();
  return true;
}

bool BoxWrap::GetAlign(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  BoxHook* pBox = wrap->pBox;
  args.rval().setNumber(static_cast<uint32_t>(pBox->GetAlign()));
  return true;
}

bool BoxWrap::SetAlign(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  BoxHook* pBox = wrap->pBox;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pBox->SetAlign((Align)ival);
  args.rval().setUndefined();
  return true;
}

bool BoxWrap::GetZOrder(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  BoxHook* pBox = wrap->pBox;
  args.rval().setNumber(static_cast<uint32_t>(pBox->GetZOrder()));
  return true;
}

bool BoxWrap::SetZOrder(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  BoxHook* pBox = wrap->pBox;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pBox->SetZOrder((ushort)ival);
  args.rval().setUndefined();
  return true;
}

bool BoxWrap::GetClick(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  BoxHook* pBox = wrap->pBox;
  JS::RootedObject fun(ctx, pBox->GetClickHandler());
  args.rval().setObject(*fun);
  return true;
}

bool BoxWrap::SetClick(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  BoxHook* pBox = wrap->pBox;
  JS::RootedObject fun(ctx, args[0].toObjectOrNull());
  pBox->SetClickHandler(fun);
  args.rval().setUndefined();
  return true;
}

bool BoxWrap::GetHover(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  BoxHook* pBox = wrap->pBox;
  JS::RootedObject fun(ctx, pBox->GetHoverHandler());
  args.rval().setObject(*fun);
  return true;
}

bool BoxWrap::SetHover(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  BoxHook* pBox = wrap->pBox;
  JS::RootedObject fun(ctx, args[0].toObjectOrNull());
  pBox->SetHoverHandler(fun);
  args.rval().setUndefined();
  return true;
}

bool BoxWrap::GetColor(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  BoxHook* pBox = wrap->pBox;
  args.rval().setNumber(static_cast<uint32_t>(pBox->GetColor()));
  return true;
}

bool BoxWrap::SetColor(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  BoxHook* pBox = wrap->pBox;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pBox->SetColor((ushort)ival);
  args.rval().setUndefined();
  return true;
}

bool BoxWrap::GetOpacity(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  BoxHook* pBox = wrap->pBox;
  args.rval().setNumber(static_cast<uint32_t>(pBox->GetOpacity()));
  return true;
}

bool BoxWrap::SetOpacity(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  BoxHook* pBox = wrap->pBox;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pBox->SetOpacity((ushort)ival);
  args.rval().setUndefined();
  return true;
}

// functions
bool BoxWrap::Remove(JSContext* ctx, JS::CallArgs& args) {
  BoxWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Genhook::EnterGlobalSection();
  BoxHook* pBox = wrap->pBox;
  if (pBox) {
    delete pBox;
    wrap->pBox = nullptr;
  }
  Genhook::LeaveGlobalSection();
  args.rval().setUndefined();
  return true;
}

D2BS_BINDING_INTERNAL(BoxWrap, BoxWrap::Initialize)

// Line functions

JSObject* LineWrap::Instantiate(JSContext* ctx, LineHook* line) {
  JS::RootedObject global(ctx, JS::CurrentGlobalOrNull(ctx));
  JS::RootedValue constructor_val(ctx);
  if (!JS_GetProperty(ctx, global, "Line", &constructor_val)) {
    JS_ReportErrorASCII(ctx, "Could not find constructor object for Line");
    return nullptr;
  }
  if (!constructor_val.isObject()) {
    JS_ReportErrorASCII(ctx, "Line is not a constructor");
    return nullptr;
  }
  JS::RootedObject constructor(ctx, &constructor_val.toObject());

  JS::RootedObject obj(ctx, JS_New(ctx, constructor, JS::HandleValueArray::empty()));
  if (!obj) {
    JS_ReportErrorASCII(ctx, "Calling Line constructor failed");
    return nullptr;
  }
  LineWrap* wrap = new LineWrap(ctx, obj, line);
  if (!wrap) {
    JS_ReportOutOfMemory(ctx);
    return nullptr;
  }
  return obj;
}

void LineWrap::Initialize(JSContext* ctx, JS::HandleObject target) {
  JS::RootedObject proto(ctx, JS_InitClass(ctx, target, nullptr, &m_class, trampoline<New>, 0, m_props, m_methods, nullptr, nullptr));
  if (!proto) {
    return;
  }
}

LineWrap::LineWrap(JSContext* ctx, JS::HandleObject obj, LineHook* line) : BaseObject(ctx, obj), pLine(line) {
}

LineWrap::~LineWrap() {
  Genhook::EnterGlobalSection();
  if (pLine) {
    delete pLine;
    pLine = nullptr;
  }
  Genhook::LeaveGlobalSection();
}

void LineWrap::finalize(JSFreeOp* /*fop*/, JSObject* obj) {
  BaseObject* wrap = BaseObject::FromJSObject(obj);
  if (wrap) {
    delete wrap;
  }
}

bool LineWrap::New(JSContext* ctx, JS::CallArgs& args) {
  ThreadState* ts = (ThreadState*)JS_GetContextPrivate(ctx);
  Script* script = ts->script;

  ScreenhookState state = (script->GetMode() == kScriptModeMenu) ? OOG : IG;
  uint32_t x = 0, y = 0, x2 = 0, y2 = 0, color = 0;
  bool automap = false;
  JS::RootedObject click(ctx);
  JS::RootedObject hover(ctx);

  if (args.length() > 0 && args[0].isNumber())
    JS::ToUint32(ctx, args[0], &x);
  if (args.length() > 1 && args[1].isNumber())
    JS::ToUint32(ctx, args[1], &y);
  if (args.length() > 2 && args[2].isNumber())
    JS::ToUint32(ctx, args[2], &x2);
  if (args.length() > 3 && args[3].isNumber())
    JS::ToUint32(ctx, args[3], &y2);
  if (args.length() > 4 && args[4].isNumber())
    JS::ToUint32(ctx, args[4], &color);
  if (args.length() > 5 && args[5].isBoolean())
    automap = args[5].toBoolean();
  if (args.length() > 6 && args[6].isObject() && JS_ObjectIsFunction(ctx, args[6].toObjectOrNull()))
    click.set(args[6].toObjectOrNull());
  if (args.length() > 7 && args[7].isObject() && JS_ObjectIsFunction(ctx, args[7].toObjectOrNull()))
    hover.set(args[7].toObjectOrNull());

  JS::RootedObject newObject(ctx, JS_NewObjectForConstructor(ctx, &m_class, args));
  if (!newObject) {
    THROW_ERROR(ctx, "failed to instantiate line");
  }
  LineHook* pLineHook = new LineHook(script, newObject, x, y, x2, y2, static_cast<ushort>(color), automap, Left, state);
  if (!pLineHook) {
    THROW_ERROR(ctx, "Unable to initalize a line class.");
  }

  LineWrap* wrap = new LineWrap(ctx, newObject, pLineHook);
  if (!wrap) {
    JS_ReportOutOfMemory(ctx);
    return false;
  }

  // pLineHook->SetClickHandler(click);
  // pLineHook->SetHoverHandler(hover);
  args.rval().setObject(*newObject);
  return true;
}

// properties
bool LineWrap::GetX(JSContext* ctx, JS::CallArgs& args) {
  LineWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  LineHook* pLine = wrap->pLine;
  args.rval().setNumber(pLine->GetX());
  return true;
}

bool LineWrap::SetX(JSContext* ctx, JS::CallArgs& args) {
  LineWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  LineHook* pLine = wrap->pLine;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pLine->SetX(ival);
  args.rval().setUndefined();
  return true;
}

bool LineWrap::GetY(JSContext* ctx, JS::CallArgs& args) {
  LineWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  LineHook* pLine = wrap->pLine;
  args.rval().setNumber(pLine->GetY());
  return true;
}

bool LineWrap::SetY(JSContext* ctx, JS::CallArgs& args) {
  LineWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  LineHook* pLine = wrap->pLine;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pLine->SetY(ival);
  args.rval().setUndefined();
  return true;
}

bool LineWrap::GetX2(JSContext* ctx, JS::CallArgs& args) {
  LineWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  LineHook* pLine = wrap->pLine;
  args.rval().setNumber(pLine->GetX2());
  return true;
}

bool LineWrap::SetX2(JSContext* ctx, JS::CallArgs& args) {
  LineWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  LineHook* pLine = wrap->pLine;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pLine->SetX2(ival);
  args.rval().setUndefined();
  return true;
}

bool LineWrap::GetY2(JSContext* ctx, JS::CallArgs& args) {
  LineWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  LineHook* pLine = wrap->pLine;
  args.rval().setNumber(pLine->GetY2());
  return true;
}

bool LineWrap::SetY2(JSContext* ctx, JS::CallArgs& args) {
  LineWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  LineHook* pLine = wrap->pLine;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pLine->SetY2(ival);
  args.rval().setUndefined();
  return true;
}

bool LineWrap::GetVisible(JSContext* ctx, JS::CallArgs& args) {
  LineWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  LineHook* pLine = wrap->pLine;
  args.rval().setBoolean(pLine->GetIsVisible());
  return true;
}

bool LineWrap::SetVisible(JSContext* ctx, JS::CallArgs& args) {
  LineWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  LineHook* pLine = wrap->pLine;
  if (!args[0].isBoolean()) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pLine->SetIsVisible(args[0].toBoolean());
  args.rval().setUndefined();
  return true;
}

bool LineWrap::GetColor(JSContext* ctx, JS::CallArgs& args) {
  LineWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  LineHook* pLine = wrap->pLine;
  args.rval().setNumber(static_cast<uint32_t>(pLine->GetColor()));
  return true;
}

bool LineWrap::SetColor(JSContext* ctx, JS::CallArgs& args) {
  LineWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  LineHook* pLine = wrap->pLine;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pLine->SetColor(static_cast<ushort>(ival));
  args.rval().setUndefined();
  return true;
}

bool LineWrap::GetZOrder(JSContext* ctx, JS::CallArgs& args) {
  LineWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  LineHook* pLine = wrap->pLine;
  args.rval().setNumber(static_cast<uint32_t>(pLine->GetZOrder()));
  return true;
}

bool LineWrap::SetZOrder(JSContext* ctx, JS::CallArgs& args) {
  LineWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  LineHook* pLine = wrap->pLine;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pLine->SetZOrder((ushort)ival);
  args.rval().setUndefined();
  return true;
}

bool LineWrap::GetClick(JSContext* ctx, JS::CallArgs& args) {
  LineWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  LineHook* pLine = wrap->pLine;
  JS::RootedObject fun(ctx, pLine->GetClickHandler());
  args.rval().setObject(*fun);
  return true;
}

bool LineWrap::SetClick(JSContext* ctx, JS::CallArgs& args) {
  LineWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  LineHook* pLine = wrap->pLine;
  JS::RootedObject fun(ctx, args[0].toObjectOrNull());
  pLine->SetClickHandler(fun);
  args.rval().setUndefined();
  return true;
}

bool LineWrap::GetHover(JSContext* ctx, JS::CallArgs& args) {
  LineWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  LineHook* pLine = wrap->pLine;
  JS::RootedObject fun(ctx, pLine->GetHoverHandler());
  args.rval().setObject(*fun);
  return true;
}

bool LineWrap::SetHover(JSContext* ctx, JS::CallArgs& args) {
  LineWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  LineHook* pLine = wrap->pLine;
  JS::RootedObject fun(ctx, args[0].toObjectOrNull());
  pLine->SetHoverHandler(fun);
  args.rval().setUndefined();
  return true;
}

// functions
bool LineWrap::Remove(JSContext* ctx, JS::CallArgs& args) {
  LineWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Genhook::EnterGlobalSection();
  LineHook* pLine = wrap->pLine;
  if (pLine) {
    delete pLine;
    wrap->pLine = nullptr;
  }
  Genhook::LeaveGlobalSection();
  args.rval().setUndefined();
  return true;
}

D2BS_BINDING_INTERNAL(LineWrap, LineWrap::Initialize)

JSObject* TextWrap::Instantiate(JSContext* ctx, TextHook* text) {
  JS::RootedObject global(ctx, JS::CurrentGlobalOrNull(ctx));
  JS::RootedValue constructor_val(ctx);
  if (!JS_GetProperty(ctx, global, "Text", &constructor_val)) {
    JS_ReportErrorASCII(ctx, "Could not find constructor object for Text");
    return nullptr;
  }
  if (!constructor_val.isObject()) {
    JS_ReportErrorASCII(ctx, "Text is not a constructor");
    return nullptr;
  }
  JS::RootedObject constructor(ctx, &constructor_val.toObject());

  JS::RootedObject obj(ctx, JS_New(ctx, constructor, JS::HandleValueArray::empty()));
  if (!obj) {
    JS_ReportErrorASCII(ctx, "Calling Text constructor failed");
    return nullptr;
  }
  TextWrap* wrap = new TextWrap(ctx, obj, text);
  if (!wrap) {
    JS_ReportOutOfMemory(ctx);
    return nullptr;
  }
  return obj;
}

void TextWrap::Initialize(JSContext* ctx, JS::HandleObject target) {
  JS::RootedObject proto(ctx, JS_InitClass(ctx, target, nullptr, &m_class, trampoline<New>, 0, m_props, m_methods, nullptr, nullptr));
  if (!proto) {
    return;
  }
}

TextWrap::TextWrap(JSContext* ctx, JS::HandleObject obj, TextHook* text) : BaseObject(ctx, obj), pText(text) {
}

TextWrap::~TextWrap() {
  Genhook::EnterGlobalSection();
  if (pText) {
    delete pText;
    pText = nullptr;
  }
  Genhook::LeaveGlobalSection();
}

void TextWrap::finalize(JSFreeOp* /*fop*/, JSObject* obj) {
  BaseObject* wrap = BaseObject::FromJSObject(obj);
  if (wrap) {
    delete wrap;
  }
}

bool TextWrap::New(JSContext* ctx, JS::CallArgs& args) {
  ThreadState* ts = (ThreadState*)JS_GetContextPrivate(ctx);
  Script* script = ts->script;

  ScreenhookState state = (script->GetMode() == kScriptModeMenu) ? OOG : IG;
  uint32_t x = 0, y = 0, color = 0, font = 0, align = Left;
  bool automap = false;
  JS::RootedObject click(ctx);
  JS::RootedObject hover(ctx);
  std::wstring szText;

  if (args.length() > 0 && args[0].isString()) {
    char* str = JS_EncodeString(ctx, args[0].toString());
    if (!str) {
      THROW_ERROR(ctx, "failed to encode string");
    }
    szText = AnsiToWide(str);
    JS_free(ctx, str);
  }
  if (args.length() > 1 && args[1].isNumber())
    JS::ToUint32(ctx, args[1], &x);
  if (args.length() > 2 && args[2].isNumber())
    JS::ToUint32(ctx, args[2], &y);
  if (args.length() > 3 && args[3].isNumber())
    JS::ToUint32(ctx, args[3], &color);
  if (args.length() > 4 && args[4].isNumber())
    JS::ToUint32(ctx, args[4], &font);
  if (args.length() > 5 && args[5].isNumber())
    JS::ToUint32(ctx, args[5], &align);
  if (args.length() > 6 && args[6].isBoolean())
    automap = args[6].toBoolean();
  if (args.length() > 7 && args[7].isObject() && JS_ObjectIsFunction(ctx, args[7].toObjectOrNull()))
    click.set(args[7].toObjectOrNull());
  if (args.length() > 8 && args[8].isObject() && JS_ObjectIsFunction(ctx, args[8].toObjectOrNull()))
    hover.set(args[8].toObjectOrNull());

  JS::RootedObject newObject(ctx, JS_NewObjectForConstructor(ctx, &m_class, args));
  if (!newObject) {
    THROW_ERROR(ctx, "failed to instantiate text");
  }
  TextHook* pTextHook = new TextHook(script, newObject, szText.c_str(), x, y, static_cast<ushort>(font), static_cast<ushort>(color), automap, (Align)align, state);
  if (!pTextHook) {
    THROW_ERROR(ctx, "Failed to create texthook");
  }

  TextWrap* wrap = new TextWrap(ctx, newObject, pTextHook);
  if (!wrap) {
    JS_ReportOutOfMemory(ctx);
    return false;
  }

  // pFrameHook->SetClickHandler(click);
  // pFrameHook->SetHoverHandler(hover);
  args.rval().setObject(*newObject);
  return true;
}

// properties
bool TextWrap::GetX(JSContext* ctx, JS::CallArgs& args) {
  TextWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  TextHook* pText = wrap->pText;
  args.rval().setNumber(pText->GetX());
  return true;
}

bool TextWrap::SetX(JSContext* ctx, JS::CallArgs& args) {
  TextWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  TextHook* pText = wrap->pText;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pText->SetX(ival);
  args.rval().setUndefined();
  return true;
}

bool TextWrap::GetY(JSContext* ctx, JS::CallArgs& args) {
  TextWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  TextHook* pText = wrap->pText;
  args.rval().setNumber(pText->GetY());
  return true;
}

bool TextWrap::SetY(JSContext* ctx, JS::CallArgs& args) {
  TextWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  TextHook* pText = wrap->pText;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pText->SetY(ival);
  args.rval().setUndefined();
  return true;
}

bool TextWrap::GetColor(JSContext* ctx, JS::CallArgs& args) {
  TextWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  TextHook* pText = wrap->pText;
  args.rval().setNumber(static_cast<uint32_t>(pText->GetColor()));
  return true;
}

bool TextWrap::SetColor(JSContext* ctx, JS::CallArgs& args) {
  TextWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  TextHook* pText = wrap->pText;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pText->SetColor((ushort)ival);
  args.rval().setUndefined();
  return true;
}

bool TextWrap::GetVisible(JSContext* ctx, JS::CallArgs& args) {
  TextWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  TextHook* pText = wrap->pText;
  args.rval().setBoolean(pText->GetIsVisible());
  return true;
}

bool TextWrap::SetVisible(JSContext* ctx, JS::CallArgs& args) {
  TextWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  TextHook* pText = wrap->pText;
  if (!args[0].isBoolean()) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pText->SetIsVisible(args[0].toBoolean());
  args.rval().setUndefined();
  return true;
}

bool TextWrap::GetAlign(JSContext* ctx, JS::CallArgs& args) {
  TextWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  TextHook* pText = wrap->pText;
  args.rval().setNumber(static_cast<uint32_t>(pText->GetAlign()));
  return true;
}

bool TextWrap::SetAlign(JSContext* ctx, JS::CallArgs& args) {
  TextWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  TextHook* pText = wrap->pText;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pText->SetAlign((Align)ival);
  args.rval().setUndefined();
  return true;
}

bool TextWrap::GetZOrder(JSContext* ctx, JS::CallArgs& args) {
  TextWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  TextHook* pText = wrap->pText;
  args.rval().setNumber(static_cast<uint32_t>(pText->GetZOrder()));
  return true;
}

bool TextWrap::SetZOrder(JSContext* ctx, JS::CallArgs& args) {
  TextWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  TextHook* pText = wrap->pText;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pText->SetZOrder((ushort)ival);
  args.rval().setUndefined();
  return true;
}

bool TextWrap::GetClick(JSContext* ctx, JS::CallArgs& args) {
  TextWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  TextHook* pText = wrap->pText;
  JS::RootedObject fun(ctx, pText->GetClickHandler());
  args.rval().setObject(*fun);
  return true;
}

bool TextWrap::SetClick(JSContext* ctx, JS::CallArgs& args) {
  TextWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  TextHook* pText = wrap->pText;
  JS::RootedObject fun(ctx, args[0].toObjectOrNull());
  pText->SetClickHandler(fun);
  args.rval().setUndefined();
  return true;
}

bool TextWrap::GetHover(JSContext* ctx, JS::CallArgs& args) {
  TextWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  TextHook* pText = wrap->pText;
  JS::RootedObject fun(ctx, pText->GetHoverHandler());
  args.rval().setObject(*fun);
  return true;
}

bool TextWrap::SetHover(JSContext* ctx, JS::CallArgs& args) {
  TextWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  TextHook* pText = wrap->pText;
  JS::RootedObject fun(ctx, args[0].toObjectOrNull());
  pText->SetHoverHandler(fun);
  args.rval().setUndefined();
  return true;
}

bool TextWrap::GetFont(JSContext* ctx, JS::CallArgs& args) {
  TextWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  TextHook* pText = wrap->pText;
  args.rval().setNumber(static_cast<uint32_t>(pText->GetFont()));
  return true;
}

bool TextWrap::SetFont(JSContext* ctx, JS::CallArgs& args) {
  TextWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  TextHook* pText = wrap->pText;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pText->SetFont((ushort)ival);
  args.rval().setUndefined();
  return true;
}

bool TextWrap::GetText(JSContext* ctx, JS::CallArgs& args) {
  TextWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  TextHook* pText = wrap->pText;
  args.rval().setString(JS_NewUCStringCopyZ(ctx, reinterpret_cast<const char16_t*>(pText->GetText())));
  return true;
}

bool TextWrap::SetText(JSContext* ctx, JS::CallArgs& args) {
  TextWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  TextHook* pTextHook = wrap->pText;
  if (args[0].isString()) {
    char* szText = JS_EncodeString(ctx, args[0].toString());
    if (!szText) {
      THROW_ERROR(ctx, "failed to encode string");
    }
    std::wstring pText = AnsiToWide(szText);
    pTextHook->SetText(pText.c_str());
    JS_free(ctx, szText);
  }
  args.rval().setUndefined();
  return true;
}

// functions
bool TextWrap::Remove(JSContext* ctx, JS::CallArgs& args) {
  TextWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Genhook::EnterGlobalSection();
  TextHook* pText = wrap->pText;
  if (pText) {
    delete pText;
    wrap->pText = nullptr;
  }
  Genhook::LeaveGlobalSection();
  args.rval().setUndefined();
  return true;
}

D2BS_BINDING_INTERNAL(TextWrap, TextWrap::Initialize)

// Function to create a image which gets called on a "new Image ()"

JSObject* ImageWrap::Instantiate(JSContext* ctx, ImageHook* image) {
  JS::RootedObject global(ctx, JS::CurrentGlobalOrNull(ctx));
  JS::RootedValue constructor_val(ctx);
  if (!JS_GetProperty(ctx, global, "Image", &constructor_val)) {
    JS_ReportErrorASCII(ctx, "Could not find constructor object for Image");
    return nullptr;
  }
  if (!constructor_val.isObject()) {
    JS_ReportErrorASCII(ctx, "Image is not a constructor");
    return nullptr;
  }
  JS::RootedObject constructor(ctx, &constructor_val.toObject());

  JS::RootedObject obj(ctx, JS_New(ctx, constructor, JS::HandleValueArray::empty()));
  if (!obj) {
    JS_ReportErrorASCII(ctx, "Calling Image constructor failed");
    return nullptr;
  }
  ImageWrap* wrap = new ImageWrap(ctx, obj, image);
  if (!wrap) {
    JS_ReportOutOfMemory(ctx);
    return nullptr;
  }
  return obj;
}

void ImageWrap::Initialize(JSContext* ctx, JS::HandleObject target) {
  JS::RootedObject proto(ctx, JS_InitClass(ctx, target, nullptr, &m_class, trampoline<New>, 0, m_props, m_methods, nullptr, nullptr));
  if (!proto) {
    return;
  }
}

ImageWrap::ImageWrap(JSContext* ctx, JS::HandleObject obj, ImageHook* image) : BaseObject(ctx, obj), pImage(image) {
}

ImageWrap::~ImageWrap() {
  Genhook::EnterGlobalSection();
  if (pImage) {
    delete pImage;
    pImage = nullptr;
  }
  Genhook::LeaveGlobalSection();
}

void ImageWrap::finalize(JSFreeOp* /*fop*/, JSObject* obj) {
  BaseObject* wrap = BaseObject::FromJSObject(obj);
  if (wrap) {
    delete wrap;
  }
}

bool ImageWrap::New(JSContext* ctx, JS::CallArgs& args) {
  ThreadState* ts = (ThreadState*)JS_GetContextPrivate(ctx);
  Script* script = ts->script;

  ScreenhookState state = (script->GetMode() == kScriptModeMenu) ? OOG : IG;
  uint32_t x = 0, y = 0, color = 0, align = Left;
  bool automap = false;
  JS::RootedObject click(ctx);
  JS::RootedObject hover(ctx);
  char* szText = nullptr;
  wchar_t path[_MAX_FNAME + _MAX_PATH];

  if (args.length() > 0 && args[0].isString()) {
    char* str = JS_EncodeString(ctx, args[0].toString());
    if (!str) {
      THROW_ERROR(ctx, "failed to encode string");
    }
  }
  if (args.length() > 1 && args[1].isNumber())
    JS::ToUint32(ctx, args[1], &x);
  if (args.length() > 2 && args[2].isNumber())
    JS::ToUint32(ctx, args[2], &y);
  if (args.length() > 3 && args[3].isNumber())
    JS::ToUint32(ctx, args[3], &color);
  if (args.length() > 4 && args[4].isNumber())
    JS::ToUint32(ctx, args[4], &align);
  if (args.length() > 5 && args[5].isBoolean())
    automap = args[5].toBoolean();
  if (args.length() > 6 && args[6].isObject() && JS_ObjectIsFunction(ctx, args[6].toObjectOrNull()))
    click.set(args[6].toObjectOrNull());
  if (args.length() > 7 && args[7].isObject() && JS_ObjectIsFunction(ctx, args[7].toObjectOrNull()))
    hover.set(args[7].toObjectOrNull());

  if (isValidPath(szText)) {
    swprintf_s(path, _countof(path), L"%S", szText);
    JS_free(ctx, szText);
  } else {
    JS_free(ctx, szText);
    THROW_ERROR(ctx, "Invalid image file path");
  }

  JS::RootedObject newObject(ctx, JS_NewObjectForConstructor(ctx, &m_class, args));
  if (!newObject) {
    THROW_ERROR(ctx, "failed to instantiate image");
  }
  ImageHook* pImageHook = new ImageHook(script, newObject, path, x, y, static_cast<ushort>(color), automap, (Align)align, state, true);
  if (!pImageHook)
    THROW_ERROR(ctx, "Failed to create ImageHook");

  ImageWrap* wrap = new ImageWrap(ctx, newObject, pImageHook);
  if (!wrap) {
    JS_ReportOutOfMemory(ctx);
    return false;
  }

  // pFrameHook->SetClickHandler(click);
  // pFrameHook->SetHoverHandler(hover);
  args.rval().setObject(*newObject);
  return true;
}

// properties
bool ImageWrap::GetX(JSContext* ctx, JS::CallArgs& args) {
  ImageWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  ImageHook* pImageHook = wrap->pImage;
  args.rval().setNumber(pImageHook->GetX());
  return true;
}

bool ImageWrap::SetX(JSContext* ctx, JS::CallArgs& args) {
  ImageWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  ImageHook* pImageHook = wrap->pImage;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pImageHook->SetX(ival);
  args.rval().setUndefined();
  return true;
}

bool ImageWrap::GetY(JSContext* ctx, JS::CallArgs& args) {
  ImageWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  ImageHook* pImageHook = wrap->pImage;
  args.rval().setNumber(pImageHook->GetY());
  return true;
}

bool ImageWrap::SetY(JSContext* ctx, JS::CallArgs& args) {
  ImageWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  ImageHook* pImageHook = wrap->pImage;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pImageHook->SetY(ival);
  args.rval().setUndefined();
  return true;
}

bool ImageWrap::GetVisible(JSContext* ctx, JS::CallArgs& args) {
  ImageWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  ImageHook* pImageHook = wrap->pImage;
  args.rval().setBoolean(pImageHook->GetIsVisible());
  return true;
}

bool ImageWrap::GetLocation(JSContext* ctx, JS::CallArgs& args) {
  ImageWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  ImageHook* pImageHook = wrap->pImage;
  args.rval().setString(JS_NewUCStringCopyZ(ctx, reinterpret_cast<const char16_t*>(pImageHook->GetImage())));
  return true;
}

bool ImageWrap::SetLocation(JSContext* ctx, JS::CallArgs& args) {
  ImageWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  ImageHook* pImageHook = wrap->pImage;
  if (args[0].isString()) {
    char* szText = JS_EncodeString(ctx, args[0].toString());
    if (!szText) {
      THROW_ERROR(ctx, "failed to encode string");
    }
    std::wstring pText = AnsiToWide(szText);
    pImageHook->SetImage(pText.c_str());
    JS_free(ctx, szText);
  }
  args.rval().setUndefined();
  return true;
}

bool ImageWrap::SetVisible(JSContext* ctx, JS::CallArgs& args) {
  ImageWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  ImageHook* pImageHook = wrap->pImage;
  if (!args[0].isBoolean()) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pImageHook->SetIsVisible(args[0].toBoolean());
  args.rval().setUndefined();
  return true;
}

bool ImageWrap::GetAlign(JSContext* ctx, JS::CallArgs& args) {
  ImageWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  ImageHook* pImageHook = wrap->pImage;
  args.rval().setNumber(static_cast<uint32_t>(pImageHook->GetAlign()));
  return true;
}

bool ImageWrap::SetAlign(JSContext* ctx, JS::CallArgs& args) {
  ImageWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  ImageHook* pImageHook = wrap->pImage;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pImageHook->SetAlign((Align)ival);
  args.rval().setUndefined();
  return true;
}

bool ImageWrap::GetZOrder(JSContext* ctx, JS::CallArgs& args) {
  ImageWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  ImageHook* pImageHook = wrap->pImage;
  args.rval().setNumber(static_cast<uint32_t>(pImageHook->GetZOrder()));
  return true;
}

bool ImageWrap::SetZOrder(JSContext* ctx, JS::CallArgs& args) {
  ImageWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  ImageHook* pImageHook = wrap->pImage;
  uint32_t ival = 0;
  if (!args[0].isNumber() || !JS::ToUint32(ctx, args[0], &ival)) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  pImageHook->SetZOrder(static_cast<ushort>(ival));
  args.rval().setUndefined();
  return true;
}

bool ImageWrap::GetClick(JSContext* ctx, JS::CallArgs& args) {
  ImageWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  ImageHook* pImageHook = wrap->pImage;
  JS::RootedObject fun(ctx, pImageHook->GetClickHandler());
  args.rval().setObject(*fun);
  return true;
}

bool ImageWrap::SetClick(JSContext* ctx, JS::CallArgs& args) {
  ImageWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  ImageHook* pImageHook = wrap->pImage;
  JS::RootedObject fun(ctx, args[0].toObjectOrNull());
  pImageHook->SetClickHandler(fun);
  args.rval().setUndefined();
  return true;
}

bool ImageWrap::GetHover(JSContext* ctx, JS::CallArgs& args) {
  ImageWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  ImageHook* pImageHook = wrap->pImage;
  JS::RootedObject fun(ctx, pImageHook->GetHoverHandler());
  args.rval().setObject(*fun);
  return true;
}

bool ImageWrap::SetHover(JSContext* ctx, JS::CallArgs& args) {
  ImageWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  ImageHook* pImageHook = wrap->pImage;
  JS::RootedObject fun(ctx, args[0].toObjectOrNull());
  pImageHook->SetHoverHandler(fun);
  args.rval().setUndefined();
  return true;
}

// functions
bool ImageWrap::Remove(JSContext* ctx, JS::CallArgs& args) {
  ImageWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Genhook::EnterGlobalSection();
  ImageHook* pImageHook = wrap->pImage;
  if (pImageHook) {
    delete pImageHook;
    wrap->pImage = nullptr;
  }
  Genhook::LeaveGlobalSection();
  args.rval().setUndefined();
  return true;
}

D2BS_BINDING_INTERNAL(ImageWrap, ImageWrap::Initialize)
