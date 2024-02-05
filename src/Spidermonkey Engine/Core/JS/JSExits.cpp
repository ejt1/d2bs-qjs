#include "JSExits.h"

EMPTY_CTOR(exit)

CLASS_FINALIZER(exit) {
  JSExit* pExit = (JSExit*)JS_GetOpaque3(val);
  delete pExit;
}

JSAPI_PROP(exit_getProperty) {
  JSExit* pExit = (JSExit*)JS_GetOpaque3(this_val);

  if (!pExit)
    return JS_UNDEFINED;

  switch (magic) {
    case EXIT_X:
      return JS_NewUint32(ctx, pExit->x);
      break;
    case EXIT_Y:
      return JS_NewUint32(ctx, pExit->y);
      break;
    case EXIT_TARGET:
      return JS_NewUint32(ctx, pExit->id);
      break;
    case EXIT_TYPE:
      return JS_NewUint32(ctx, pExit->type);
      break;
    case EXIT_TILEID:
      return JS_NewUint32(ctx, pExit->tileid);
      break;
    case EXIT_LEVELID:
      return JS_NewUint32(ctx, pExit->level);
      break;
    default:
      break;
  }

  return JS_UNDEFINED;
}
