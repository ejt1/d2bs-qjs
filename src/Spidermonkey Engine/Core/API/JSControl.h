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

  static bool New(JSContext* ctx, unsigned argc, JS::Value* vp);

  // properties
  static bool GetText(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetText(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetSizeX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetSizeY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetState(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetState(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetPassword(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetType(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetCursorPos(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetCursorPos(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetSelectStart(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetSelectEnd(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetDisabled(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool SetDisabled(JSContext* ctx, unsigned argc, JS::Value* vp);

  // functions
  static bool GetNext(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool Click(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool FreeGetText(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool FreeSetText(JSContext* ctx, unsigned argc, JS::Value* vp);

  // globals
  static bool GetControl(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetControls(JSContext* ctx, unsigned argc, JS::Value* vp);

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
