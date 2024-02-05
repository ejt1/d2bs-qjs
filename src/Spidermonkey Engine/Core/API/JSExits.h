#ifndef EXITS_H
#define EXITS_H

#include "js32.h"
#include <windows.h>

CLASS_CTOR(exit);
CLASS_FINALIZER(exit);

JSAPI_PROP(exit_getProperty);

enum exit_tinyid { EXIT_X, EXIT_Y, EXIT_TARGET, EXIT_TYPE, EXIT_TILEID, EXIT_LEVELID };

static JSCFunctionListEntry exit_proto_funcs[] = {
    JS_CGETSET_MAGIC_DEF("x", exit_getProperty, nullptr, EXIT_X),            //
    JS_CGETSET_MAGIC_DEF("y", exit_getProperty, nullptr, EXIT_Y),            //
    JS_CGETSET_MAGIC_DEF("target", exit_getProperty, nullptr, EXIT_TARGET),  //
    JS_CGETSET_MAGIC_DEF("type", exit_getProperty, nullptr, EXIT_TYPE),      //
    JS_CGETSET_MAGIC_DEF("tileid", exit_getProperty, nullptr, EXIT_TILEID),  //
    JS_CGETSET_MAGIC_DEF("level", exit_getProperty, nullptr, EXIT_LEVELID),
};

struct myExit {
  DWORD x;
  DWORD y;
  DWORD id;
  DWORD type;
  DWORD tileid;
  DWORD level;
};

#endif