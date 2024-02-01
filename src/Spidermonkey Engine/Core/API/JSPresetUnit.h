#ifndef PRESETUNIT_H
#define PRESETUNIT_H

#include <windows.h>
#include "js32.h"

CLASS_CTOR(presetunit);
CLASS_FINALIZER(presetunit);

JSAPI_PROP(presetunit_getProperty);

JSAPI_FUNC(my_getPresetUnit);
JSAPI_FUNC(my_getPresetUnits);

enum presetunit_tinyid {
  PUNIT_TYPE,   // 0
  PUNIT_ROOMX,  // 1
  PUNIT_ROOMY,  // 2
  PUNIT_X,      // 3
  PUNIT_Y,      // 4
  PUNIT_ID,     // 5
  PUINT_LEVEL   // 6
};

static JSPropertySpec presetunit_props[] = {
    JS_CGETSET_MAGIC_DEF("type", presetunit_getProperty, nullptr, PUNIT_TYPE),   JS_CGETSET_MAGIC_DEF("roomx", presetunit_getProperty, nullptr, PUNIT_ROOMX),
    JS_CGETSET_MAGIC_DEF("roomy", presetunit_getProperty, nullptr, PUNIT_ROOMY), JS_CGETSET_MAGIC_DEF("x", presetunit_getProperty, nullptr, PUNIT_X),
    JS_CGETSET_MAGIC_DEF("y", presetunit_getProperty, nullptr, PUNIT_Y),         JS_CGETSET_MAGIC_DEF("id", presetunit_getProperty, nullptr, PUNIT_ID),
    JS_CGETSET_MAGIC_DEF("level", presetunit_getProperty, nullptr, PUINT_LEVEL),
};

struct myPresetUnit {
  DWORD dwType;
  DWORD dwRoomX;
  DWORD dwRoomY;
  DWORD dwPosX;
  DWORD dwPosY;
  DWORD dwId;
  DWORD dwLevel;
};

#endif