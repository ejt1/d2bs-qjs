#pragma once

#include "Game/Drlg/D2DrlgDrlg.h"  // D2DrlgRoomStrc
#include "JSBaseObject.h"

class RoomWrap : BaseObject {
 public:
  static JSObject* Instantiate(JSContext* ctx, D2DrlgRoomStrc* room);
  static void Initialize(JSContext* ctx, JS::HandleObject target);

 private:
  RoomWrap(JSContext* ctx, JS::HandleObject obj, D2DrlgRoomStrc* room);

  static void finalize(JSFreeOp* fop, JSObject* obj);

  // constructor
  static bool New(JSContext* ctx, JS::CallArgs& args);

  // properties
  static bool GetNumber(JSContext* ctx, JS::CallArgs& args);
  static bool GetX(JSContext* ctx, JS::CallArgs& args);
  static bool GetY(JSContext* ctx, JS::CallArgs& args);
  static bool GetSizeX(JSContext* ctx, JS::CallArgs& args);
  static bool GetSizeY(JSContext* ctx, JS::CallArgs& args);
  static bool GetArea(JSContext* ctx, JS::CallArgs& args);
  static bool GetLevel(JSContext* ctx, JS::CallArgs& args);
  static bool GetCorrectTomb(JSContext* ctx, JS::CallArgs& args);

  // functions
  static bool GetFirst(JSContext* ctx, JS::CallArgs& args);
  static bool GetNext(JSContext* ctx, JS::CallArgs& args);
  static bool Reveal(JSContext* ctx, JS::CallArgs& args);
  static bool GetPresetUnits(JSContext* ctx, JS::CallArgs& args);
  static bool GetCollision(JSContext* ctx, JS::CallArgs& args);
  static bool GetCollisionA(JSContext* ctx, JS::CallArgs& args);
  static bool GetNearby(JSContext* ctx, JS::CallArgs& args);
  static bool GetStat(JSContext* ctx, JS::CallArgs& args);
  static bool UnitInRoom(JSContext* ctx, JS::CallArgs& args);

  // globals
  static bool GetRoom(JSContext* ctx, JS::CallArgs& args);

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
      "Room",
      JSCLASS_HAS_RESERVED_SLOTS(kInternalFieldCount) | JSCLASS_FOREGROUND_FINALIZE,
      &m_ops,
  };
  static inline JSPropertySpec m_props[] = {
      JS_PSG("number", trampoline<GetNumber>, JSPROP_ENUMERATE),
      JS_PSG("x", trampoline<GetX>, JSPROP_ENUMERATE),
      JS_PSG("y", trampoline<GetY>, JSPROP_ENUMERATE),
      JS_PSG("xsize", trampoline<GetSizeX>, JSPROP_ENUMERATE),
      JS_PSG("ysize", trampoline<GetSizeY>, JSPROP_ENUMERATE),
      JS_PSG("area", trampoline<GetArea>, JSPROP_ENUMERATE),
      JS_PSG("level", trampoline<GetLevel>, JSPROP_ENUMERATE),
      JS_PSG("correcttomb", trampoline<GetCorrectTomb>, JSPROP_ENUMERATE),
      JS_PS_END,
  };
  static inline JSFunctionSpec m_methods[] = {
      JS_FN("getFirst", trampoline<GetFirst>, 0, JSPROP_ENUMERATE),
      JS_FN("getNext", trampoline<GetNext>, 0, JSPROP_ENUMERATE),
      JS_FN("reveal", trampoline<Reveal>, 1, JSPROP_ENUMERATE),
      JS_FN("getPresetUnits", trampoline<GetPresetUnits>, 0, JSPROP_ENUMERATE),
      JS_FN("getCollision", trampoline<GetCollision>, 0, JSPROP_ENUMERATE),
      JS_FN("getCollisionA", trampoline<GetCollisionA>, 0, JSPROP_ENUMERATE),
      JS_FN("getNearby", trampoline<GetNearby>, 0, JSPROP_ENUMERATE),
      JS_FN("getStat", trampoline<GetStat>, 0, JSPROP_ENUMERATE),
      JS_FN("unitInRoom", trampoline<UnitInRoom>, 1, JSPROP_ENUMERATE),
      JS_FS_END,
  };

  D2DrlgRoomStrc* pRoom;
};
