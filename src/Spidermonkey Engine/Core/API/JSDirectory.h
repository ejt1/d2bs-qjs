#pragma once

#include "js32.h"

class DirectoryWrap {
 public:
  static JSValue Instantiate(JSContext* ctx, JSValue new_target, const char* name);
  static void Initialize(JSContext* ctx, JSValue target);

 private:
  DirectoryWrap(JSContext* ctx, const char* name);

  // properties
  static JSValue GetName(JSContext* ctx, JSValue this_val);

  // functions
  static JSValue Create(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue Delete(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue GetFiles(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
  static JSValue GetFolders(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  // globals
  static JSValue OpenDirectory(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  static inline JSClassID m_class_id = 0;
  static inline JSCFunctionListEntry m_proto_funcs[] = {
      JS_CGETSET_DEF("name", GetName, nullptr),

      JS_FS("create", Create, 1, FUNCTION_FLAGS),          //
      JS_FS("remove", Delete, 1, FUNCTION_FLAGS),          //
      JS_FS("getFiles", GetFiles, 1, FUNCTION_FLAGS),      //
      JS_FS("getFolders", GetFolders, 1, FUNCTION_FLAGS),  //
  };

  char szName[_MAX_PATH];
};