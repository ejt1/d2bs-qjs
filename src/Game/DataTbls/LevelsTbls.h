#pragma once

#include <Windows.h>

// rename D2LevelsTxt
struct LevelTxt {
  DWORD dwLevelNo;            // 0x00
  DWORD _1[60];               // 0x04
  BYTE _2;                    // 0xF4
  char szName[40];            // 0xF5
  char szEntranceText[40];    // 0x11D
  char szLevelDesc[41];       // 0x145
  wchar_t wName[40];          // 0x16E
  wchar_t wEntranceText[40];  // 0x1BE
  BYTE nObjGroup[8];          // 0x196
  BYTE nObjPrb[8];            // 0x19E
};