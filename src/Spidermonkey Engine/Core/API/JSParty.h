#pragma once

#include "js32.h"

#include "Game/D2Roster.h" // D2RosterUnitStrc

class PartyWrap {
 public:
  static JSValue Instantiate(JSContext* ctx, JSValue new_target, D2RosterUnitStrc* unit);
  static void Initialize(JSContext* ctx, JSValue target);

 private:
  PartyWrap(JSContext* ctx, D2RosterUnitStrc* unit);

  // constructor
  static JSValue New(JSContext* ctx, JSValue new_target, int argc, JSValue* argv);

  // properties
  static JSValue GetX(JSContext* ctx, JSValue this_val);
  static JSValue GetY(JSContext* ctx, JSValue this_val);
  static JSValue GetArea(JSContext* ctx, JSValue this_val);
  static JSValue GetGid(JSContext* ctx, JSValue this_val);
  static JSValue GetLife(JSContext* ctx, JSValue this_val);
  static JSValue GetPartyFlag(JSContext* ctx, JSValue this_val);
  static JSValue GetPartyId(JSContext* ctx, JSValue this_val);
  static JSValue GetName(JSContext* ctx, JSValue this_val);
  static JSValue GetClassId(JSContext* ctx, JSValue this_val);
  static JSValue GetLevel(JSContext* ctx, JSValue this_val);

  // functions
  static JSValue GetNext(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  // globals
  static JSValue GetParty(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);

  static inline JSClassID m_class_id = 0;
  static inline JSCFunctionListEntry m_proto_funcs[] = {
      JS_CGETSET_DEF("x", GetX, nullptr),
      JS_CGETSET_DEF("y", GetY, nullptr),
      JS_CGETSET_DEF("area", GetArea, nullptr),
      JS_CGETSET_DEF("gid", GetGid, nullptr),
      JS_CGETSET_DEF("life", GetLife, nullptr),
      JS_CGETSET_DEF("partyflag", GetPartyFlag, nullptr),
      JS_CGETSET_DEF("partyid", GetPartyId, nullptr),
      JS_CGETSET_DEF("name", GetName, nullptr),
      JS_CGETSET_DEF("classid", GetClassId, nullptr),
      JS_CGETSET_DEF("level", GetLevel, nullptr),

      JS_FS("getNext", GetNext, 0, FUNCTION_FLAGS),
  };

  D2RosterUnitStrc* pPresetUnit;
};