#pragma once

#include "js32.h"
#include "MapHeader.h" // Exit

//CLASS_CTOR(exit);
//CLASS_FINALIZER(exit);
//
//JSAPI_PROP(exit_getProperty);
//
//struct JSExit {
//  DWORD x;
//  DWORD y;
//  DWORD id;
//  DWORD type;
//  DWORD tileid;
//  DWORD level;
//};
//
//enum exit_tinyid { EXIT_X, EXIT_Y, EXIT_TARGET, EXIT_TYPE, EXIT_TILEID, EXIT_LEVELID };
//
//static JSCFunctionListEntry exit_proto_funcs[] = {
//    JS_CGETSET_MAGIC_DEF("x", exit_getProperty, nullptr, EXIT_X),            //
//    JS_CGETSET_MAGIC_DEF("y", exit_getProperty, nullptr, EXIT_Y),            //
//    JS_CGETSET_MAGIC_DEF("target", exit_getProperty, nullptr, EXIT_TARGET),  //
//    JS_CGETSET_MAGIC_DEF("type", exit_getProperty, nullptr, EXIT_TYPE),      //
//    JS_CGETSET_MAGIC_DEF("tileid", exit_getProperty, nullptr, EXIT_TILEID),  //
//    JS_CGETSET_MAGIC_DEF("level", exit_getProperty, nullptr, EXIT_LEVELID),
//};

class ExitWrap {
 public:
  static JSValue Instantiate(JSContext* ctx, JSValue new_target, Exit* exit, uint32_t level_id);
  static void Initialize(JSContext* ctx, JSValue target);

 private:
  ExitWrap(JSContext* ctx, Exit* exit, uint32_t level_id);

  // properties
  static JSValue GetX(JSContext* ctx, JSValue this_val);
  static JSValue GetY(JSContext* ctx, JSValue this_val);
  static JSValue GetTarget(JSContext* ctx, JSValue this_val);
  static JSValue GetType(JSContext* ctx, JSValue this_val);
  static JSValue GetTileId(JSContext* ctx, JSValue this_val);
  static JSValue GetLevelId(JSContext* ctx, JSValue this_val);

  static inline JSClassID m_class_id = 0;
  static inline JSCFunctionListEntry m_proto_funcs[] = {
      JS_CGETSET_DEF("x", GetX, nullptr),        //
      JS_CGETSET_DEF("y", GetY, nullptr),    //
      JS_CGETSET_DEF("target", GetTarget, nullptr),  //
      JS_CGETSET_DEF("type", GetType, nullptr),       //
      JS_CGETSET_DEF("tileid", GetTileId, nullptr),       //
      JS_CGETSET_DEF("level", GetLevelId, nullptr),  //
  };

  uint32_t x;
  uint32_t y;
  uint32_t id;
  uint32_t type;
  uint32_t tileid;
  uint32_t level;
};