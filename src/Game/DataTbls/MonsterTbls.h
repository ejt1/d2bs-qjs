#pragma once

#include <Windows.h>

struct D2MonStatsTxt {
  BYTE _1[0x6];       // 0x00
  WORD nLocaleTxtNo;  // 0x06
  WORD flag;          // 0x08
  WORD _1a;           // 0x0A
  union {
    DWORD flag1;  // 0x0C
    struct {
      BYTE flag1a;     // 0x0C
      BYTE flag1b;     // 0x0D
      BYTE flag1c[2];  // 0x0E
    } SFlags;
  } UFlags;
  BYTE _2[0x22];               // 0x10
  WORD velocity;               // 0x32
  BYTE _2a[0x52];              // 0x34
  WORD tcs[3][4];              // 0x86
  BYTE _2b[0x52];              // 0x9E
  wchar_t szDescriptor[0x3c];  // 0xF0
  BYTE _3[0x1a0];              // 0x12C
};