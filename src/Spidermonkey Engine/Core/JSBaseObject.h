#pragma once

#include "Trampoline.h"

class BaseObject {
 public:
  enum InternalFields {
    // reserved for future use
    kReserved1,
    kReserved2,

    // stores the internal pointer of BaseObject*
    kSlot, 

    // max slots for BaseObject, derived types are free to reserve additional slots after this
    kInternalFieldCount, 
  };

  BaseObject() = delete;
  BaseObject(JSContext* ctx, JS::HandleObject obj);
  virtual ~BaseObject();

  static BaseObject* FromJSObject(JSObject* object);
  static BaseObject* FromJSObject(JS::HandleValue object);
  template <typename T>
  static T* FromJSObject(JS::HandleValue object);
};

template <typename T>
inline T* BaseObject::FromJSObject(JS::HandleValue object) {
  return static_cast<T*>(FromJSObject(object));
}

#define UNWRAP_OR_RETURN(ctx, ptr, obj)                                                                    \
  *ptr = static_cast<typename std::remove_reference<decltype(*ptr)>::type>(BaseObject::FromJSObject(obj)); \
  if (!(*ptr)) {                                                                                           \
    JS_ReportErrorASCII(ctx, "failed to unwrap");                                                          \
    return false;                                                                                          \
  }
