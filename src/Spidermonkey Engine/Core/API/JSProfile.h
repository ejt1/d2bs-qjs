#pragma once

#include "JSBaseObject.h"
#include "Profile.h"

class ProfileWrap : public BaseObject {
 public:
  static JSObject* Instantiate(JSContext* ctx, Profile* prof);
  static void Initialize(JSContext* ctx, JS::HandleObject target);

 private:
  ProfileWrap(JSContext* ctx, JS::HandleObject obj, Profile* prof);
  virtual ~ProfileWrap();

  static void finalize(JSFreeOp* fop, JSObject* obj);

  // constructor
  static bool New(JSContext* ctx, JS::CallArgs& args);

  // properties
  static bool GetType(JSContext* ctx, JS::CallArgs& args);
  static bool GetIP(JSContext* ctx, JS::CallArgs& args);
  static bool GetUsername(JSContext* ctx, JS::CallArgs& args);
  static bool GetGateway(JSContext* ctx, JS::CallArgs& args);
  static bool GetCharacter(JSContext* ctx, JS::CallArgs& args);
  static bool GetDifficulty(JSContext* ctx, JS::CallArgs& args);
  static bool GetMaxLoginTime(JSContext* ctx, JS::CallArgs& args);
  static bool GetMaxCharacterSelectTime(JSContext* ctx, JS::CallArgs& args);

  // functions
  static bool Login(JSContext* ctx, JS::CallArgs& args);

  static inline JSClassOps m_ops = {
      .addProperty = nullptr,
      .delProperty = nullptr,
      .enumerate = nullptr,
      .newEnumerate = nullptr,
      .resolve = nullptr,
      .mayResolve = nullptr,
      .finalize = finalize,
      .call = nullptr,
      .hasInstance = nullptr,
      .construct = nullptr,
      .trace = nullptr,
  };
  static inline JSClass m_class = {
      "Profile",
      JSCLASS_HAS_RESERVED_SLOTS(kInternalFieldCount) | JSCLASS_FOREGROUND_FINALIZE,
      &m_ops,
  };

  Profile* profile;
};
