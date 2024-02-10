#pragma once

#include "Control.h"
#include "js32.h"

class ControlWrap {
 public:
  static JSValue Instantiate(JSContext* ctx, JSValue new_target, D2WinControlStrc* control);
  static void Initialize(JSContext* ctx, JSValue target);

 private:
  ControlWrap(JSContext* ctx, D2WinControlStrc* control);

  // properties
  static JSValue GetText(JSContext* ctx, JSValue this_val);
  static JSValue SetText(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetX(JSContext* ctx, JSValue this_val);
  static JSValue GetY(JSContext* ctx, JSValue this_val);
  static JSValue GetSizeX(JSContext* ctx, JSValue this_val);
  static JSValue GetSizeY(JSContext* ctx, JSValue this_val);
  static JSValue GetState(JSContext* ctx, JSValue this_val);
  static JSValue SetState(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetPassword(JSContext* ctx, JSValue this_val);
  static JSValue GetType(JSContext* ctx, JSValue this_val);
  static JSValue GetCursorPos(JSContext* ctx, JSValue this_val);
  static JSValue SetCursorPos(JSContext* ctx, JSValue this_val, JSValue val);
  static JSValue GetSelectStart(JSContext* ctx, JSValue this_val);
  static JSValue GetSelectEnd(JSContext* ctx, JSValue this_val);
  static JSValue GetDisabled(JSContext* ctx, JSValue this_val);
  static JSValue SetDisabled(JSContext* ctx, JSValue this_val, JSValue val);

  // functions
  static JSValue GetNext(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue Click(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue FreeGetText(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue FreeSetText(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  // globals
  static JSValue GetControl(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue GetControls(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  static inline JSClassID m_class_id = 0;
  static inline JSCFunctionListEntry m_proto_funcs[] = {
      JS_CGETSET_DEF("text", GetText, SetText),
      JS_CGETSET_DEF("x", GetX, nullptr),
      JS_CGETSET_DEF("y", GetY, nullptr),
      JS_CGETSET_DEF("xsize", GetSizeX, nullptr),
      JS_CGETSET_DEF("ysize", GetSizeY, nullptr),
      JS_CGETSET_DEF("state", GetState, SetState),
      JS_CGETSET_DEF("password", GetPassword, nullptr),
      JS_CGETSET_DEF("type", GetType, nullptr),
      JS_CGETSET_DEF("cursorpos", GetCursorPos, SetCursorPos),
      JS_CGETSET_DEF("selectstart", GetSelectStart, nullptr),
      JS_CGETSET_DEF("selectend", GetSelectEnd, nullptr),
      JS_CGETSET_DEF("disabled", GetDisabled, SetDisabled),

      JS_FS("getNext", GetNext, 0, FUNCTION_FLAGS),
      JS_FS("click", Click, 0, FUNCTION_FLAGS),
      JS_FS("getText", FreeGetText, 0, FUNCTION_FLAGS),
      JS_FS("setText", FreeSetText, 1, FUNCTION_FLAGS),
  };

  uint32_t dwType;
  uint32_t dwX;
  uint32_t dwY;
  uint32_t dwSizeX;
  uint32_t dwSizeY;
};