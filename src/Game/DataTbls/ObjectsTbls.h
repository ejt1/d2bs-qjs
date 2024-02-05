#pragma once

#include <Windows.h>

// rename D2ObjectsTxt
struct ObjectTxt {
  char szName[0x40];      // 0x00
  wchar_t wszName[0x40];  // 0x40
  BYTE _1[4];             // 0xC0
  BYTE nSelectable0;      // 0xC4
  BYTE _2[0x87];          // 0xC5
  BYTE nOrientation;      // 0x14C
  BYTE _2b[0x19];         // 0x14D
  BYTE nSubClass;         // 0x166
  BYTE _3[0x11];          // 0x167
  BYTE nParm0;            // 0x178
  BYTE _4[0x39];          // 0x179
  BYTE nPopulateFn;       // 0x1B2
  BYTE nOperateFn;        // 0x1B3
  BYTE _5[8];             // 0x1B4
  DWORD nAutoMap;         // 0x1BB
};