#pragma once

#include <cstdint>

struct D2LevelsTxt {
  uint32_t dwLevelNo;         // 0x00
  uint32_t _1[60];            // 0x04
  uint8_t _2;                 // 0xF4
  char szName[40];            // 0xF5
  char szEntranceText[40];    // 0x11D
  char szLevelDesc[41];       // 0x145
  wchar_t wName[40];          // 0x16E
  wchar_t wEntranceText[40];  // 0x1BE
  uint8_t nObjGroup[8];       // 0x196
  uint8_t nObjPrb[8];         // 0x19E
};

// rename D2LevelDefBin
struct D2LevelDefBin {
  uint32_t _1[2];     // 0x00
  uint32_t nLayerNo;  // 0x08
};