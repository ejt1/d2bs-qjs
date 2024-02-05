#pragma once

#include "GAME/DataTbls/SkillsTbls.h"

#include <cstdint>

struct D2SkillStrc {
  D2SkillsTxt* pSkillInfo;  // 0x00
  D2SkillStrc* pNextSkill;  // 0x04
  uint32_t _1[8];           // 0x08
  uint32_t dwSkillLevel;    // 0x28
  uint32_t _2[2];           // 0x2C
  uint32_t ItemId;          // 0x34 0xFFFFFFFF if not a charge
  uint32_t ChargesLeft;     // 0x38
  uint32_t IsCharge;        // 0x3C 1 for charge, else 0
};                          // size = 0x40

struct D2SkillListStrc {
  uint8_t* pGame1C;          // 0x00
  D2SkillStrc* pFirstSkill;  // 0x04
  D2SkillStrc* pLeftSkill;   // 0x08
  D2SkillStrc* pRightSkill;  // 0x0C
};