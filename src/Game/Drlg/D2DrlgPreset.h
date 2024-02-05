#pragma once

#include <cstdint>

#pragma pack(push, 1)
struct D2PresetUnitStrc {
  uint32_t _1;              // 0x00
  uint32_t dwTxtFileNo;           // 0x04
  uint32_t dwPosX;          // 0x08
  D2PresetUnitStrc* pPresetNext;  // 0x0C
  uint32_t _3;                    // 0x10
  uint32_t dwType;                // 0x14
  uint32_t dwPosY;                // 0x18
};
#pragma pack(pop)