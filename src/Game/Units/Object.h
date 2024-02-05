#pragma once

#include <cstdint>

struct D2ObjectsTxt;

struct D2ObjectDataStrc {
  D2ObjectsTxt* pTxt;  // 0x00
  union {
    uint8_t Type;  // 0x04 (0x0F would be a Exp Shrine)
    struct {
      uint8_t _1 : 7;
      uint8_t ChestLocked : 1;
    } UFlags;
  } UData;
  uint32_t _2[8];      // 0x08
  char szOwner[0x10];  // 0x28
};