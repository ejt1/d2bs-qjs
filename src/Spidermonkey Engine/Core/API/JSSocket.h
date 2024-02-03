#ifndef __JSSOCKET_H__
#define __JSSOCKET_H__

#include "js32.h"

//////////////////////////////////////////////////////////////////
// socket stuff
//////////////////////////////////////////////////////////////////

CLASS_CTOR(socket);
CLASS_FINALIZER(socket);

JSAPI_PROP(socket_getProperty);
JSAPI_STRICT_PROP(socket_setProperty);

JSAPI_FUNC(socket_open);
JSAPI_FUNC(socket_close);
JSAPI_FUNC(socket_send);
JSAPI_FUNC(socket_read);

enum { SOCKET_READABLE, SOCKET_WRITEABLE };

static JSCFunctionListEntry socket_methods[] = {
    JS_FN("close", socket_close, 0, FUNCTION_FLAGS),
    JS_FN("send", socket_send, 1, FUNCTION_FLAGS),
    JS_FN("read", socket_read, 0, FUNCTION_FLAGS),
};

static JSCFunctionListEntry socket_s_methods[] = {
    JS_FN("open", socket_open, 2, FUNCTION_FLAGS),
};

static JSCFunctionListEntry socket_props[] = {
    JS_CGETSET_MAGIC_DEF("readable", socket_getProperty, nullptr, SOCKET_READABLE),
    JS_CGETSET_MAGIC_DEF("writeable", socket_getProperty, nullptr, SOCKET_WRITEABLE),
};

#endif
