#pragma once

#include <cstdint>

struct D2MonStatsTxt {
  uint8_t _1[0x6];        // 0x00
  uint16_t nLocaleTxtNo;  // 0x06
  uint16_t flag;          // 0x08
  uint16_t _1a;           // 0x0A
  union {
    uint16_t flag1;  // 0x0C
    struct {
      uint8_t flag1a;     // 0x0C
      uint8_t flag1b;     // 0x0D
      uint8_t flag1c[2];  // 0x0E
    } SFlags;
  } UFlags;
  uint8_t _2[0x22];            // 0x10
  uint16_t velocity;           // 0x32
  uint8_t _2a[0x52];           // 0x34
  uint16_t tcs[3][4];          // 0x86
  uint8_t _2b[0x52];           // 0x9E
  wchar_t szDescriptor[0x3c];  // 0xF0
  uint8_t _3[0x1a0];           // 0x12C
};