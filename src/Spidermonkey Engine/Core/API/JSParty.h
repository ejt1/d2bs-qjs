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
  static bool New(JSContext* ctx, unsigned argc, JS::Value* vp);

  // properties
  static bool GetX(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetY(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetArea(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetGid(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetLife(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetPartyFlag(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetPartyId(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetName(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetClassId(JSContext* ctx, unsigned argc, JS::Value* vp);
  static bool GetLevel(JSContext* ctx, unsigned argc, JS::Value* vp);

  // functions
  static bool GetNext(JSContext* ctx, unsigned argc, JS::Value* vp);

  // globals
  static bool GetParty(JSContext* ctx, unsigned argc, JS::Value* vp);

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
