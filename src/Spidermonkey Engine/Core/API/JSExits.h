#pragma once

#include "JSBaseObject.h"
#include "MapHeader.h"  // Exit

class ExitWrap : public BaseObject {
 public:
  static JSObject* Instantiate(JSContext* ctx, Exit* exit, uint32_t level_id);
  static void Initialize(JSContext* ctx, JS::HandleObject target);

 private:
  ExitWrap(JSContext* ctx, JS::HandleObject obj, Exit* exit, uint32_t level_id);

  static void finalize(JSFreeOp* fop, JSObject* obj);

  static bool New(JSContext* ctx, unsigned argc, JS::Value* vp);

  // properties
  static bool GetX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetTarget(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetType(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetTileId(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetLevelId(JSContext* ctx, unsigned argc, JS::Value* vp);

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
      "Exit",
      JSCLASS_HAS_RESERVED_SLOTS(kInternalFieldCount) | JSCLASS_FOREGROUND_FINALIZE,
      &m_ops,
  };
  static inline JSPropertySpec m_props[] = {
      JS_PSG("x", GetX, JSPROP_ENUMERATE | JSPROP_PERMANENT),            //
      JS_PSG("y", GetY, JSPROP_ENUMERATE | JSPROP_PERMANENT),            //
      JS_PSG("target", GetTarget, JSPROP_ENUMERATE | JSPROP_PERMANENT),  //
      JS_PSG("type", GetType, JSPROP_ENUMERATE | JSPROP_PERMANENT),      //
      JS_PSG("tileid", GetTileId, JSPROP_ENUMERATE | JSPROP_PERMANENT),  //
      JS_PSG("level", GetLevelId, JSPROP_ENUMERATE | JSPROP_PERMANENT),  //
      JS_PS_END,
  };

  uint32_t x;
  uint32_t y;
  uint32_t id;
  uint32_t type;
  uint32_t tileid;
  uint32_t level;
};
