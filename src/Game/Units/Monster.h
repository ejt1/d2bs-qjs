#pragma once

#include <cstdint>

struct D2MonsterDataStrc {
  uint8_t _1[22];  // 0x00
  struct {
    uint8_t fUnk : 1;
    uint8_t fNormal : 1;
    uint8_t fChamp : 1;
    uint8_t fBoss : 1;
    uint8_t fMinion : 1;
  } UFlags;  // 0x16
  uint8_t _2[5];
  uint8_t anEnchants[9];  // 0x1C
  uint16_t wUniqueNo;     // 0x26
  uint32_t _5;            // 0x28
  struct {
    wchar_t wName[28];
  } UName;  // 0x2C
};