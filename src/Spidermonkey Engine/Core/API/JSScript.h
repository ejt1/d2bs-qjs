#pragma once

#include "js32.h"
#include "Script.h"

#include <windows.h>

CLASS_CTOR(script);

JSAPI_PROP(script_getProperty);

JSAPI_FUNC(script_getNext);
JSAPI_FUNC(script_stop);
JSAPI_FUNC(script_send);
JSAPI_FUNC(script_pause);
JSAPI_FUNC(script_resume);
JSAPI_FUNC(script_join);
JSAPI_FUNC(my_getScript);
JSAPI_FUNC(my_getScripts);

enum script_tinyid {
  SCRIPT_FILENAME,
  SCRIPT_GAMETYPE,
  SCRIPT_RUNNING,
  SCRIPT_THREADID,
  SCRIPT_MEMORY,
};

static JSCFunctionListEntry script_proto_funcs[] = {
    JS_CGETSET_MAGIC_DEF("name", script_getProperty, nullptr, SCRIPT_FILENAME),      //
    JS_CGETSET_MAGIC_DEF("type", script_getProperty, nullptr, SCRIPT_GAMETYPE),      //
    JS_CGETSET_MAGIC_DEF("running", script_getProperty, nullptr, SCRIPT_RUNNING),    //
    JS_CGETSET_MAGIC_DEF("threadid", script_getProperty, nullptr, SCRIPT_THREADID),  //
    JS_CGETSET_MAGIC_DEF("memory", script_getProperty, nullptr, SCRIPT_MEMORY),

    JS_FS("getNext", script_getNext, 0, FUNCTION_FLAGS),  //
    JS_FS("pause", script_pause, 0, FUNCTION_FLAGS),      //
    JS_FS("resume", script_resume, 0, FUNCTION_FLAGS),    //
    JS_FS("stop", script_stop, 0, FUNCTION_FLAGS),        //
    JS_FS("join", script_join, 0, FUNCTION_FLAGS),        //
    JS_FS("send", script_send, 1, FUNCTION_FLAGS),
};