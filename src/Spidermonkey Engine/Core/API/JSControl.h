#pragma once

#include "Control.h"

#include "js32.h"

#include <windows.h>

CLASS_CTOR(control);
CLASS_FINALIZER(control);

JSAPI_PROP(control_getProperty);
JSAPI_STRICT_PROP(control_setProperty);

JSAPI_FUNC(control_getNext);
JSAPI_FUNC(control_click);
JSAPI_FUNC(control_setText);
JSAPI_FUNC(control_getText);

struct JSControl {
  DWORD _dwPrivate;
  D2WinControlStrc* pControl;

  DWORD dwType;
  DWORD dwX;
  DWORD dwY;
  DWORD dwSizeX;
  DWORD dwSizeY;
};

enum control_tinyid {
  CONTROL_TEXT,
  CONTROL_X,
  CONTROL_Y,
  CONTROL_XSIZE,
  CONTROL_YSIZE,
  CONTROL_STATE,
  CONTROL_MAXLENGTH,
  CONTROL_TYPE,
  CONTROL_VISIBLE,
  CONTROL_CURSORPOS,
  CONTROL_SELECTSTART,
  CONTROL_SELECTEND,
  CONTROL_PASSWORD,
  CONTROL_DISABLED
};

static JSCFunctionListEntry control_proto_funcs[] = {
    JS_CGETSET_MAGIC_DEF("text", control_getProperty, control_setProperty, CONTROL_TEXT),
    JS_CGETSET_MAGIC_DEF("x", control_getProperty, nullptr, CONTROL_X),
    JS_CGETSET_MAGIC_DEF("y", control_getProperty, nullptr, CONTROL_Y),
    JS_CGETSET_MAGIC_DEF("xsize", control_getProperty, nullptr, CONTROL_XSIZE),
    JS_CGETSET_MAGIC_DEF("ysize", control_getProperty, nullptr, CONTROL_YSIZE),
    JS_CGETSET_MAGIC_DEF("state", control_getProperty, control_setProperty, CONTROL_STATE),
    JS_CGETSET_MAGIC_DEF("password", control_getProperty, nullptr, CONTROL_PASSWORD),
    JS_CGETSET_MAGIC_DEF("type", control_getProperty, nullptr, CONTROL_TYPE),
    JS_CGETSET_MAGIC_DEF("cursorpos", control_getProperty, control_setProperty, CONTROL_CURSORPOS),
    JS_CGETSET_MAGIC_DEF("selectstart", control_getProperty, nullptr, CONTROL_SELECTSTART),
    JS_CGETSET_MAGIC_DEF("selectend", control_getProperty, nullptr, CONTROL_SELECTEND),
    JS_CGETSET_MAGIC_DEF("disabled", control_getProperty, control_setProperty, CONTROL_DISABLED),

    JS_FS("getNext", control_getNext, 0, FUNCTION_FLAGS),
    JS_FS("click", control_click, 0, FUNCTION_FLAGS),
    JS_FS("setText", control_setText, 1, FUNCTION_FLAGS),
    JS_FS("getText", control_getText, 0, FUNCTION_FLAGS),
};