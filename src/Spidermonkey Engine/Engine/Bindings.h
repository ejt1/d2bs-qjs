#pragma once

#include "js32.h"

typedef void (*D2BSClassRegisterFunc)(JSContext*, JS::HandleObject);

struct D2BSClass {
  const char* name;
  const char* filename;
  D2BSClassRegisterFunc cregister_func;
};

void D2BSClassRegister(JSContext* ctx, JS::HandleObject target, D2BSClass* c);

#define D2BS_BINDING_INTERNAL(name, regfunc)                 \
  static D2BSClass _d2bs_class_##name = {#name, __FILE__, regfunc}; \
  void _register##name(JSContext* ctx, JS::HandleObject target) {     \
    D2BSClassRegister(ctx, target, &_d2bs_class_##name);            \
  }

void RegisterBuiltinBindings(JSContext* ctx, JS::HandleObject target);
