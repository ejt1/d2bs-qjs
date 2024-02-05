#pragma once

#include "GAME/DataTbls/SkillsTbls.h"

// rename D2SkillStrc
struct Skill {
  SkillInfo* pSkillInfo;  // 0x00
  Skill* pNextSkill;      // 0x04
  DWORD _1[8];            // 0x08
  DWORD dwSkillLevel;     // 0x28
  DWORD _2[2];            // 0x2C
  DWORD ItemId;           // 0x34 0xFFFFFFFF if not a charge
  DWORD ChargesLeft;      // 0x38
  DWORD IsCharge;         // 0x3C 1 for charge, else 0
};                        // size = 0x40

// rename D2SkillListStrc
struct Info {
  BYTE* pGame1C;       // 0x00
  Skill* pFirstSkill;  // 0x04
  Skill* pLeftSkill;   // 0x08
  Skill* pRightSkill;  // 0x0C
};