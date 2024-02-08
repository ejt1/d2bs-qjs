#ifndef AREA_H
#define AREA_H

#include "js32.h"

class AreaWrap {
 public:
  static JSValue Instantiate(JSContext* ctx, JSValue new_target, uint32_t dwAreaId);
  static void Initialize(JSContext* ctx, JSValue target);

 private:
  AreaWrap(JSContext* ctx, uint32_t dwAreaId);

  // properties
  static JSValue GetId(JSContext* ctx, JSValue this_val);
  static JSValue GetName(JSContext* ctx, JSValue this_val);
  static JSValue GetExits(JSContext* ctx, JSValue this_val);
  static JSValue GetPosX(JSContext* ctx, JSValue this_val);
  static JSValue GetPosY(JSContext* ctx, JSValue this_val);
  static JSValue GetSizeX(JSContext* ctx, JSValue this_val);
  static JSValue GetSizeY(JSContext* ctx, JSValue this_val);

  // globals
  static JSValue GetArea(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  static inline JSClassID m_class_id = 0;
  static inline JSCFunctionListEntry m_proto_funcs[] = {
      JS_CGETSET_DEF("id", GetId, nullptr),        //
      JS_CGETSET_DEF("name", GetName, nullptr),    //
      JS_CGETSET_DEF("exits", GetExits, nullptr),  //
      JS_CGETSET_DEF("x", GetPosX, nullptr),       //
      JS_CGETSET_DEF("y", GetPosY, nullptr),       //
      JS_CGETSET_DEF("xsize", GetSizeX, nullptr),  //
      JS_CGETSET_DEF("ysize", GetSizeY, nullptr),  //
  };

  uint32_t dwAreaId;
  uint32_t dwExits;
  JSValue arrExitArray;
};

#endif