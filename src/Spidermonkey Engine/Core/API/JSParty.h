#pragma once

#include "JSBaseObject.h"

#include "Game/D2Roster.h"  // D2RosterUnitStrc

class PartyWrap : public BaseObject {
 public:
  static JSObject* Instantiate(JSContext* ctx, D2RosterUnitStrc* unit);
  static void Initialize(JSContext* ctx, JS::HandleObject target);

  D2RosterUnitStrc* GetData();

 private:
  PartyWrap(JSContext* ctx, JS::HandleObject obj, D2RosterUnitStrc* unit);

  static void finalize(JSFreeOp* fop, JSObject* obj);

  // constructor
  static bool New(JSContext* ctx, JS::CallArgs& args);

  // properties
  static bool GetX(JSContext* ctx, JS::CallArgs& args);
  static bool GetY(JSContext* ctx, JS::CallArgs& args);
  static bool GetArea(JSContext* ctx, JS::CallArgs& args);
  static bool GetGid(JSContext* ctx, JS::CallArgs& args);
  static bool GetLife(JSContext* ctx, JS::CallArgs& args);
  static bool GetPartyFlag(JSContext* ctx, JS::CallArgs& args);
  static bool GetPartyId(JSContext* ctx, JS::CallArgs& args);
  static bool GetName(JSContext* ctx, JS::CallArgs& args);
  static bool GetClassId(JSContext* ctx, JS::CallArgs& args);
  static bool GetLevel(JSContext* ctx, JS::CallArgs& args);

  // functions
  static bool GetNext(JSContext* ctx, JS::CallArgs& args);

  // globals
  static bool GetParty(JSContext* ctx, JS::CallArgs& args);

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
      "Party",
      JSCLASS_HAS_RESERVED_SLOTS(kInternalFieldCount) | JSCLASS_FOREGROUND_FINALIZE,
      &m_ops,
  };

  D2RosterUnitStrc* pPresetUnit;
};
