#pragma once

#include "js32.h"
#include "Profile.h"

class ProfileWrap {
 public:
  static JSValue Instantiate(JSContext* ctx, JSValue new_target, Profile* prof);
  static void Initialize(JSContext* ctx, JSValue target);

 private:
  ProfileWrap(JSContext* ctx, Profile* prof);
  virtual ~ProfileWrap();

  // constructor
  static JSValue New(JSContext* ctx, JSValue new_target, int argc, JSValue* argv);

  // properties
  static JSValue GetType(JSContext* ctx, JSValue this_val);
  static JSValue GetIP(JSContext* ctx, JSValue this_val);
  static JSValue GetUsername(JSContext* ctx, JSValue this_val);
  static JSValue GetGateway(JSContext* ctx, JSValue this_val);
  static JSValue GetCharacter(JSContext* ctx, JSValue this_val);
  static JSValue GetDifficulty(JSContext* ctx, JSValue this_val);
  static JSValue GetMaxLoginTime(JSContext* ctx, JSValue this_val);
  static JSValue GetMaxCharacterSelectTime(JSContext* ctx, JSValue this_val);

  // functions
  static JSValue Login(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  static inline JSClassID m_class_id = 0;
  static inline JSCFunctionListEntry m_proto_funcs[] = {
      JS_CGETSET_DEF("type", GetType, nullptr),                                      //
      JS_CGETSET_DEF("ip", GetIP, nullptr),                                          //
      JS_CGETSET_DEF("username", GetUsername, nullptr),                              //
      JS_CGETSET_DEF("gateway", GetGateway, nullptr),                                //
      JS_CGETSET_DEF("character", GetCharacter, nullptr),                            //
      JS_CGETSET_DEF("difficulty", GetDifficulty, nullptr),                          //
      JS_CGETSET_DEF("maxLoginTime", GetMaxLoginTime, nullptr),                      //
      JS_CGETSET_DEF("maxCharacterSelectTime", GetMaxCharacterSelectTime, nullptr),  //

      JS_FS("login", Login, 0, FUNCTION_FLAGS),
  };

  Profile* profile;
};