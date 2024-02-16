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

  static bool New(JSContext* ctx, unsigned argc, JS::Value* vp);

  // properties
  static bool GetId(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetName(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetExits(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetPosX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetPosY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetSizeX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetSizeY(JSContext* ctx, unsigned argc, JS::Value* vp);

  // globals
  static bool GetArea(JSContext* ctx, unsigned argc, JS::Value* vp);

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
      JS_PSG("id", GetId, JSPROP_ENUMERATE | JSPROP_PERMANENT),        //
      JS_PSG("name", GetName, JSPROP_ENUMERATE | JSPROP_PERMANENT),    //
      JS_PSG("exits", GetExits, JSPROP_ENUMERATE | JSPROP_PERMANENT),  //
      JS_PSG("x", GetPosX, JSPROP_ENUMERATE | JSPROP_PERMANENT),       //
      JS_PSG("y", GetPosY, JSPROP_ENUMERATE | JSPROP_PERMANENT),       //
      JS_PSG("xsize", GetSizeX, JSPROP_ENUMERATE | JSPROP_PERMANENT),  //
      JS_PSG("ysize", GetSizeY, JSPROP_ENUMERATE | JSPROP_PERMANENT),  //
      JS_PS_END,                                                       //
  };

  uint32_t dwAreaId;
  uint32_t dwExits;
  JS::PersistentRootedValue arrExitArray;
};

#endif
