#pragma once

#include <Windows.h>

// rename D2ItemsTxt
struct ItemTxt {
  wchar_t szName2[0x40];  // 0x00
  union {
    DWORD dwCode;
    char szCode[4];
  };                  // 0x40
  BYTE _2[0x70];      // 0x84
  WORD nLocaleTxtNo;  // 0xF4
  BYTE _2a[0x19];     // 0xF7
  BYTE xSize;         // 0xFC
  BYTE ySize;         // 0xFD
  BYTE _2b[13];       // 0xFE
  BYTE nType;         // 0x11E
  BYTE _3[0x0d];      // 0x11F
  BYTE fQuest;        // 0x12A
};