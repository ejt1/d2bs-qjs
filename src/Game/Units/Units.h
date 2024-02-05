#pragma once

#include "Game/Units/Item.h"
#include "Game/Units/Monster.h"
#include "Game/Units/Object.h"
#include "Game/Units/Player.h"
#include "Game/D2StatList.h"
#include "Game/Path/Path.h"
#include "Game/D2Skills.h"
#include "Game/Drlg/D2DrlgDrlg.h"
#include "Game/D2Gfx.h"

#include <cstdint>

struct OverheadMsg;

struct D2UnitStrc {
  uint32_t dwType;       // 0x00
  uint32_t dwTxtFileNo;  // 0x04
  uint32_t _1;           // 0x08
  uint32_t dwUnitId;     // 0x0C
  uint32_t dwMode;       // 0x10
  union {
    D2PlayerDataStrc* pPlayerData;
    D2ItemDataStrc* pItemData;
    D2MonsterDataStrc* pMonsterData;
    D2ObjectDataStrc* pObjectData;
    // TileData *pTileData doesn't appear to exist anymore
  };                    // 0x14
  uint32_t dwAct;       // 0x18
  D2DrlgActStrc* pAct;  // 0x1C
  uint32_t dwSeed[2];   // 0x20
  uint32_t _2;          // 0x28
  union {
    D2DynamicPathStrc* pPath;
    D2StaticPathStrc* pItemPath;
  };                            // 0x2C
  uint32_t _3[5];               // 0x30
  uint32_t dwGfxFrame;          // 0x44
  uint32_t dwFrameRemain;       // 0x48
  uint16_t wFrameRate;          // 0x4C
  uint16_t _4;                  // 0x4E
  BYTE* pGfxUnk;                // 0x50
  uint32_t* pGfxInfo;           // 0x54
  uint32_t _5;                  // 0x58
  D2StatListExStrc* pStats;     // 0x5C
  D2InventoryStrc* pInventory;  // 0x60
  D2GfxLightStrc* ptLight;      // 0x64
  uint32_t dwStartLightRadius;  // 0x68
  uint16_t nPl2ShiftIdx;        // 0x6C
  uint16_t nUpdateType;         // 0x6E
  D2UnitStrc* pUpdateUnit;      // 0x70 - Used when updating unit.
  uint32_t* pQuestRecord;       // 0x74
  uint32_t bSparklyChest;       // 0x78 bool
  uint32_t* pTimerArgs;         // 0x7C
  uint32_t dwSoundSync;         // 0x80
  uint32_t _6[2];               // 0x84
  uint16_t wX;                  // 0x8C
  uint16_t wY;                  // 0x8E
  uint32_t _7;                  // 0x90
  uint32_t dwOwnerType;         // 0x94
  uint32_t dwOwnerId;           // 0x98
  uint32_t _8[2];               // 0x9C
  OverheadMsg* pOMsg;           // 0xA4
  D2SkillListStrc* pInfo;       // 0xA8
  uint32_t _9[6];               // 0xAC
  uint32_t dwFlags;             // 0xC4
  uint32_t dwFlags2;            // 0xC8
  uint32_t _10[5];              // 0xCC
  D2UnitStrc* pChangedNext;     // 0xE0
  D2UnitStrc* pListNext;        // 0xE4 -> 0xD8
  D2UnitStrc* pRoomNext;        // 0xE8
};

struct UnitHashTable {
  D2UnitStrc* table[128];
};