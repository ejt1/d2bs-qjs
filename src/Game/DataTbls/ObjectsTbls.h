#pragma once

#include <cstdint>

struct D2ObjectsTxt {
  char szName[0x40];      // 0x00
  wchar_t wszName[0x40];  // 0x40
  uint8_t _1[4];          // 0xC0
  uint8_t nSelectable0;   // 0xC4
  uint8_t _2[0x87];       // 0xC5
  uint8_t nOrientation;   // 0x14C
  uint8_t _2b[0x19];      // 0x14D
  uint8_t nSubClass;      // 0x166
  uint8_t _3[0x11];       // 0x167
  uint8_t nParm0;         // 0x178
  uint8_t _4[0x39];       // 0x179
  uint8_t nPopulateFn;    // 0x1B2
  uint8_t nOperateFn;     // 0x1B3
  uint8_t _5[8];          // 0x1B4
  uint32_t nAutoMap;      // 0x1BB
};

inline D2ObjectsTxt*(__stdcall* D2COMMON_GetObjectText)(uint32_t objno) = nullptr;