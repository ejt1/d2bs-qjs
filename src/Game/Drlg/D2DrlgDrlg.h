#pragma once

#include "D2DrlgPreset.h"

#include <cstdint>

struct D2DrlgActStrc;
struct D2DrlgLevelStrc;
struct D2ActiveRoomStrc;
struct D2RoomTileStrc;
struct D2UnitStrc;

#pragma pack(push, 1)
struct D2DrlgCoordsStrc {
  uint32_t dwPosGameX;   // 0x00
  uint32_t dwPosGameY;   // 0x04
  uint32_t dwSizeGameX;  // 0x08
  uint32_t dwSizeGameY;  // 0x0C
  uint32_t dwPosRoomX;   // 0x10
  uint32_t dwPosRoomY;   // 0x14
  uint32_t dwSizeRoomX;  // 0x18
  uint32_t dwSizeRoomY;  // 0x1C
  uint16_t* pMapStart;   // 0x20
  uint16_t* pMapEnd;     // 0x22
};
#pragma pack(pop)

#pragma pack(push, 1)
struct D2DrlgRoomStrc {
  uint32_t _1[2];               // 0x00
  D2DrlgRoomStrc** pRoom2Near;  // 0x08
  uint32_t _2[5];               // 0x0C
  struct {
    uint32_t dwRoomNumber;     // 0x00
    uint32_t _1;               // 0x04
    uint32_t* pdwSubNumber;    // 0x08
  }* pType2Info;               // 0x20
  D2DrlgRoomStrc* pRoom2Next;  // 0x24
  uint32_t dwRoomFlags;        // 0x28
  uint32_t dwRoomsNear;        // 0x2C
  D2ActiveRoomStrc* pRoom1;    // 0x30
  uint32_t dwPosX;             // 0x34
  uint32_t dwPosY;             // 0x38
  uint32_t dwSizeX;            // 0x3C
  uint32_t dwSizeY;            // 0x40
  uint32_t _3;                 // 0x44
  uint32_t dwPresetType;       // 0x48
  D2RoomTileStrc* pRoomTiles;  // 0x4C
  uint32_t _4[2];              // 0x50
  D2DrlgLevelStrc* pLevel;     // 0x58
  D2PresetUnitStrc* pPreset;   // 0x5C
};
#pragma pack(pop)

struct D2ActiveRoomStrc {
  D2ActiveRoomStrc** pRoomsNear;  // 0x00
  uint32_t _1[3];                 // 0x04
  D2DrlgRoomStrc* pRoom2;         // 0x10
  uint32_t _2[3];                 // 0x14
  D2DrlgCoordsStrc* Coll;         // 0x20
  uint32_t dwRoomsNear;           // 0x24
  uint32_t _3[9];                 // 0x28
  uint32_t dwXStart;              // 0x4C
  uint32_t dwYStart;              // 0x50
  uint32_t dwXSize;               // 0x54
  uint32_t dwYSize;               // 0x58
  uint32_t _4[6];                 // 0x5C
  D2UnitStrc* pUnitFirst;         // 0x74
  uint32_t _5;                    // 0x78
  D2ActiveRoomStrc* pRoomNext;    // 0x7C
};

#pragma pack(push, 1)
struct D2RoomTileStrc {
  D2DrlgRoomStrc* pRoom2;  // 0x00
  D2RoomTileStrc* pNext;   // 0x04
  uint32_t _2[2];          // 0x08
  uint32_t* nNum;          // 0x10
};
#pragma pack(pop)

struct D2DrlgStrc {
  uint32_t _1[37];               // 0x00
  uint32_t dwStaffTombLevel;     // 0x94
  uint32_t _2[245];              // 0x98
  D2DrlgActStrc* pAct;           // 0x46C
  uint32_t _3[3];                // 0x470
  D2DrlgLevelStrc* pLevelFirst;  // 0x47C
};

struct D2DrlgActStrc {
  uint32_t _1[3];            // 0x00
  uint32_t dwMapSeed;        // 0x0C
  D2ActiveRoomStrc* pRoom1;  // 0x10
  uint32_t dwAct;            // 0x14
  uint32_t _2[12];           // 0x18
  D2DrlgStrc* pMisc;         // 0x48
};

#pragma pack(push, 1)
struct D2DrlgLevelStrc {
  uint32_t _1[4];               // 0x00
  D2DrlgRoomStrc* pRoom2First;  // 0x10
  uint32_t _2[2];               // 0x14
  uint32_t dwPosX;              // 0x1C
  uint32_t dwPosY;              // 0x20
  uint32_t dwSizeX;             // 0x24
  uint32_t dwSizeY;             // 0x28
  uint32_t _3[96];              // 0x2C
  D2DrlgLevelStrc* pNextLevel;  // 0x1AC
  uint32_t _4;                  // 0x1B0
  D2DrlgStrc* pMisc;            // 0x1B4
  uint32_t _5[6];               // 0x1BC
  uint32_t dwLevelNo;           // 0x1D0
  uint32_t _6[3];               // 0x1D4
  union {
    uint32_t RoomCenterX[9];
    uint32_t WarpX[9];
  };  // 0x1E0
  union {
    uint32_t RoomCenterY[9];
    uint32_t WarpY[9];
  };                       // 0x204
  uint32_t dwRoomEntries;  // 0x228
};
#pragma pack(pop)