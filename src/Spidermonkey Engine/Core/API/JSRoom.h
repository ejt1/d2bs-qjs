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
  static bool New(JSContext* ctx, unsigned argc, JS::Value* vp);

  // properties
  static bool GetNumber(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetSizeX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetSizeY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetArea(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetLevel(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetCorrectTomb(JSContext* ctx, unsigned argc, JS::Value* vp);

  // functions
  static bool GetFirst(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetNext(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool Reveal(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetPresetUnits(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetCollision(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetCollisionA(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetNearby(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetStat(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool UnitInRoom(JSContext* ctx, unsigned argc, JS::Value* vp);

  // globals
  static bool GetRoom(JSContext* ctx, unsigned argc, JS::Value* vp);

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
      JS_PSG("number", GetNumber, JSPROP_ENUMERATE),
      JS_PSG("x", GetX, JSPROP_ENUMERATE),
      JS_PSG("y", GetY, JSPROP_ENUMERATE),
      JS_PSG("xsize", GetSizeX, JSPROP_ENUMERATE),
      JS_PSG("ysize", GetSizeY, JSPROP_ENUMERATE),
      JS_PSG("area", GetArea, JSPROP_ENUMERATE),
      JS_PSG("level", GetLevel, JSPROP_ENUMERATE),
      JS_PSG("correcttomb", GetCorrectTomb, JSPROP_ENUMERATE),
      JS_PS_END,
  };
  static inline JSFunctionSpec m_methods[] = {
      JS_FN("getFirst", GetFirst, 0, JSPROP_ENUMERATE),
      JS_FN("getNext", GetNext, 0, JSPROP_ENUMERATE),
      JS_FN("reveal", Reveal, 1, JSPROP_ENUMERATE),
      JS_FN("getPresetUnits", GetPresetUnits, 0, JSPROP_ENUMERATE),
      JS_FN("getCollision", GetCollision, 0, JSPROP_ENUMERATE),
      JS_FN("getCollisionA", GetCollisionA, 0, JSPROP_ENUMERATE),
      JS_FN("getNearby", GetNearby, 0, JSPROP_ENUMERATE),
      JS_FN("getStat", GetStat, 0, JSPROP_ENUMERATE),
      JS_FN("unitInRoom", UnitInRoom, 1, JSPROP_ENUMERATE),
      JS_FS_END,
  };

  D2DrlgRoomStrc* pRoom;
};
