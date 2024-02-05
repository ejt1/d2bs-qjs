#pragma once

#pragma pack(push, 1)
struct D2RosterUnitStrc {
  char szName[16];          // 0x00
  DWORD dwUnitId;           // 0x10
  DWORD dwPartyLife;        // 0x14
  DWORD _1;                 // 0x18
  DWORD dwClassId;          // 0x1C
  WORD wLevel;              // 0x20
  WORD wPartyId;            // 0x22
  DWORD dwLevelId;          // 0x24
  DWORD Xpos;               // 0x28
  DWORD Ypos;               // 0x2C
  DWORD dwPartyFlags;       // 0x30
  BYTE* _5;                 // 0x34
  DWORD _6[11];             // 0x38
  WORD _7;                  // 0x64
  char szName2[16];         // 0x66
  WORD _8;                  // 0x76
  DWORD _9[2];              // 0x78
  D2RosterUnitStrc* pNext;  // 0x80
};
#pragma pack(pop)