#pragma once

#include "GAME/DataTbls/SkillsTbls.h"

struct D2SkillStrc {
  D2SkillsTxt* pSkillInfo;    // 0x00
  D2SkillStrc* pNextSkill;  // 0x04
  DWORD _1[8];              // 0x08
  DWORD dwSkillLevel;       // 0x28
  DWORD _2[2];              // 0x2C
  DWORD ItemId;             // 0x34 0xFFFFFFFF if not a charge
  DWORD ChargesLeft;        // 0x38
  DWORD IsCharge;           // 0x3C 1 for charge, else 0
};                          // size = 0x40

struct D2SkillListStrc {
  BYTE* pGame1C;             // 0x00
  D2SkillStrc* pFirstSkill;  // 0x04
  D2SkillStrc* pLeftSkill;   // 0x08
  D2SkillStrc* pRightSkill;  // 0x0C
};