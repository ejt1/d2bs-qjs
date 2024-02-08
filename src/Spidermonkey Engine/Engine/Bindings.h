#pragma once

#include "js32.h"

typedef void (*D2BSClassRegisterFunc)(JSContext*, JSValue);

struct D2BSClass {
  const char* name;
  const char* filename;
  D2BSClassRegisterFunc cregister_func;
};

void D2BSClassRegister(JSContext* ctx, JSValue target, D2BSClass* c);

#define D2BS_BINDING_INTERNAL(name, regfunc)                 \
  static D2BSClass _d2bs_class = {#name, __FILE__, regfunc}; \
  void _register##name(JSContext* ctx, JSValue target) {     \
    D2BSClassRegister(ctx, target, &_d2bs_class);            \
  }

void RegisterBuiltinBindings(JSContext* ctx);