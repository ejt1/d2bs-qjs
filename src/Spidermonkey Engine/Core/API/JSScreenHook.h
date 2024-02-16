#ifndef __JSSCREENHOOK_H__
#define __JSSCREENHOOK_H__

#include "JSBaseObject.h"
#include "ScreenHook.h"

// TODO(ejt): the whole gui part must be rewritten from the ground up otherwise
// I would put effort into making the wrappers be derivative using a base class
// instead of redefining everything

/*********************************************************
                                        frame Header
**********************************************************/

class FrameWrap : public BaseObject {
 public:
  static JSObject* Instantiate(JSContext* ctx);
  static void Initialize(JSContext* ctx, JS::HandleObject target);

 private:
  FrameWrap(JSContext* ctx, JS::HandleObject obj, FrameHook* frame);
  virtual ~FrameWrap();

  static void finalize(JSFreeOp* fop, JSObject* obj);

  // constructor
  static bool New(JSContext* ctx, JS::CallArgs& args);

  // properties
  static bool GetX(JSContext* ctx, JS::CallArgs& args);
  static bool SetX(JSContext* ctx, JS::CallArgs& args);
  static bool GetY(JSContext* ctx, JS::CallArgs& args);
  static bool SetY(JSContext* ctx, JS::CallArgs& args);
  static bool GetSizeX(JSContext* ctx, JS::CallArgs& args);
  static bool SetSizeX(JSContext* ctx, JS::CallArgs& args);
  static bool GetSizeY(JSContext* ctx, JS::CallArgs& args);
  static bool SetSizeY(JSContext* ctx, JS::CallArgs& args);
  static bool GetVisible(JSContext* ctx, JS::CallArgs& args);
  static bool SetVisible(JSContext* ctx, JS::CallArgs& args);
  static bool GetAlign(JSContext* ctx, JS::CallArgs& args);
  static bool SetAlign(JSContext* ctx, JS::CallArgs& args);
  static bool GetZOrder(JSContext* ctx, JS::CallArgs& args);
  static bool SetZOrder(JSContext* ctx, JS::CallArgs& args);
  static bool GetClick(JSContext* ctx, JS::CallArgs& args);
  static bool SetClick(JSContext* ctx, JS::CallArgs& args);
  static bool GetHover(JSContext* ctx, JS::CallArgs& args);
  static bool SetHover(JSContext* ctx, JS::CallArgs& args);

  // functions
  static bool Remove(JSContext* ctx, JS::CallArgs& args);

  static inline JSClassOps m_ops = {
      .addProperty = nullptr,
      .delProperty = nullptr,
      .enumerate = nullptr,
      .newEnumerate = nullptr,
      .resolve = nullptr,
      .mayResolve = nullptr,
      .finalize = finalize,
      .call = nullptr,
      .hasInstance = nullptr,
      .construct = nullptr,
      .trace = nullptr,
  };
  static inline JSClass m_class = {
      "Frame",
      JSCLASS_HAS_RESERVED_SLOTS(kInternalFieldCount) | JSCLASS_FOREGROUND_FINALIZE,
      &m_ops,
  };
  static inline JSPropertySpec m_props[] = {
      JS_PSGS("x", trampoline<GetX>, trampoline<SetX>, JSPROP_ENUMERATE),
      JS_PSGS("y", trampoline<GetY>, trampoline<SetY>, JSPROP_ENUMERATE),
      JS_PSGS("xsize", trampoline<GetSizeX>, trampoline<SetSizeX>, JSPROP_ENUMERATE),
      JS_PSGS("ysize", trampoline<GetSizeY>, trampoline<SetSizeY>, JSPROP_ENUMERATE),
      JS_PSGS("visible", trampoline<GetVisible>, trampoline<SetVisible>, JSPROP_ENUMERATE),
      JS_PSGS("align", trampoline<GetAlign>, trampoline<SetAlign>, JSPROP_ENUMERATE),
      JS_PSGS("zorder", trampoline<GetZOrder>, trampoline<SetZOrder>, JSPROP_ENUMERATE),
      JS_PSGS("click", trampoline<GetClick>, trampoline<SetClick>, JSPROP_ENUMERATE),
      JS_PSGS("hover", trampoline<GetHover>, trampoline<SetHover>, JSPROP_ENUMERATE),
      JS_PS_END,
  };
  static inline JSFunctionSpec m_methods[] = {
      JS_FN("remove", trampoline<Remove>, 0, JSPROP_ENUMERATE),
      JS_FS_END,
  };

  FrameHook* pFrame;
};

/*********************************************************
                                        box Header
**********************************************************/

class BoxWrap : public BaseObject {
 public:
  static JSObject* Instantiate(JSContext* ctx, BoxHook* frame);
  static void Initialize(JSContext* ctx, JS::HandleObject target);

 private:
  BoxWrap(JSContext* ctx, JS::HandleObject obj, BoxHook* box);
  virtual ~BoxWrap();

  static void finalize(JSFreeOp* fop, JSObject* obj);

  // constructor
  static bool New(JSContext* ctx, JS::CallArgs& args);

  // properties
  static bool GetX(JSContext* ctx, JS::CallArgs& args);
  static bool SetX(JSContext* ctx, JS::CallArgs& args);
  static bool GetY(JSContext* ctx, JS::CallArgs& args);
  static bool SetY(JSContext* ctx, JS::CallArgs& args);
  static bool GetSizeX(JSContext* ctx, JS::CallArgs& args);
  static bool SetSizeX(JSContext* ctx, JS::CallArgs& args);
  static bool GetSizeY(JSContext* ctx, JS::CallArgs& args);
  static bool SetSizeY(JSContext* ctx, JS::CallArgs& args);
  static bool GetVisible(JSContext* ctx, JS::CallArgs& args);
  static bool SetVisible(JSContext* ctx, JS::CallArgs& args);
  static bool GetAlign(JSContext* ctx, JS::CallArgs& args);
  static bool SetAlign(JSContext* ctx, JS::CallArgs& args);
  static bool GetZOrder(JSContext* ctx, JS::CallArgs& args);
  static bool SetZOrder(JSContext* ctx, JS::CallArgs& args);
  static bool GetClick(JSContext* ctx, JS::CallArgs& args);
  static bool SetClick(JSContext* ctx, JS::CallArgs& args);
  static bool GetHover(JSContext* ctx, JS::CallArgs& args);
  static bool SetHover(JSContext* ctx, JS::CallArgs& args);

  static bool GetColor(JSContext* ctx, JS::CallArgs& args);
  static bool SetColor(JSContext* ctx, JS::CallArgs& args);
  static bool GetOpacity(JSContext* ctx, JS::CallArgs& args);
  static bool SetOpacity(JSContext* ctx, JS::CallArgs& args);

  // functions
  static bool Remove(JSContext* ctx, JS::CallArgs& args);

  static inline JSClassOps m_ops = {
      .addProperty = nullptr,
      .delProperty = nullptr,
      .enumerate = nullptr,
      .newEnumerate = nullptr,
      .resolve = nullptr,
      .mayResolve = nullptr,
      .finalize = finalize,
      .call = nullptr,
      .hasInstance = nullptr,
      .construct = nullptr,
      .trace = nullptr,
  };
  static inline JSClass m_class = {
      "Box",
      JSCLASS_HAS_RESERVED_SLOTS(kInternalFieldCount) | JSCLASS_FOREGROUND_FINALIZE,
      &m_ops,
  };
  static inline JSPropertySpec m_props[] = {
      JS_PSGS("x", trampoline<GetX>, trampoline<SetX>, JSPROP_ENUMERATE),
      JS_PSGS("y", trampoline<GetY>, trampoline<SetY>, JSPROP_ENUMERATE),
      JS_PSGS("xsize", trampoline<GetSizeX>, trampoline<SetSizeX>, JSPROP_ENUMERATE),
      JS_PSGS("ysize", trampoline<GetSizeY>, trampoline<SetSizeY>, JSPROP_ENUMERATE),
      JS_PSGS("visible", trampoline<GetVisible>, trampoline<SetVisible>, JSPROP_ENUMERATE),
      JS_PSGS("align", trampoline<GetAlign>, trampoline<SetAlign>, JSPROP_ENUMERATE),
      JS_PSGS("zorder", trampoline<GetZOrder>, trampoline<SetZOrder>, JSPROP_ENUMERATE),
      JS_PSGS("click", trampoline<GetClick>, trampoline<SetClick>, JSPROP_ENUMERATE),
      JS_PSGS("hover", trampoline<GetHover>, trampoline<SetHover>, JSPROP_ENUMERATE),

      JS_PSGS("color", trampoline<GetColor>, trampoline<SetColor>, JSPROP_ENUMERATE),
      JS_PSGS("opacity", trampoline<GetOpacity>, trampoline<SetOpacity>, JSPROP_ENUMERATE),
      JS_PS_END,
  };
  static inline JSFunctionSpec m_methods[] = {
      JS_FN("remove", trampoline<Remove>, 0, JSPROP_ENUMERATE),
      JS_FS_END,
  };

  BoxHook* pBox;
};

/*********************************************************
                                        Line Header
**********************************************************/

class LineWrap : public BaseObject {
 public:
  static JSObject* Instantiate(JSContext* ctx, LineHook* frame);
  static void Initialize(JSContext* ctx, JS::HandleObject target);

 private:
  LineWrap(JSContext* ctx, JS::HandleObject obj, LineHook* line);
  virtual ~LineWrap();

  static void finalize(JSFreeOp* fop, JSObject* obj);

  // constructor
  static bool New(JSContext* ctx, JS::CallArgs& args);

  // properties
  static bool GetX(JSContext* ctx, JS::CallArgs& args);
  static bool SetX(JSContext* ctx, JS::CallArgs& args);
  static bool GetY(JSContext* ctx, JS::CallArgs& args);
  static bool SetY(JSContext* ctx, JS::CallArgs& args);
  static bool GetX2(JSContext* ctx, JS::CallArgs& args);
  static bool SetX2(JSContext* ctx, JS::CallArgs& args);
  static bool GetY2(JSContext* ctx, JS::CallArgs& args);
  static bool SetY2(JSContext* ctx, JS::CallArgs& args);
  static bool GetVisible(JSContext* ctx, JS::CallArgs& args);
  static bool SetVisible(JSContext* ctx, JS::CallArgs& args);
  static bool GetColor(JSContext* ctx, JS::CallArgs& args);
  static bool SetColor(JSContext* ctx, JS::CallArgs& args);
  static bool GetZOrder(JSContext* ctx, JS::CallArgs& args);
  static bool SetZOrder(JSContext* ctx, JS::CallArgs& args);
  static bool GetClick(JSContext* ctx, JS::CallArgs& args);
  static bool SetClick(JSContext* ctx, JS::CallArgs& args);
  static bool GetHover(JSContext* ctx, JS::CallArgs& args);
  static bool SetHover(JSContext* ctx, JS::CallArgs& args);

  // functions
  static bool Remove(JSContext* ctx, JS::CallArgs& args);

  static inline JSClassOps m_ops = {
      .addProperty = nullptr,
      .delProperty = nullptr,
      .enumerate = nullptr,
      .newEnumerate = nullptr,
      .resolve = nullptr,
      .mayResolve = nullptr,
      .finalize = finalize,
      .call = nullptr,
      .hasInstance = nullptr,
      .construct = nullptr,
      .trace = nullptr,
  };
  static inline JSClass m_class = {
      "Line",
      JSCLASS_HAS_RESERVED_SLOTS(kInternalFieldCount) | JSCLASS_FOREGROUND_FINALIZE,
      &m_ops,
  };
  static inline JSPropertySpec m_props[] = {
      JS_PSGS("x", trampoline<GetX>, trampoline<SetX>, JSPROP_ENUMERATE),
      JS_PSGS("y", trampoline<GetY>, trampoline<SetY>, JSPROP_ENUMERATE),
      JS_PSGS("x2", trampoline<GetX2>, trampoline<SetY2>, JSPROP_ENUMERATE),
      JS_PSGS("y2", trampoline<GetY2>, trampoline<SetY2>, JSPROP_ENUMERATE),
      JS_PSGS("visible", trampoline<GetVisible>, trampoline<SetVisible>, JSPROP_ENUMERATE),
      JS_PSGS("color", trampoline<GetColor>, trampoline<SetColor>, JSPROP_ENUMERATE),
      JS_PSGS("zorder", trampoline<GetZOrder>, trampoline<SetZOrder>, JSPROP_ENUMERATE),
      JS_PSGS("click", trampoline<GetClick>, trampoline<SetClick>, JSPROP_ENUMERATE),
      JS_PSGS("hover", trampoline<GetHover>, trampoline<SetHover>, JSPROP_ENUMERATE),
      JS_PS_END,
  };
  static inline JSFunctionSpec m_methods[] = {
      JS_FN("remove", trampoline<Remove>, 0, JSPROP_ENUMERATE),
      JS_FS_END,
  };

  LineHook* pLine;
};

/*********************************************************
                                        Text Header
**********************************************************/

class TextWrap : public BaseObject {
 public:
  static JSObject* Instantiate(JSContext* ctx, TextHook* frame);
  static void Initialize(JSContext* ctx, JS::HandleObject target);

 private:
  TextWrap(JSContext* ctx, JS::HandleObject obj, TextHook* text);
  virtual ~TextWrap();

  static void finalize(JSFreeOp* fop, JSObject* obj);

  // constructor
  static bool New(JSContext* ctx, JS::CallArgs& args);

  // properties
  static bool GetX(JSContext* ctx, JS::CallArgs& args);
  static bool SetX(JSContext* ctx, JS::CallArgs& args);
  static bool GetY(JSContext* ctx, JS::CallArgs& args);
  static bool SetY(JSContext* ctx, JS::CallArgs& args);
  static bool GetColor(JSContext* ctx, JS::CallArgs& args);
  static bool SetColor(JSContext* ctx, JS::CallArgs& args);
  static bool GetVisible(JSContext* ctx, JS::CallArgs& args);
  static bool SetVisible(JSContext* ctx, JS::CallArgs& args);
  static bool GetAlign(JSContext* ctx, JS::CallArgs& args);
  static bool SetAlign(JSContext* ctx, JS::CallArgs& args);
  static bool GetZOrder(JSContext* ctx, JS::CallArgs& args);
  static bool SetZOrder(JSContext* ctx, JS::CallArgs& args);
  static bool GetClick(JSContext* ctx, JS::CallArgs& args);
  static bool SetClick(JSContext* ctx, JS::CallArgs& args);
  static bool GetHover(JSContext* ctx, JS::CallArgs& args);
  static bool SetHover(JSContext* ctx, JS::CallArgs& args);

  static bool GetFont(JSContext* ctx, JS::CallArgs& args);
  static bool SetFont(JSContext* ctx, JS::CallArgs& args);
  static bool GetText(JSContext* ctx, JS::CallArgs& args);
  static bool SetText(JSContext* ctx, JS::CallArgs& args);

  // functions
  static bool Remove(JSContext* ctx, JS::CallArgs& args);

  static inline JSClassOps m_ops = {
      .addProperty = nullptr,
      .delProperty = nullptr,
      .enumerate = nullptr,
      .newEnumerate = nullptr,
      .resolve = nullptr,
      .mayResolve = nullptr,
      .finalize = finalize,
      .call = nullptr,
      .hasInstance = nullptr,
      .construct = nullptr,
      .trace = nullptr,
  };
  static inline JSClass m_class = {
      "Text",
      JSCLASS_HAS_RESERVED_SLOTS(kInternalFieldCount) | JSCLASS_FOREGROUND_FINALIZE,
      &m_ops,
  };
  static inline JSPropertySpec m_props[] = {
      JS_PSGS("x", trampoline<GetX>, trampoline<SetX>, JSPROP_ENUMERATE),
      JS_PSGS("y", trampoline<GetY>, trampoline<SetY>, JSPROP_ENUMERATE),
      JS_PSGS("color", trampoline<GetColor>, trampoline<SetColor>, JSPROP_ENUMERATE),
      JS_PSGS("visible", trampoline<GetVisible>, trampoline<SetVisible>, JSPROP_ENUMERATE),
      JS_PSGS("align", trampoline<GetAlign>, trampoline<SetAlign>, JSPROP_ENUMERATE),
      JS_PSGS("zorder", trampoline<GetZOrder>, trampoline<SetZOrder>, JSPROP_ENUMERATE),
      JS_PSGS("click", trampoline<GetClick>, trampoline<SetClick>, JSPROP_ENUMERATE),
      JS_PSGS("hover", trampoline<GetHover>, trampoline<SetHover>, JSPROP_ENUMERATE),

      JS_PSGS("font", trampoline<GetFont>, trampoline<SetFont>, JSPROP_ENUMERATE),
      JS_PSGS("text", trampoline<GetText>, trampoline<SetText>, JSPROP_ENUMERATE),
      JS_PS_END,
  };
  static inline JSFunctionSpec m_methods[] = {
      JS_FN("remove", trampoline<Remove>, 0, JSPROP_ENUMERATE),
      JS_FS_END,
  };

  TextHook* pText;
};

/*********************************************************
                                        Image Header
**********************************************************/

class ImageWrap : public BaseObject {
 public:
  static JSObject* Instantiate(JSContext* ctx, ImageHook* frame);
  static void Initialize(JSContext* ctx, JS::HandleObject target);

 private:
  ImageWrap(JSContext* ctx, JS::HandleObject obj, ImageHook* image);
  virtual ~ImageWrap();

  static void finalize(JSFreeOp* fop, JSObject* obj);

  // constructor
  static bool New(JSContext* ctx, JS::CallArgs& args);

  // properties
  static bool GetX(JSContext* ctx, JS::CallArgs& args);
  static bool SetX(JSContext* ctx, JS::CallArgs& args);
  static bool GetY(JSContext* ctx, JS::CallArgs& args);
  static bool SetY(JSContext* ctx, JS::CallArgs& args);
  static bool GetVisible(JSContext* ctx, JS::CallArgs& args);
  static bool SetVisible(JSContext* ctx, JS::CallArgs& args);
  static bool GetLocation(JSContext* ctx, JS::CallArgs& args);
  static bool SetLocation(JSContext* ctx, JS::CallArgs& args);
  static bool GetAlign(JSContext* ctx, JS::CallArgs& args);
  static bool SetAlign(JSContext* ctx, JS::CallArgs& args);
  static bool GetZOrder(JSContext* ctx, JS::CallArgs& args);
  static bool SetZOrder(JSContext* ctx, JS::CallArgs& args);
  static bool GetClick(JSContext* ctx, JS::CallArgs& args);
  static bool SetClick(JSContext* ctx, JS::CallArgs& args);
  static bool GetHover(JSContext* ctx, JS::CallArgs& args);
  static bool SetHover(JSContext* ctx, JS::CallArgs& args);

  // functions
  static bool Remove(JSContext* ctx, JS::CallArgs& args);

  static inline JSClassOps m_ops = {
      .addProperty = nullptr,
      .delProperty = nullptr,
      .enumerate = nullptr,
      .newEnumerate = nullptr,
      .resolve = nullptr,
      .mayResolve = nullptr,
      .finalize = finalize,
      .call = nullptr,
      .hasInstance = nullptr,
      .construct = nullptr,
      .trace = nullptr,
  };
  static inline JSClass m_class = {
      "Image",
      JSCLASS_HAS_RESERVED_SLOTS(kInternalFieldCount) | JSCLASS_FOREGROUND_FINALIZE,
      &m_ops,
  };
  static inline JSPropertySpec m_props[] = {
      JS_PSGS("x", trampoline<GetX>, trampoline<SetX>, JSPROP_ENUMERATE),
      JS_PSGS("y", trampoline<GetY>, trampoline<SetY>, JSPROP_ENUMERATE),
      JS_PSGS("visible", trampoline<GetVisible>, trampoline<SetVisible>, JSPROP_ENUMERATE),
      JS_PSGS("location", trampoline<GetLocation>, trampoline<SetLocation>, JSPROP_ENUMERATE),
      JS_PSGS("align", trampoline<GetAlign>, trampoline<SetAlign>, JSPROP_ENUMERATE),
      JS_PSGS("zorder", trampoline<GetZOrder>, trampoline<SetZOrder>, JSPROP_ENUMERATE),
      JS_PSGS("click", trampoline<GetClick>, trampoline<SetClick>, JSPROP_ENUMERATE),
      JS_PSGS("hover", trampoline<GetHover>, trampoline<SetHover>, JSPROP_ENUMERATE),
      JS_PS_END,
  };
  static inline JSFunctionSpec m_methods[] = {
      JS_FN("remove", trampoline<Remove>, 0, JSPROP_ENUMERATE),
      JS_FS_END,
  };

  ImageHook* pImage;
};

#endif
