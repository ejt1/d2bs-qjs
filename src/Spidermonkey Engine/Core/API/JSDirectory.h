#pragma once

#include "JSBaseObject.h"

class DirectoryWrap : public BaseObject {
 public:
  static JSObject* Instantiate(JSContext* ctx, const char* name);
  static void Initialize(JSContext* ctx, JS::HandleObject target);

 private:
  DirectoryWrap(JSContext* ctx, JS::HandleObject obj, const char* name);

  static void finalize(JSFreeOp* fop, JSObject* obj);

  static bool New(JSContext* ctx, unsigned argc, JS::Value* vp);

  // properties
  static bool GetName(JSContext* ctx, unsigned argc, JS::Value* vp);

  // functions
  static bool Create(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool Delete(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetFiles(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetFolders(JSContext* ctx, unsigned argc, JS::Value* vp);

  // globals
  static bool OpenDirectory(JSContext* ctx, unsigned argc, JS::Value* vp);

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
