#include "Bindings.h"

#define D2BS_BUILTIN_BINDINGS(V) \
  V(AreaWrap)                    \
  V(ControlWrap)                 \
  V(DirectoryWrap)               \
  V(ExitWrap)                    \
  V(FileWrap)                    \
  V(FileToolsWrap)               \
  V(PartyWrap)                   \
  V(PresetUnitWrap)              \
  V(ProfileWrap)                 \
  V(RoomWrap)                    \
  V(SocketWrap)                  \
  V(ScriptWrap)                  \
  /* ScreenHooks */              \
  V(FrameWrap)                   \
  V(BoxWrap)                     \
  V(LineWrap)                    \
  V(TextWrap)                    \
  V(ImageWrap)

#define V(name) void _register##name(JSContext* ctx, JSValue target);
D2BS_BUILTIN_BINDINGS(V)
#undef V

void D2BSClassRegister(JSContext* ctx, JSValue target, D2BSClass* c) {
  if (c->cregister_func) {
    c->cregister_func(ctx, target);
  }
}

void RegisterBuiltinBindings(JSContext* ctx) {
  JSValue global_obj = JS_GetGlobalObject(ctx);
#define V(name) _register##name(ctx, global_obj);
  D2BS_BUILTIN_BINDINGS(V)
#undef V
  JS_FreeValue(ctx, global_obj);
}
