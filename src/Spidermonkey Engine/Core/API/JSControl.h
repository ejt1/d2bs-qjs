#pragma once

#include "Control.h"
#include "JSBaseObject.h"

class ControlWrap : public BaseObject {
 public:
  static JSObject* Instantiate(JSContext* ctx, D2WinControlStrc* control);
  static void Initialize(JSContext* ctx, JS::HandleObject target);

 private:
  ControlWrap(JSContext* ctx, JS::HandleObject obj, D2WinControlStrc* control);

  static void finalize(JSFreeOp* fop, JSObject* obj);

  static bool New(JSContext* ctx, JS::CallArgs& args);

  // properties
  static bool GetText(JSContext* ctx, JS::CallArgs& args);
  static bool SetText(JSContext* ctx, JS::CallArgs& args);
  static bool GetX(JSContext* ctx, JS::CallArgs& args);
  static bool GetY(JSContext* ctx, JS::CallArgs& args);
  static bool GetSizeX(JSContext* ctx, JS::CallArgs& args);
  static bool GetSizeY(JSContext* ctx, JS::CallArgs& args);
  static bool GetState(JSContext* ctx, JS::CallArgs& args);
  static bool SetState(JSContext* ctx, JS::CallArgs& args);
  static bool GetPassword(JSContext* ctx, JS::CallArgs& args);
  static bool GetType(JSContext* ctx, JS::CallArgs& args);
  static bool GetCursorPos(JSContext* ctx, JS::CallArgs& args);
  static bool SetCursorPos(JSContext* ctx, JS::CallArgs& args);
  static bool GetSelectStart(JSContext* ctx, JS::CallArgs& args);
  static bool GetSelectEnd(JSContext* ctx, JS::CallArgs& args);
  static bool GetDisabled(JSContext* ctx, JS::CallArgs& args);
  static bool SetDisabled(JSContext* ctx, JS::CallArgs& args);

  // functions
  static bool GetNext(JSContext* ctx, JS::CallArgs& args);
  static bool Click(JSContext* ctx, JS::CallArgs& args);
  static bool FreeGetText(JSContext* ctx, JS::CallArgs& args);
  static bool FreeSetText(JSContext* ctx, JS::CallArgs& args);

  // globals
  static bool GetControl(JSContext* ctx, JS::CallArgs& args);
  static bool GetControls(JSContext* ctx, JS::CallArgs& args);

  static inline JSClassOps m_ops = {
      .addProperty = nullptr,
      .delProperty = nullptr,
      .enumerate = nullptr,
      .newEnumerate = nullptr,
      .resolve = nullptr,
      .mayResolve = nullptr,
      .finalize = finalize,
      .call = nullptr,
      .hasInstance = nullptr,
      .construct = nullptr,
      .trace = nullptr,
  };
  static inline JSClass m_class = {
      "Control",
      JSCLASS_HAS_RESERVED_SLOTS(kInternalFieldCount) | JSCLASS_FOREGROUND_FINALIZE,
      &m_ops,
  };

  uint32_t dwType;
  uint32_t dwX;
  uint32_t dwY;
  uint32_t dwSizeX;
  uint32_t dwSizeY;
};

#define UNWRAP_CONTROL_OR_ERROR(ctx, ptr, obj)                                                                 \
  ControlWrap* wrap;                                                                                           \
  UNWRAP_OR_RETURN(ctx, &wrap, obj)                                                                            \
  *ptr = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY); \
  if (!*ptr) {                                                                                                 \
    THROW_ERROR(ctx, "Unable to get Control");                                                                 \
  }
