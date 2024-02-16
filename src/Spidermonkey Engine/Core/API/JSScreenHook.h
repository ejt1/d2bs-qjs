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
  static JSObject* Instantiate(JSContext* ctx, FrameHook* frame);
  static void Initialize(JSContext* ctx, JS::HandleObject target);

 private:
  FrameWrap(JSContext* ctx, JS::HandleObject obj, FrameHook* frame);
  virtual ~FrameWrap();

  static void finalize(JSFreeOp* fop, JSObject* obj);

  // constructor
  static bool New(JSContext* ctx, unsigned argc, JS::Value* vp);

  // properties
  static bool GetX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetSizeX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetSizeX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetSizeY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetSizeY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetVisible(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetVisible(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetAlign(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetAlign(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetZOrder(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetZOrder(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetClick(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetClick(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetHover(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetHover(JSContext* ctx, unsigned argc, JS::Value* vp);

  // functions
  static bool Remove(JSContext* ctx, unsigned argc, JS::Value* vp);

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
  static inline JSPropertySpec m_props[] = {JS_PSGS("x", GetX, SetX, JSPROP_ENUMERATE),
                                            JS_PSGS("y", GetY, SetY, JSPROP_ENUMERATE),
                                            JS_PSGS("xsize", GetSizeX, SetSizeX, JSPROP_ENUMERATE),
                                            JS_PSGS("ysize", GetSizeY, SetSizeY, JSPROP_ENUMERATE),
                                            JS_PSGS("visible", GetVisible, SetVisible, JSPROP_ENUMERATE),
                                            JS_PSGS("align", GetAlign, SetAlign, JSPROP_ENUMERATE),
                                            JS_PSGS("zorder", GetZOrder, SetZOrder, JSPROP_ENUMERATE),
                                            JS_PSGS("click", GetClick, SetClick, JSPROP_ENUMERATE),
                                            JS_PSGS("hover", GetHover, SetHover, JSPROP_ENUMERATE),
                                            JS_PS_END};
  static inline JSFunctionSpec m_methods[] = {
      JS_FN("remove", Remove, 0, JSPROP_ENUMERATE),
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
  static bool New(JSContext* ctx, unsigned argc, JS::Value* vp);

  // properties
  static bool GetX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetSizeX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetSizeX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetSizeY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetSizeY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetVisible(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetVisible(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetAlign(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetAlign(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetZOrder(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetZOrder(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetClick(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetClick(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetHover(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetHover(JSContext* ctx, unsigned argc, JS::Value* vp);

  static bool GetColor(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetColor(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetOpacity(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetOpacity(JSContext* ctx, unsigned argc, JS::Value* vp);

  // functions
  static bool Remove(JSContext* ctx, unsigned argc, JS::Value* vp);

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
      JS_PSGS("x", GetX, SetX, JSPROP_ENUMERATE),
      JS_PSGS("y", GetY, SetY, JSPROP_ENUMERATE),
      JS_PSGS("xsize", GetSizeX, SetSizeX, JSPROP_ENUMERATE),
      JS_PSGS("ysize", GetSizeY, SetSizeY, JSPROP_ENUMERATE),
      JS_PSGS("visible", GetVisible, SetVisible, JSPROP_ENUMERATE),
      JS_PSGS("align", GetAlign, SetAlign, JSPROP_ENUMERATE),
      JS_PSGS("zorder", GetZOrder, SetZOrder, JSPROP_ENUMERATE),
      JS_PSGS("click", GetClick, SetClick, JSPROP_ENUMERATE),
      JS_PSGS("hover", GetHover, SetHover, JSPROP_ENUMERATE),

      JS_PSGS("color", GetColor, SetColor, JSPROP_ENUMERATE),
      JS_PSGS("opacity", GetOpacity, SetOpacity, JSPROP_ENUMERATE),
      JS_PS_END,
  };
  static inline JSFunctionSpec m_methods[] = {
      JS_FN("remove", Remove, 0, JSPROP_ENUMERATE),
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
  static bool New(JSContext* ctx, unsigned argc, JS::Value* vp);

  // properties
  static bool GetX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetX2(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetX2(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetY2(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetY2(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetVisible(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetVisible(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetColor(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetColor(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetZOrder(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetZOrder(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetClick(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetClick(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetHover(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetHover(JSContext* ctx, unsigned argc, JS::Value* vp);

  // functions
  static bool Remove(JSContext* ctx, unsigned argc, JS::Value* vp);

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
      JS_PSGS("x", GetX, SetX, JSPROP_ENUMERATE),
      JS_PSGS("y", GetY, SetY, JSPROP_ENUMERATE),
      JS_PSGS("x2", GetX2, SetY2, JSPROP_ENUMERATE),
      JS_PSGS("y2", GetY2, SetY2, JSPROP_ENUMERATE),
      JS_PSGS("visible", GetVisible, SetVisible, JSPROP_ENUMERATE),
      JS_PSGS("color", GetColor, SetColor, JSPROP_ENUMERATE),
      JS_PSGS("zorder", GetZOrder, SetZOrder, JSPROP_ENUMERATE),
      JS_PSGS("click", GetClick, SetClick, JSPROP_ENUMERATE),
      JS_PSGS("hover", GetHover, SetHover, JSPROP_ENUMERATE),
      JS_PS_END,
  };
  static inline JSFunctionSpec m_methods[] = {
      JS_FN("remove", Remove, 0, JSPROP_ENUMERATE),
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
  static bool New(JSContext* ctx, unsigned argc, JS::Value* vp);

  // properties
  static bool GetX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetColor(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetColor(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetVisible(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetVisible(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetAlign(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetAlign(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetZOrder(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetZOrder(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetClick(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetClick(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetHover(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetHover(JSContext* ctx, unsigned argc, JS::Value* vp);

  static bool GetFont(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetFont(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetText(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetText(JSContext* ctx, unsigned argc, JS::Value* vp);

  // functions
  static bool Remove(JSContext* ctx, unsigned argc, JS::Value* vp);

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
      JS_PSGS("x", GetX, SetX, JSPROP_ENUMERATE),
      JS_PSGS("y", GetY, SetY, JSPROP_ENUMERATE),
      JS_PSGS("color", GetColor, SetColor, JSPROP_ENUMERATE),
      JS_PSGS("visible", GetVisible, SetVisible, JSPROP_ENUMERATE),
      JS_PSGS("align", GetAlign, SetAlign, JSPROP_ENUMERATE),
      JS_PSGS("zorder", GetZOrder, SetZOrder, JSPROP_ENUMERATE),
      JS_PSGS("click", GetClick, SetClick, JSPROP_ENUMERATE),
      JS_PSGS("hover", GetHover, SetHover, JSPROP_ENUMERATE),

      JS_PSGS("font", GetFont, SetFont, JSPROP_ENUMERATE),
      JS_PSGS("text", GetText, SetText, JSPROP_ENUMERATE),
      JS_PS_END,
  };
  static inline JSFunctionSpec m_methods[] = {
      JS_FN("remove", Remove, 0, JSPROP_ENUMERATE),
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
  static bool New(JSContext* ctx, unsigned argc, JS::Value* vp);

  // properties
  static bool GetX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetVisible(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetVisible(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetLocation(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetLocation(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetAlign(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetAlign(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetZOrder(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetZOrder(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetClick(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetClick(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetHover(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetHover(JSContext* ctx, unsigned argc, JS::Value* vp);

  // functions
  static bool Remove(JSContext* ctx, unsigned argc, JS::Value* vp);

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
      JS_PSGS("x", GetX, SetX, JSPROP_ENUMERATE),
      JS_PSGS("y", GetY, SetY, JSPROP_ENUMERATE),
      JS_PSGS("visible", GetVisible, SetVisible, JSPROP_ENUMERATE),
      JS_PSGS("location", GetLocation, SetLocation, JSPROP_ENUMERATE),
      JS_PSGS("align", GetAlign, SetAlign, JSPROP_ENUMERATE),
      JS_PSGS("zorder", GetZOrder, SetZOrder, JSPROP_ENUMERATE),
      JS_PSGS("click", GetClick, SetClick, JSPROP_ENUMERATE),
      JS_PSGS("hover", GetHover, SetHover, JSPROP_ENUMERATE),
      JS_PS_END,
  };
  static inline JSFunctionSpec m_methods[] = {
      JS_FN("remove", Remove, 0, JSPROP_ENUMERATE),
      JS_FS_END,
  };

  ImageHook* pImage;
};

#endif
