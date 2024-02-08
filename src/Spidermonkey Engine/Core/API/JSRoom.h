#pragma once

#include "js32.h"
#include "Game/Drlg/D2DrlgDrlg.h" // D2DrlgRoomStrc

class RoomWrap {
 public:
  static JSValue Instantiate(JSContext* ctx, JSValue new_target, D2DrlgRoomStrc* room);
  static void Initialize(JSContext* ctx, JSValue target);

 private:
  RoomWrap(JSContext* ctx, D2DrlgRoomStrc* room);

  // constructor
  static JSValue New(JSContext* ctx, JSValue new_target, int argc, JSValue* argv);

  // properties
  static JSValue GetNumber(JSContext* ctx, JSValue this_val);
  static JSValue GetX(JSContext* ctx, JSValue this_val);
  static JSValue GetY(JSContext* ctx, JSValue this_val);
  static JSValue GetSizeX(JSContext* ctx, JSValue this_val);
  static JSValue GetSizeY(JSContext* ctx, JSValue this_val);
  static JSValue GetArea(JSContext* ctx, JSValue this_val);
  static JSValue GetLevel(JSContext* ctx, JSValue this_val);
  static JSValue GetCorrectTomb(JSContext* ctx, JSValue this_val);

  // functions
  static JSValue GetFirst(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue GetNext(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue Reveal(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue GetPresetUnits(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue GetCollision(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue GetCollisionA(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue GetNearby(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue GetStat(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue UnitInRoom(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  // globals
  static JSValue GetRoom(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  static inline JSClassID m_class_id = 0;
  static inline JSCFunctionListEntry m_proto_funcs[] = {
      JS_CGETSET_DEF("number", GetNumber, nullptr),
      JS_CGETSET_DEF("x", GetX, nullptr),
      JS_CGETSET_DEF("y", GetY, nullptr),
      JS_CGETSET_DEF("xsize", GetSizeX, nullptr),
      JS_CGETSET_DEF("ysize", GetSizeY, nullptr),
      JS_CGETSET_DEF("area", GetArea, nullptr),
      JS_CGETSET_DEF("level", GetLevel, nullptr),
      JS_CGETSET_DEF("correcttomb", GetCorrectTomb, nullptr),

      JS_FS("getFirst", GetFirst, 0, FUNCTION_FLAGS),
      JS_FS("getNext", GetNext, 0, FUNCTION_FLAGS),
      JS_FS("reveal", Reveal, 1, FUNCTION_FLAGS),
      JS_FS("getPresetUnits", GetPresetUnits, 0, FUNCTION_FLAGS),
      JS_FS("getCollision", GetCollision, 0, FUNCTION_FLAGS),
      JS_FS("getCollisionA", GetCollisionA, 0, FUNCTION_FLAGS),
      JS_FS("getNearby", GetNearby, 0, FUNCTION_FLAGS),
      JS_FS("getStat", GetStat, 0, FUNCTION_FLAGS),
      JS_FS("unitInRoom", UnitInRoom, 1, FUNCTION_FLAGS),
  };

   D2DrlgRoomStrc* pRoom;
};