#ifndef JSPROFILE_H
#define JSPROFILE_H

#include <windows.h>
#include "js32.h"
#include "Profile.h"

CLASS_CTOR(profile);
CLASS_FINALIZER(profile);

JSAPI_FUNC(profile_login);

JSAPI_PROP(profile_getProperty);

struct jsProfilePrivate {
  char* password;
};

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

static JSCFunctionListEntry profile_props[] = {
    JS_CGETSET_MAGIC_DEF("type", profile_getProperty, nullptr, PROFILE_TYPE),
    JS_CGETSET_MAGIC_DEF("ip", profile_getProperty, nullptr, PROFILE_IP),
    JS_CGETSET_MAGIC_DEF("username", profile_getProperty, nullptr, PROFILE_USERNAME),
    JS_CGETSET_MAGIC_DEF("gateway", profile_getProperty, nullptr, PROFILE_GATEWAY),
    JS_CGETSET_MAGIC_DEF("character", profile_getProperty, nullptr, PROFILE_CHARACTER),
    JS_CGETSET_MAGIC_DEF("difficulty", profile_getProperty, nullptr, PROFILE_DIFFICULTY),
    JS_CGETSET_MAGIC_DEF("maxLoginTime", profile_getProperty, nullptr, PROFILE_MAXLOGINTIME),
    JS_CGETSET_MAGIC_DEF("maxCharacterSelectTime", profile_getProperty, nullptr, PROFILE_MAXCHARSELTIME),
};

static JSCFunctionListEntry profile_methods[] = {
    JS_FS("login", profile_login, 0, FUNCTION_FLAGS),
};

CLASS_CTOR(profileType);

JSAPI_PROP(profileType_getProperty);

static JSCFunctionListEntry profileType_props[] = {
    JS_CGETSET_MAGIC_DEF("singlePlayer", profileType_getProperty, nullptr, PROFILETYPE_SINGLEPLAYER),
    JS_CGETSET_MAGIC_DEF("battleNet", profileType_getProperty, nullptr, PROFILETYPE_BATTLENET),
    JS_CGETSET_MAGIC_DEF("openBattleNet", profileType_getProperty, nullptr, PROFILETYPE_OPEN_BATTLENET),
    JS_CGETSET_MAGIC_DEF("tcpIpHost", profileType_getProperty, nullptr, PROFILETYPE_TCPIP_HOST),
    JS_CGETSET_MAGIC_DEF("tcpIpJoin", profileType_getProperty, nullptr, PROFILETYPE_TCPIP_JOIN),
};

#endif