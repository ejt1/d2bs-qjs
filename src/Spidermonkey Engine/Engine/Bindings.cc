#include "Bindings.h"

#include "D2Helpers.h"  // Log

#define D2BS_BUILTIN_BINDINGS(V) \
  V(AreaWrap)                    \
  V(ScriptWrap)

#define V(name) void _register##name(JSContext* ctx, JSValue target);
D2BS_BUILTIN_BINDINGS(V)
#undef V

void D2BSClassRegister(JSContext* ctx, JSValue target, D2BSClass* c) {
  Log("Bind %s", c->name);
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
