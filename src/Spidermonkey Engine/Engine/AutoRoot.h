#pragma once

#ifndef __AUTOROOT_H__
#define __AUTOROOT_H__

#include "js32.h"

class AutoRoot {
 private:
  JSValue var;
  int count;
  JSContext* cx;
  AutoRoot(const AutoRoot&);

 public:
  AutoRoot() : var(JS_NULL), count(0) {
  }
  AutoRoot(JSContext* cx, JSValue var);
  ~AutoRoot();
  void Take();
  void Release();
  JSValue* value() {
    return &var;
  }
  JSValue operator*() {
    return *value();
  }
  bool operator==(AutoRoot& other) {
    return value() == other.value();
  }
};

#endif
