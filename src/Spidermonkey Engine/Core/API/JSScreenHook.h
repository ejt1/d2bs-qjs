#ifndef __JSSCREENHOOK_H__
#define __JSSCREENHOOK_H__

#include "js32.h"
#include "ScreenHook.h"

// TODO(ejt): the whole gui part must be rewritten from the ground up otherwise
// I would put effort into making the wrappers be derivative using a base class
// instead of redefining everything

/*********************************************************
                                        frame Header
**********************************************************/

class FrameWrap {
 public:
  static JSValue Instantiate(JSContext* ctx, JSValue new_target, FrameHook* frame);
  static void Initialize(JSContext* ctx, JSValue target);

 private:
  FrameWrap(JSContext* ctx, FrameHook* frame);
  virtual ~FrameWrap();

  // constructor
  static JSValue New(JSContext* ctx, JSValue new_target, int argc, JSValue* argv);

  // properties
  static JSValue GetX(JSContext* ctx, JSValue this_val);
  static JSValue SetX(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetY(JSContext* ctx, JSValue this_val);
  static JSValue SetY(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetSizeX(JSContext* ctx, JSValue this_val);
  static JSValue SetSizeX(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetSizeY(JSContext* ctx, JSValue this_val);
  static JSValue SetSizeY(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetVisible(JSContext* ctx, JSValue this_val);
  static JSValue SetVisible(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetAlign(JSContext* ctx, JSValue this_val);
  static JSValue SetAlign(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetZOrder(JSContext* ctx, JSValue this_val);
  static JSValue SetZOrder(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetClick(JSContext* ctx, JSValue this_val);
  static JSValue SetClick(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetHover(JSContext* ctx, JSValue this_val);
  static JSValue SetHover(JSContext* ctx, JSValue this_val, JSValue val);

  // functions
  static JSValue Remove(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  static inline JSClassID m_class_id = 0;
  static inline JSCFunctionListEntry m_proto_funcs[] = {
      JS_CGETSET_DEF("x", GetX, SetX),
      JS_CGETSET_DEF("y", GetY, SetY),
      JS_CGETSET_DEF("xsize", GetSizeX, SetSizeX),
      JS_CGETSET_DEF("ysize", GetSizeY, SetSizeY),
      JS_CGETSET_DEF("visible", GetVisible, SetVisible),
      JS_CGETSET_DEF("align", GetAlign, SetAlign),
      JS_CGETSET_DEF("zorder", GetZOrder, SetZOrder),
      JS_CGETSET_DEF("click", GetClick, SetClick),
      JS_CGETSET_DEF("hover", GetHover, SetHover),

      JS_FS("remove", Remove, 0, FUNCTION_FLAGS),
  };

  FrameHook* pFrame;
};

/*********************************************************
                                        box Header
**********************************************************/

class BoxWrap {
 public:
  static JSValue Instantiate(JSContext* ctx, JSValue new_target, BoxHook* box);
  static void Initialize(JSContext* ctx, JSValue target);

 private:
  BoxWrap(JSContext* ctx, BoxHook* box);
  virtual ~BoxWrap();

  // constructor
  static JSValue New(JSContext* ctx, JSValue new_target, int argc, JSValue* argv);

  // properties
  static JSValue GetX(JSContext* ctx, JSValue this_val);
  static JSValue SetX(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetY(JSContext* ctx, JSValue this_val);
  static JSValue SetY(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetSizeX(JSContext* ctx, JSValue this_val);
  static JSValue SetSizeX(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetSizeY(JSContext* ctx, JSValue this_val);
  static JSValue SetSizeY(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetVisible(JSContext* ctx, JSValue this_val);
  static JSValue SetVisible(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetAlign(JSContext* ctx, JSValue this_val);
  static JSValue SetAlign(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetZOrder(JSContext* ctx, JSValue this_val);
  static JSValue SetZOrder(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetClick(JSContext* ctx, JSValue this_val);
  static JSValue SetClick(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetHover(JSContext* ctx, JSValue this_val);
  static JSValue SetHover(JSContext* ctx, JSValue this_val, JSValue val);

  static JSValue GetColor(JSContext* ctx, JSValue this_val);
  static JSValue SetColor(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetOpacity(JSContext* ctx, JSValue this_val);
  static JSValue SetOpacity(JSContext* ctx, JSValue this_val, JSValue val);

  // functions
  static JSValue Remove(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  static inline JSClassID m_class_id = 0;
  static inline JSCFunctionListEntry m_proto_funcs[] = {
      JS_CGETSET_DEF("x", GetX, SetX),
      JS_CGETSET_DEF("y", GetY, SetY),
      JS_CGETSET_DEF("xsize", GetSizeX, SetSizeX),
      JS_CGETSET_DEF("ysize", GetSizeY, SetSizeY),
      JS_CGETSET_DEF("visible", GetVisible, SetVisible),
      JS_CGETSET_DEF("align", GetAlign, SetAlign),
      JS_CGETSET_DEF("zorder", GetZOrder, SetZOrder),
      JS_CGETSET_DEF("click", GetClick, SetClick),
      JS_CGETSET_DEF("hover", GetHover, SetHover),

      JS_CGETSET_DEF("color", GetColor, SetColor),
      JS_CGETSET_DEF("opacity", GetOpacity, SetOpacity),

      JS_FS("remove", Remove, 0, FUNCTION_FLAGS),
  };

  BoxHook* pBox;
};

/*********************************************************
                                        Line Header
**********************************************************/

class LineWrap {
 public:
  static JSValue Instantiate(JSContext* ctx, JSValue new_target, LineHook* line);
  static void Initialize(JSContext* ctx, JSValue target);

 private:
  LineWrap(JSContext* ctx, LineHook* line);
  virtual ~LineWrap();

  // constructor
  static JSValue New(JSContext* ctx, JSValue new_target, int argc, JSValue* argv);

  // properties
  static JSValue GetX(JSContext* ctx, JSValue this_val);
  static JSValue SetX(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetY(JSContext* ctx, JSValue this_val);
  static JSValue SetY(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetX2(JSContext* ctx, JSValue this_val);
  static JSValue SetX2(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetY2(JSContext* ctx, JSValue this_val);
  static JSValue SetY2(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetVisible(JSContext* ctx, JSValue this_val);
  static JSValue SetVisible(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetColor(JSContext* ctx, JSValue this_val);
  static JSValue SetColor(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetZOrder(JSContext* ctx, JSValue this_val);
  static JSValue SetZOrder(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetClick(JSContext* ctx, JSValue this_val);
  static JSValue SetClick(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetHover(JSContext* ctx, JSValue this_val);
  static JSValue SetHover(JSContext* ctx, JSValue this_val, JSValue val);

  // functions
  static JSValue Remove(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  static inline JSClassID m_class_id = 0;
  static inline JSCFunctionListEntry m_proto_funcs[] = {
      JS_CGETSET_DEF("x", GetX, SetX),
      JS_CGETSET_DEF("y", GetY, SetY),
      JS_CGETSET_DEF("x2", GetX2, SetY2),
      JS_CGETSET_DEF("y2", GetY2, SetY2),
      JS_CGETSET_DEF("visible", GetVisible, SetVisible),
      JS_CGETSET_DEF("color", GetColor, SetColor),
      JS_CGETSET_DEF("zorder", GetZOrder, SetZOrder),
      JS_CGETSET_DEF("click", GetClick, SetClick),
      JS_CGETSET_DEF("hover", GetHover, SetHover),

      JS_FS("remove", Remove, 0, FUNCTION_FLAGS),
  };

  LineHook* pLine;
};

/*********************************************************
                                        Text Header
**********************************************************/

class TextWrap {
 public:
  static JSValue Instantiate(JSContext* ctx, JSValue new_target, TextHook* text);
  static void Initialize(JSContext* ctx, JSValue target);

 private:
  TextWrap(JSContext* ctx, TextHook* text);
  virtual ~TextWrap();

  // constructor
  static JSValue New(JSContext* ctx, JSValue new_target, int argc, JSValue* argv);

  // properties
  static JSValue GetX(JSContext* ctx, JSValue this_val);
  static JSValue SetX(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetY(JSContext* ctx, JSValue this_val);
  static JSValue SetY(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetColor(JSContext* ctx, JSValue this_val);
  static JSValue SetColor(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetVisible(JSContext* ctx, JSValue this_val);
  static JSValue SetVisible(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetAlign(JSContext* ctx, JSValue this_val);
  static JSValue SetAlign(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetZOrder(JSContext* ctx, JSValue this_val);
  static JSValue SetZOrder(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetClick(JSContext* ctx, JSValue this_val);
  static JSValue SetClick(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetHover(JSContext* ctx, JSValue this_val);
  static JSValue SetHover(JSContext* ctx, JSValue this_val, JSValue val);

  static JSValue GetFont(JSContext* ctx, JSValue this_val);
  static JSValue SetFont(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetText(JSContext* ctx, JSValue this_val);
  static JSValue SetText(JSContext* ctx, JSValue this_val, JSValue val);

  // functions
  static JSValue Remove(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  static inline JSClassID m_class_id = 0;
  static inline JSCFunctionListEntry m_proto_funcs[] = {
      JS_CGETSET_DEF("x", GetX, SetX),
      JS_CGETSET_DEF("y", GetY, SetY),
      JS_CGETSET_DEF("color", GetColor, SetColor),
      JS_CGETSET_DEF("visible", GetVisible, SetVisible),
      JS_CGETSET_DEF("align", GetAlign, SetAlign),
      JS_CGETSET_DEF("zorder", GetZOrder, SetZOrder),
      JS_CGETSET_DEF("click", GetClick, SetClick),
      JS_CGETSET_DEF("hover", GetHover, SetHover),

      JS_CGETSET_DEF("font", GetFont, SetFont),
      JS_CGETSET_DEF("text", GetText, SetText),

      JS_FS("remove", Remove, 0, FUNCTION_FLAGS),
  };

  TextHook* pText;
};

/*********************************************************
                                        Image Header
**********************************************************/

class ImageWrap {
 public:
  static JSValue Instantiate(JSContext* ctx, JSValue new_target, ImageHook* image);
  static void Initialize(JSContext* ctx, JSValue target);

 private:
  ImageWrap(JSContext* ctx, ImageHook* image);
  virtual ~ImageWrap();

  // constructor
  static JSValue New(JSContext* ctx, JSValue new_target, int argc, JSValue* argv);

  // properties
  static JSValue GetX(JSContext* ctx, JSValue this_val);
  static JSValue SetX(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetY(JSContext* ctx, JSValue this_val);
  static JSValue SetY(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetVisible(JSContext* ctx, JSValue this_val);
  static JSValue SetVisible(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetLocation(JSContext* ctx, JSValue this_val);
  static JSValue SetLocation(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetAlign(JSContext* ctx, JSValue this_val);
  static JSValue SetAlign(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetZOrder(JSContext* ctx, JSValue this_val);
  static JSValue SetZOrder(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetClick(JSContext* ctx, JSValue this_val);
  static JSValue SetClick(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetHover(JSContext* ctx, JSValue this_val);
  static JSValue SetHover(JSContext* ctx, JSValue this_val, JSValue val);

  // functions
  static JSValue Remove(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  static inline JSClassID m_class_id = 0;
  static inline JSCFunctionListEntry m_proto_funcs[] = {
      JS_CGETSET_DEF("x", GetX, SetX),
      JS_CGETSET_DEF("y", GetY, SetY),
      JS_CGETSET_DEF("visible", GetVisible, SetVisible),
      JS_CGETSET_DEF("location", GetLocation, SetLocation),
      JS_CGETSET_DEF("align", GetAlign, SetAlign),
      JS_CGETSET_DEF("zorder", GetZOrder, SetZOrder),
      JS_CGETSET_DEF("click", GetClick, SetClick),
      JS_CGETSET_DEF("hover", GetHover, SetHover),

      JS_FS("remove", Remove, 0, FUNCTION_FLAGS),
  };

  ImageHook* pImage;
};

#endif
