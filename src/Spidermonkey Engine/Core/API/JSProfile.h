#pragma once

#include <windows.h>
#include "js32.h"
#include "Profile.h"

CLASS_CTOR(profile);
CLASS_FINALIZER(profile);

JSAPI_FUNC(profile_login);

JSAPI_PROP(profile_getProperty);

enum jsProfileProperty_ids {
  PROFILE_TYPE,
  PROFILE_IP,
  PROFILE_USERNAME,
  PROFILE_GATEWAY,
  PROFILE_CHARACTER,
  PROFILE_DIFFICULTY,
  PROFILE_MAXLOGINTIME,
  PROFILE_MAXCHARSELTIME
};

static JSCFunctionListEntry profile_proto_funcs[] = {
    JS_CGETSET_MAGIC_DEF("type", profile_getProperty, nullptr, PROFILE_TYPE),
    JS_CGETSET_MAGIC_DEF("ip", profile_getProperty, nullptr, PROFILE_IP),
    JS_CGETSET_MAGIC_DEF("username", profile_getProperty, nullptr, PROFILE_USERNAME),
    JS_CGETSET_MAGIC_DEF("gateway", profile_getProperty, nullptr, PROFILE_GATEWAY),
    JS_CGETSET_MAGIC_DEF("character", profile_getProperty, nullptr, PROFILE_CHARACTER),
    JS_CGETSET_MAGIC_DEF("difficulty", profile_getProperty, nullptr, PROFILE_DIFFICULTY),
    JS_CGETSET_MAGIC_DEF("maxLoginTime", profile_getProperty, nullptr, PROFILE_MAXLOGINTIME),
    JS_CGETSET_MAGIC_DEF("maxCharacterSelectTime", profile_getProperty, nullptr, PROFILE_MAXCHARSELTIME),

    JS_FS("login", profile_login, 0, FUNCTION_FLAGS),
};