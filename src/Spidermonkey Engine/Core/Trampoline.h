#pragma once

#include "js32.h"

typedef bool (*TrampolineFunction)(JSContext*, JS::CallArgs&);


// simple trampoline that turns JSNative into a function taking JS::CallArgs
template <TrampolineFunction Fx>
static bool trampoline(JSContext* ctx, unsigned argc, JS::Value* argv) {
  // TODO(ejt): give this function a better name
  JS::CallArgs args = JS::CallArgsFromVp(argc, argv);
  return Fx(ctx, args);
}
