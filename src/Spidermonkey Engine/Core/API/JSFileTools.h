#pragma once

#include "Trampoline.h"

class FileToolsWrap {
 public:
  static void Initialize(JSContext* ctx, JS::HandleObject target);

 private:
  static bool New(JSContext* ctx, JS::CallArgs& args);

  // static functions
  static bool Remove(JSContext* ctx, JS::CallArgs& args);
  static bool Rename(JSContext* ctx, JS::CallArgs& args);
  static bool Copy(JSContext* ctx, JS::CallArgs& args);
  static bool Exists(JSContext* ctx, JS::CallArgs& args);
  static bool ReadText(JSContext* ctx, JS::CallArgs& args);
  static bool WriteText(JSContext* ctx, JS::CallArgs& args);
  static bool AppendText(JSContext* ctx, JS::CallArgs& args);

  static inline JSClass m_class = {"FileTools"};
};
