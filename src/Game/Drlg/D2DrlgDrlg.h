#pragma once

#include <Windows.h>

#include "D2DrlgPreset.h"

struct Act;
struct Level;
struct Room1;
struct RoomTile;
struct UnitAny;

// rename D2DrlgCoordsStrc
#pragma pack(push, 1)
struct CollMap {
  DWORD dwPosGameX;   // 0x00
  DWORD dwPosGameY;   // 0x04
  DWORD dwSizeGameX;  // 0x08
  DWORD dwSizeGameY;  // 0x0C
  DWORD dwPosRoomX;   // 0x10
  DWORD dwPosRoomY;   // 0x14
  DWORD dwSizeRoomX;  // 0x18
  DWORD dwSizeRoomY;  // 0x1C
  WORD* pMapStart;    // 0x20
  WORD* pMapEnd;      // 0x22
};
#pragma pack(pop)

// rename D2DrlgRoomStrc
#pragma pack(push, 1)
struct Room2 {
  DWORD _1[2];         // 0x00
  Room2** pRoom2Near;  // 0x08
  DWORD _2[5];         // 0x0C
  struct {
    DWORD dwRoomNumber;   // 0x00
    DWORD _1;             // 0x04
    DWORD* pdwSubNumber;  // 0x08
  }* pType2Info;          // 0x20
  Room2* pRoom2Next;      // 0x24
  DWORD dwRoomFlags;      // 0x28
  DWORD dwRoomsNear;      // 0x2C
  Room1* pRoom1;          // 0x30
  DWORD dwPosX;           // 0x34
  DWORD dwPosY;           // 0x38
  DWORD dwSizeX;          // 0x3C
  DWORD dwSizeY;          // 0x40
  DWORD _3;               // 0x44
  DWORD dwPresetType;     // 0x48
  RoomTile* pRoomTiles;   // 0x4C
  DWORD _4[2];            // 0x50
  Level* pLevel;          // 0x58
  PresetUnit* pPreset;    // 0x5C
};
#pragma pack(pop)

// rename D2ActiveRoomStrc
struct Room1 {
  Room1** pRoomsNear;   // 0x00
  DWORD _1[3];          // 0x04
  Room2* pRoom2;        // 0x10
  DWORD _2[3];          // 0x14
  CollMap* Coll;        // 0x20
  DWORD dwRoomsNear;    // 0x24
  DWORD _3[9];          // 0x28
  DWORD dwXStart;       // 0x4C
  DWORD dwYStart;       // 0x50
  DWORD dwXSize;        // 0x54
  DWORD dwYSize;        // 0x58
  DWORD _4[6];          // 0x5C
  UnitAny* pUnitFirst;  // 0x74
  DWORD _5;             // 0x78
  Room1* pRoomNext;     // 0x7C
};

// rename D2RoomTileStrc
#pragma pack(push, 1)
struct RoomTile {
  Room2* pRoom2;    // 0x00
  RoomTile* pNext;  // 0x04
  DWORD _2[2];      // 0x08
  DWORD* nNum;      // 0x10
};
#pragma pack(pop)

// rename D2DrlgStrc
struct ActMisc {
  DWORD _1[37];            // 0x00
  DWORD dwStaffTombLevel;  // 0x94
  DWORD _2[245];           // 0x98
  Act* pAct;               // 0x46C
  DWORD _3[3];             // 0x470
  Level* pLevelFirst;      // 0x47C
};

// rename D2DrlgActStrc
struct Act {
  DWORD _1[3];      // 0x00
  DWORD dwMapSeed;  // 0x0C
  Room1* pRoom1;    // 0x10
  DWORD dwAct;      // 0x14
  DWORD _2[12];     // 0x18
  ActMisc* pMisc;   // 0x48
};

// rename D2DrlgLevelStrc
#pragma pack(push, 1)
struct Level {
  DWORD _1[4];         // 0x00
  Room2* pRoom2First;  // 0x10
  DWORD _2[2];         // 0x14
  DWORD dwPosX;        // 0x1C
  DWORD dwPosY;        // 0x20
  DWORD dwSizeX;       // 0x24
  DWORD dwSizeY;       // 0x28
  DWORD _3[96];        // 0x2C
  Level* pNextLevel;   // 0x1AC
  DWORD _4;            // 0x1B0
  ActMisc* pMisc;      // 0x1B4
  DWORD _5[6];         // 0x1BC
  DWORD dwLevelNo;     // 0x1D0
  DWORD _6[3];         // 0x1D4
  union {
    DWORD RoomCenterX[9];
    DWORD WarpX[9];
  };  // 0x1E0
  union {
    DWORD RoomCenterY[9];
    DWORD WarpY[9];
  };                    // 0x204
  DWORD dwRoomEntries;  // 0x228
};
#pragma pack(pop)