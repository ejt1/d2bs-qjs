#pragma once

#include "JSBaseObject.h"

#include "Game/Drlg/D2DrlgDrlg.h"

class PresetUnitWrap : public BaseObject {
 public:
  static JSObject* Instantiate(JSContext* ctx, D2PresetUnitStrc* preset, D2DrlgRoomStrc* room, uint32_t level);
  static void Initialize(JSContext* ctx, JS::HandleObject target);

 private:
  PresetUnitWrap(JSContext* ctx, JS::HandleObject obj, D2PresetUnitStrc* preset, D2DrlgRoomStrc* room, uint32_t level);

  static void finalize(JSFreeOp* fop, JSObject* obj);

  // constructor
  static bool New(JSContext* ctx, JS::CallArgs& args);

  // properties
  static bool GetType(JSContext* ctx, JS::CallArgs& args);
  static bool GetRoomX(JSContext* ctx, JS::CallArgs& args);
  static bool GetRoomY(JSContext* ctx, JS::CallArgs& args);
  static bool GetX(JSContext* ctx, JS::CallArgs& args);
  static bool GetY(JSContext* ctx, JS::CallArgs& args);
  static bool GetId(JSContext* ctx, JS::CallArgs& args);
  static bool GetLevel(JSContext* ctx, JS::CallArgs& args);

  // globals
  static bool GetPresetUnit(JSContext* ctx, JS::CallArgs& args);
  static bool GetPresetUnits(JSContext* ctx, JS::CallArgs& args);

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
      "PresetUnit",
      JSCLASS_HAS_RESERVED_SLOTS(kInternalFieldCount) | JSCLASS_FOREGROUND_FINALIZE,
      &m_ops,
  };
  static inline JSPropertySpec m_props[] = {
      JS_PSG("type", trampoline<GetType>, JSPROP_ENUMERATE),    //
      JS_PSG("roomx", trampoline<GetRoomX>, JSPROP_ENUMERATE),  //
      JS_PSG("roomy", trampoline<GetRoomY>, JSPROP_ENUMERATE),  //
      JS_PSG("x", trampoline<GetX>, JSPROP_ENUMERATE),          //
      JS_PSG("y", trampoline<GetY>, JSPROP_ENUMERATE),          //
      JS_PSG("id", trampoline<GetId>, JSPROP_ENUMERATE),        //
      JS_PSG("level", trampoline<GetLevel>, JSPROP_ENUMERATE),  //
      JS_PS_END,
  };

  uint32_t dwPosX;
  uint32_t dwPosY;
  uint32_t dwRoomX;
  uint32_t dwRoomY;
  uint32_t dwType;
  uint32_t dwId;
  uint32_t dwLevel;
};
