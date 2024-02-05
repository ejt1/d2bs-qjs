#pragma once

#include <cstdint>

#pragma pack(push, 1)
struct D2RosterUnitStrc {
  char szName[16];          // 0x00
  uint32_t dwUnitId;        // 0x10
  uint32_t dwPartyLife;     // 0x14
  uint32_t _1;              // 0x18
  uint32_t dwClassId;       // 0x1C
  uint16_t wLevel;          // 0x20
  uint16_t wPartyId;        // 0x22
  uint32_t dwLevelId;       // 0x24
  uint32_t Xpos;            // 0x28
  uint32_t Ypos;            // 0x2C
  uint32_t dwPartyFlags;    // 0x30
  BYTE* _5;                 // 0x34
  uint32_t _6[11];          // 0x38
  uint16_t _7;              // 0x64
  char szName2[16];         // 0x66
  uint16_t _8;              // 0x76
  uint32_t _9[2];           // 0x78
  D2RosterUnitStrc* pNext;  // 0x80
};
#pragma pack(pop)