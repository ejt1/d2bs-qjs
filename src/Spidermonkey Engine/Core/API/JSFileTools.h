#pragma once

#include "js32.h"

class FileToolsWrap {
 public:
  static void Initialize(JSContext* ctx, JS::HandleObject target);

 private:
  static bool New(JSContext* ctx, unsigned argc, JS::Value* vp);

  // static functions
  static bool Remove(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool Rename(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool Copy(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool Exists(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool ReadText(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool WriteText(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool AppendText(JSContext* ctx, unsigned argc, JS::Value* vp);

  static inline JSClass m_class = {"FileTools"};
};
