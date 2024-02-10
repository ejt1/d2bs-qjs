#pragma once

// #include <windows.h>
#include "js32.h"

#include "Game/Drlg/D2DrlgDrlg.h"

class PresetUnitWrap {
 public:
  static JSValue Instantiate(JSContext* ctx, JSValue new_target, D2PresetUnitStrc* preset, D2DrlgRoomStrc* room, uint32_t level);
  static void Initialize(JSContext* ctx, JSValue target);

 private:
  PresetUnitWrap(JSContext* ctx, D2PresetUnitStrc* preset, D2DrlgRoomStrc* room, uint32_t level);

  // constructor
  static JSValue New(JSContext* ctx, JSValue new_target, int argc, JSValue* argv);

  // properties
  static JSValue GetType(JSContext* ctx, JSValue this_val);
  static JSValue GetRoomX(JSContext* ctx, JSValue this_val);
  static JSValue GetRoomY(JSContext* ctx, JSValue this_val);
  static JSValue GetX(JSContext* ctx, JSValue this_val);
  static JSValue GetY(JSContext* ctx, JSValue this_val);
  static JSValue GetId(JSContext* ctx, JSValue this_val);
  static JSValue GetLevel(JSContext* ctx, JSValue this_val);

  // globals
  static JSValue GetPresetUnit(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue GetPresetUnits(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  static inline JSClassID m_class_id = 0;
  static inline JSCFunctionListEntry m_proto_funcs[] = {
      JS_CGETSET_DEF("type", GetType, nullptr),    //
      JS_CGETSET_DEF("roomx", GetRoomX, nullptr),  //
      JS_CGETSET_DEF("roomy", GetRoomY, nullptr),  //
      JS_CGETSET_DEF("x", GetX, nullptr),          //
      JS_CGETSET_DEF("y", GetY, nullptr),          //
      JS_CGETSET_DEF("id", GetId, nullptr),        //
      JS_CGETSET_DEF("level", GetLevel, nullptr),  //
  };

  uint32_t dwPosX;
  uint32_t dwPosY;
  uint32_t dwRoomX;
  uint32_t dwRoomY;
  uint32_t dwType;
  uint32_t dwId;
  uint32_t dwLevel;
};