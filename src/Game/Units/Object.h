#pragma once

struct D2ObjectsTxt;

struct D2ObjectDataStrc {
  D2ObjectsTxt* pTxt;  // 0x00
  union {
    BYTE Type;  // 0x04 (0x0F would be a Exp Shrine)
    struct {
      BYTE _1 : 7;
      BYTE ChestLocked : 1;
    } UFlags;
  } UData;
  DWORD _2[8];         // 0x08
  char szOwner[0x10];  // 0x28
};