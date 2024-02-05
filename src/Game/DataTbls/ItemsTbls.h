#pragma once

#include <cinttypes>

struct D2ItemsTxt {
  wchar_t szName2[0x40];  // 0x00
  union {
    uint32_t dwCode;
    char szCode[4];
  };                      // 0x40
  uint8_t _2[0x70];       // 0x84
  uint16_t nLocaleTxtNo;  // 0xF4
  uint8_t _2a[0x19];      // 0xF7
  uint8_t xSize;          // 0xFC
  uint8_t ySize;          // 0xFD
  uint8_t _2b[13];        // 0xFE
  uint8_t nType;          // 0x11E
  uint8_t _3[0x0d];       // 0x11F
  uint8_t fQuest;         // 0x12A
};