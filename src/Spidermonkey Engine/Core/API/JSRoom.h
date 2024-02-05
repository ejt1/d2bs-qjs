#ifndef ROOM_H
#define ROOM_H

#include "Room.h"

#include "js32.h"

CLASS_CTOR(room);

JSAPI_PROP(room_getProperty);

JSAPI_FUNC(room_getNext);
JSAPI_FUNC(room_getPresetUnits);
JSAPI_FUNC(room_getCollision);
JSAPI_FUNC(room_getCollisionTypeArray);
JSAPI_FUNC(room_getNearby);
JSAPI_FUNC(room_getStat);
JSAPI_FUNC(room_getFirst);
JSAPI_FUNC(room_unitInRoom);
JSAPI_FUNC(room_reveal);

JSAPI_FUNC(my_getRoom);

enum room_tinyid {
  ROOM_NUM,
  ROOM_XPOS,
  ROOM_YPOS,
  ROOM_XSIZE,
  ROOM_YSIZE,
  ROOM_SUBNUMBER,
  ROOM_AREA,
  ROOM_LEVEL,
  ROOM_CORRECTTOMB,
};

static JSCFunctionListEntry room_proto_funcs[] = {
    JS_CGETSET_MAGIC_DEF("number", room_getProperty, nullptr, ROOM_NUM),
    JS_CGETSET_MAGIC_DEF("x", room_getProperty, nullptr, ROOM_XPOS),
    JS_CGETSET_MAGIC_DEF("y", room_getProperty, nullptr, ROOM_YPOS),
    JS_CGETSET_MAGIC_DEF("xsize", room_getProperty, nullptr, ROOM_XSIZE),
    JS_CGETSET_MAGIC_DEF("ysize", room_getProperty, nullptr, ROOM_YSIZE),
    JS_CGETSET_MAGIC_DEF("subnumber", room_getProperty, nullptr, ROOM_SUBNUMBER),
    JS_CGETSET_MAGIC_DEF("area", room_getProperty, nullptr, ROOM_AREA),
    JS_CGETSET_MAGIC_DEF("level", room_getProperty, nullptr, ROOM_LEVEL),
    JS_CGETSET_MAGIC_DEF("correcttomb", room_getProperty, nullptr, ROOM_CORRECTTOMB),

    JS_FS("getNext", room_getNext, 0, FUNCTION_FLAGS),
    JS_FS("reveal", room_reveal, 1, FUNCTION_FLAGS),
    JS_FS("getPresetUnits", room_getPresetUnits, 0, FUNCTION_FLAGS),
    JS_FS("getCollision", room_getCollision, 0, FUNCTION_FLAGS),
    JS_FS("getCollisionA", room_getCollisionTypeArray, 0, FUNCTION_FLAGS),
    JS_FS("getNearby", room_getNearby, 0, FUNCTION_FLAGS),
    JS_FS("getStat", room_getStat, 0, FUNCTION_FLAGS),
    JS_FS("getFirst", room_getFirst, 0, FUNCTION_FLAGS),
    JS_FS("unitInRoom", room_unitInRoom, 1, FUNCTION_FLAGS),
};

#endif
