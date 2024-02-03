#ifndef PARTY_H
#define PARTY_H

#include "js32.h"

CLASS_CTOR(party);
JSAPI_PROP(party_getProperty);

JSAPI_FUNC(party_getNext);

JSAPI_FUNC(my_getParty);

enum party_tinyid {
  PARTY_AREA,
  PARTY_X,
  PARTY_Y,
  PARTY_GID,
  PARTY_LIFE,
  PARTY_NAME,
  PARTY_FLAG,
  PARTY_ID,
  PARTY_CLASSID,
  PARTY_LEVEL,
};

static JSCFunctionListEntry party_props[] = {
    JS_CGETSET_MAGIC_DEF("x", party_getProperty, nullptr, PARTY_X),
    JS_CGETSET_MAGIC_DEF("y", party_getProperty, nullptr, PARTY_Y),
    JS_CGETSET_MAGIC_DEF("area", party_getProperty, nullptr, PARTY_AREA),
    JS_CGETSET_MAGIC_DEF("gid", party_getProperty, nullptr, PARTY_GID),
    JS_CGETSET_MAGIC_DEF("life", party_getProperty, nullptr, PARTY_LIFE),
    JS_CGETSET_MAGIC_DEF("partyflag", party_getProperty, nullptr, PARTY_FLAG),
    JS_CGETSET_MAGIC_DEF("partyid", party_getProperty, nullptr, PARTY_ID),
    JS_CGETSET_MAGIC_DEF("name", party_getProperty, nullptr, PARTY_NAME),
    JS_CGETSET_MAGIC_DEF("classid", party_getProperty, nullptr, PARTY_CLASSID),
    JS_CGETSET_MAGIC_DEF("level", party_getProperty, nullptr, PARTY_LEVEL),
};

static JSCFunctionListEntry party_methods[] = {
    JS_FS("getNext", party_getNext, 0, FUNCTION_FLAGS),
};

#endif