#pragma once

// rename D2PresetUnitStrc
#pragma pack(push, 1)
struct PresetUnit {
  DWORD _1;                 // 0x00
  DWORD dwTxtFileNo;        // 0x04
  DWORD dwPosX;             // 0x08
  PresetUnit* pPresetNext;  // 0x0C
  DWORD _3;                 // 0x10
  DWORD dwType;             // 0x14
  DWORD dwPosY;             // 0x18
};
#pragma pack(pop)