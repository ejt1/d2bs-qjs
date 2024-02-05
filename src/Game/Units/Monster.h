#pragma once

// rename D2MonsterDataStrc
struct MonsterData {
  BYTE _1[22];  // 0x00
  struct {
    BYTE fUnk : 1;
    BYTE fNormal : 1;
    BYTE fChamp : 1;
    BYTE fBoss : 1;
    BYTE fMinion : 1;
  } UFlags;  // 0x16
  BYTE _2[5];
  BYTE anEnchants[9];  // 0x1C
  WORD wUniqueNo;      // 0x26
  DWORD _5;            // 0x28
  struct {
    wchar_t wName[28];
  } UName;  // 0x2C
};