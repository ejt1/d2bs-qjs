#pragma once

#include "js32.h"

class SocketWrap {
 public:
  static JSValue Instantiate(JSContext* ctx, JSValue new_target);
  static void Initialize(JSContext* ctx, JSValue target);

 private:
  SocketWrap(JSContext* ctx);

  // properties
  static JSValue GetReadable(JSContext* ctx, JSValue this_val);
  static JSValue GetWriteable(JSContext* ctx, JSValue this_val);

  // functions
  static JSValue Close(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue Send(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue Read(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  // static functions
  static JSValue Open(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  static inline JSClassID m_class_id = 0;
  static inline JSCFunctionListEntry m_proto_funcs[] = {
      JS_CGETSET_DEF("readable", GetReadable, nullptr),
      JS_CGETSET_DEF("writeable", GetWriteable, nullptr),

      JS_FN("close", Close, 0, FUNCTION_FLAGS),
      JS_FN("send", Send, 1, FUNCTION_FLAGS),
      JS_FN("read", Read, 0, FUNCTION_FLAGS),
  };

  static inline JSCFunctionListEntry m_static_funcs[] = {
      JS_FN("open", Open, 2, FUNCTION_FLAGS),
  };
};