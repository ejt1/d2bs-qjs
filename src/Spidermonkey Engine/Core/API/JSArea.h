#ifndef AREA_H
#define AREA_H

#include <windows.h>
#include "js32.h"

CLASS_CTOR(area);
CLASS_FINALIZER(area);

JSAPI_PROP(area_getProperty);

JSAPI_FUNC(my_getArea);

struct JSArea {
  DWORD AreaId;
  DWORD Exits;
  JSValue ExitArray;
};

enum area_tinyid { AUNIT_EXITS, AUNIT_NAME, AUNIT_X, AUNIT_XSIZE, AUNIT_Y, AUNIT_YSIZE, AUNIT_ID };

static JSCFunctionListEntry area_proto_funcs[] = {
    JS_CGETSET_MAGIC_DEF("exits", area_getProperty, nullptr, AUNIT_EXITS),  //
    JS_CGETSET_MAGIC_DEF("name", area_getProperty, nullptr, AUNIT_NAME),    //
    JS_CGETSET_MAGIC_DEF("x", area_getProperty, nullptr, AUNIT_X),          //
    JS_CGETSET_MAGIC_DEF("xsize", area_getProperty, nullptr, AUNIT_XSIZE),  //
    JS_CGETSET_MAGIC_DEF("y", area_getProperty, nullptr, AUNIT_Y),          //
    JS_CGETSET_MAGIC_DEF("ysize", area_getProperty, nullptr, AUNIT_YSIZE),  //
    JS_CGETSET_MAGIC_DEF("id", area_getProperty, nullptr, AUNIT_ID),        //
};

class AreaWrap {
 public:
  static void Initialize(JSContext* ctx, JSValue target);

 private:
  AreaWrap(JSContext* ctx);

  static JSValue GetId(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue GetName(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue GetExits(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue GetPosX(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue GetPosY(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue GetSizeX(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue GetSizeY(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  static inline JSClassID m_class_id = 0;
};

#endif