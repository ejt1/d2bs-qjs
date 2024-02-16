#pragma once

#include "js32.h"

typedef bool (*TrampolineFunction)(JSContext*, JS::CallArgs&);

// simple trampoline that turns JSNative into a function taking JS::CallArgs
template <TrampolineFunction Fx>
static bool __stdcall trampoline(JSContext* ctx, unsigned argc, JS::Value* argv) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, argv);
  return Fx(ctx, args);
}
