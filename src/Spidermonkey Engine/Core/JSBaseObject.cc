#include "JSBaseObject.h"

BaseObject::BaseObject(JSContext* /*ctx*/, JS::HandleObject obj) {
  JS_SetReservedSlot(obj, kSlot, JS::PrivateValue(this));
}

BaseObject::~BaseObject() {
}

BaseObject* BaseObject::FromJSObject(JSObject* object) {
  JS::Value val = JS_GetReservedSlot(object, kSlot);
  if (val.isDouble()) {
    return static_cast<BaseObject*>(val.toPrivate());
  }
  return nullptr;
}

BaseObject* BaseObject::FromJSObject(JS::HandleValue object) {
  return FromJSObject(object.toObjectOrNull());
}
