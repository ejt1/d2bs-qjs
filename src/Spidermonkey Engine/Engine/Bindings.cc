#include "Bindings.h"

#define D2BS_LEGACY_BINDINGS(V) \
  V(AreaWrap)                   \
  V(ControlWrap)                \
  V(DirectoryWrap)              \
  V(ExitWrap)                   \
  /*V(FileWrap)*/               \
  V(FileToolsWrap)              \
  V(PartyWrap)                  \
  V(PresetUnitWrap)             \
  V(ProfileWrap)                \
  V(RoomWrap)                   \
  /*V(SocketWrap)*/             \
  V(ScriptWrap)                 \
  V(UnitWrap)                   \
  /* ScreenHooks */             \
  V(FrameWrap)                  \
  V(BoxWrap)                    \
  V(LineWrap)                   \
  V(TextWrap)                   \
  V(ImageWrap)

#define D2BS_BUILTIN_BINDINGS(V) V(TimerWrap)

#define V(name) void _register##name(JSContext* ctx, JS::HandleObject target);
D2BS_LEGACY_BINDINGS(V)
D2BS_BUILTIN_BINDINGS(V)
#undef V

void D2BSClassRegister(JSContext* ctx, JS::HandleObject target, D2BSClass* c) {
  if (c->cregister_func) {
    c->cregister_func(ctx, target);
  }
}

void RegisterBuiltinBindings(JSContext* ctx, JS::HandleObject target) {
#define V(name) _register##name(ctx, target);
  D2BS_LEGACY_BINDINGS(V)
  D2BS_BUILTIN_BINDINGS(V)
#undef V
}
