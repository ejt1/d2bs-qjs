#ifndef AREA_H
#define AREA_H

#include "JSBaseObject.h"

class AreaWrap : public BaseObject {
 public:
  static JSObject* Instantiate(JSContext* ctx, uint32_t dwAreaId);
  static void Initialize(JSContext* ctx, JS::HandleObject target);

 private:
  AreaWrap(JSContext* ctx, JS::HandleObject obj, uint32_t dwAreaId);

  static void finalize(JSFreeOp* fop, JSObject* obj);

  static bool New(JSContext* ctx, JS::CallArgs& args);

  // properties
  static bool GetId(JSContext* ctx, JS::CallArgs& args);
  static bool GetName(JSContext* ctx, JS::CallArgs& args);
  static bool GetExits(JSContext* ctx, JS::CallArgs& args);
  static bool GetPosX(JSContext* ctx, JS::CallArgs& args);
  static bool GetPosY(JSContext* ctx, JS::CallArgs& args);
  static bool GetSizeX(JSContext* ctx, JS::CallArgs& args);
  static bool GetSizeY(JSContext* ctx, JS::CallArgs& args);

  // globals
  static bool GetArea(JSContext* ctx, JS::CallArgs& args);

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
      "Area",
      JSCLASS_HAS_RESERVED_SLOTS(kInternalFieldCount) | JSCLASS_FOREGROUND_FINALIZE,
      &m_ops,
  };
  static inline JSPropertySpec m_props[] = {
      JS_PSG("id", trampoline<GetId>, JSPROP_ENUMERATE | JSPROP_PERMANENT),        //
      JS_PSG("name", trampoline<GetName>, JSPROP_ENUMERATE | JSPROP_PERMANENT),    //
      JS_PSG("exits", trampoline<GetExits>, JSPROP_ENUMERATE | JSPROP_PERMANENT),  //
      JS_PSG("x", trampoline<GetPosX>, JSPROP_ENUMERATE | JSPROP_PERMANENT),       //
      JS_PSG("y", trampoline<GetPosY>, JSPROP_ENUMERATE | JSPROP_PERMANENT),       //
      JS_PSG("xsize", trampoline<GetSizeX>, JSPROP_ENUMERATE | JSPROP_PERMANENT),  //
      JS_PSG("ysize", trampoline<GetSizeY>, JSPROP_ENUMERATE | JSPROP_PERMANENT),  //
      JS_PS_END,                                                                   //
  };

  uint32_t dwAreaId;
  uint32_t dwExits;
  JS::PersistentRootedValue arrExitArray;
};

#endif
