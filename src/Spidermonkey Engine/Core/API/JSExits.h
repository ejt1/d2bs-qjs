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

  static bool New(JSContext* ctx, JS::CallArgs& args);

  // properties
  static bool GetX(JSContext* ctx, JS::CallArgs& args);
  static bool GetY(JSContext* ctx, JS::CallArgs& args);
  static bool GetTarget(JSContext* ctx, JS::CallArgs& args);
  static bool GetType(JSContext* ctx, JS::CallArgs& args);
  static bool GetTileId(JSContext* ctx, JS::CallArgs& args);
  static bool GetLevelId(JSContext* ctx, JS::CallArgs& args);

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
      JS_PSG("x", trampoline<GetX>, JSPROP_ENUMERATE | JSPROP_PERMANENT),            //
      JS_PSG("y", trampoline<GetY>, JSPROP_ENUMERATE | JSPROP_PERMANENT),            //
      JS_PSG("target", trampoline<GetTarget>, JSPROP_ENUMERATE | JSPROP_PERMANENT),  //
      JS_PSG("type", trampoline<GetType>, JSPROP_ENUMERATE | JSPROP_PERMANENT),      //
      JS_PSG("tileid", trampoline<GetTileId>, JSPROP_ENUMERATE | JSPROP_PERMANENT),  //
      JS_PSG("level", trampoline<GetLevelId>, JSPROP_ENUMERATE | JSPROP_PERMANENT),  //
      JS_PS_END,
  };

  uint32_t x;
  uint32_t y;
  uint32_t id;
  uint32_t type;
  uint32_t tileid;
  uint32_t level;
};
