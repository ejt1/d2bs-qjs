#pragma once

#include "js32.h"

class FileToolsWrap {
 public:
  static JSValue Instantiate(JSContext* ctx, JSValue new_target);
  static void Initialize(JSContext* ctx, JSValue target);

 private:
  // static functions
  static JSValue Remove(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue Rename(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue Copy(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue Exists(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue ReadText(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue WriteText(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue AppendText(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  static inline JSClassID m_class_id = 0;
  static inline JSCFunctionListEntry m_static_funcs[] = {
      JS_FS("remove", Remove, 1, FUNCTION_FLAGS),          //
      JS_FS("rename", Rename, 2, FUNCTION_FLAGS),          //
      JS_FS("copy", Copy, 2, FUNCTION_FLAGS),              //
      JS_FS("exists", Exists, 1, FUNCTION_FLAGS),          //
      JS_FS("readText", ReadText, 1, FUNCTION_FLAGS),      //
      JS_FS("writeText", WriteText, 2, FUNCTION_FLAGS),    //
      JS_FS("appendText", AppendText, 2, FUNCTION_FLAGS),  //
  };
};