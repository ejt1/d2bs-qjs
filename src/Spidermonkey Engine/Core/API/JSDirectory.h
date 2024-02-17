#pragma once

#include "JSBaseObject.h"

class DirectoryWrap : public BaseObject {
 public:
  static JSObject* Instantiate(JSContext* ctx, const char* name);
  static void Initialize(JSContext* ctx, JS::HandleObject target);

 private:
  DirectoryWrap(JSContext* ctx, JS::HandleObject obj, const char* name);

  static void finalize(JSFreeOp* fop, JSObject* obj);

  static bool New(JSContext* ctx, JS::CallArgs& args);

  // properties
  static bool GetName(JSContext* ctx, JS::CallArgs& args);

  // functions
  static bool Create(JSContext* ctx, JS::CallArgs& args);
  static bool Delete(JSContext* ctx, JS::CallArgs& args);
  static bool GetFiles(JSContext* ctx, JS::CallArgs& args);
  static bool GetFolders(JSContext* ctx, JS::CallArgs& args);

  // globals
  static bool OpenDirectory(JSContext* ctx, JS::CallArgs& args);

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
      "Folder",
      JSCLASS_HAS_RESERVED_SLOTS(kInternalFieldCount) | JSCLASS_FOREGROUND_FINALIZE,
      &m_ops,
  };

  char szName[_MAX_PATH];
};
