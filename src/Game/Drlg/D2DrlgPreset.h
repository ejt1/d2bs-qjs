#pragma once

#pragma pack(push, 1)
struct D2PresetUnitStrc {
  DWORD _1;                 // 0x00
  DWORD dwTxtFileNo;        // 0x04
  DWORD dwPosX;             // 0x08
  D2PresetUnitStrc* pPresetNext;  // 0x0C
  DWORD _3;                 // 0x10
  DWORD dwType;             // 0x14
  DWORD dwPosY;             // 0x18
};
#pragma pack(pop)