#pragma once

#include "DataTbls/LevelsTbls.h"

#include <Windows.h> // POINT

struct D2ActiveRoomStrc;

// rename D2AutomapCellStrc
struct D2AutomapCellStrc {
  uint32_t fSaved;           // 0x00
  uint16_t nCellNo;          // 0x04
  uint16_t xPixel;           // 0x06
  uint16_t yPixel;           // 0x08
  uint16_t wWeight;          // 0x0A
  D2AutomapCellStrc* pLess;  // 0x0C
  D2AutomapCellStrc* pMore;  // 0x10
};

// rename D2AutomapLayerStrc
struct D2AutomapLayerStrc {
  uint32_t nLayerNo;               // 0x00
  uint32_t fSaved;                 // 0x04
  D2AutomapCellStrc* pFloors;      // 0x08
  D2AutomapCellStrc* pWalls;       // 0x0C
  D2AutomapCellStrc* pObjects;     // 0x10
  D2AutomapCellStrc* pExtras;      // 0x14
  D2AutomapLayerStrc* pNextLayer;  // 0x18
};

inline D2AutomapLayerStrc** D2CLIENT_AutomapLayer = nullptr;
inline uint32_t* D2CLIENT_AutomapOn = nullptr;
inline int* D2CLIENT_AutomapMode = nullptr;
inline POINT* D2CLIENT_Offset = nullptr;
inline int* D2CLIENT_Divisor = nullptr;

inline uint32_t(__stdcall* D2CLIENT_GetAutomapSize)(void) = nullptr;
inline D2AutomapCellStrc*(__fastcall* D2CLIENT_NewAutomapCell)() = nullptr;
inline void(__fastcall* D2CLIENT_AddAutomapCell)(D2AutomapCellStrc* aCell, D2AutomapCellStrc** node) = nullptr;
inline void(__stdcall* D2CLIENT_RevealAutomapRoom)(D2ActiveRoomStrc* pRoom1, uint32_t dwClipFlag, D2AutomapLayerStrc* aLayer) = nullptr;
inline D2AutomapLayerStrc*(__fastcall* D2CLIENT_InitAutomapLayer_I)(uint32_t nLayerNo) = nullptr;
inline D2LevelDefBin*(__fastcall* D2COMMON_GetLayer)(uint32_t dwLevelNo) = nullptr;