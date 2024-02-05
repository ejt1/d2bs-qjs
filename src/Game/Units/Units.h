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

struct OverheadMsg;

struct D2UnitStrc {
  DWORD dwType;       // 0x00
  DWORD dwTxtFileNo;  // 0x04
  DWORD _1;           // 0x08
  DWORD dwUnitId;     // 0x0C
  DWORD dwMode;       // 0x10
  union {
    D2PlayerDataStrc* pPlayerData;
    D2ItemDataStrc* pItemData;
    D2MonsterDataStrc* pMonsterData;
    D2ObjectDataStrc* pObjectData;
    // TileData *pTileData doesn't appear to exist anymore
  };                // 0x14
  DWORD dwAct;      // 0x18
  D2DrlgActStrc* pAct;        // 0x1C
  DWORD dwSeed[2];  // 0x20
  DWORD _2;         // 0x28
  union {
    D2DynamicPathStrc* pPath;
    D2StaticPathStrc* pItemPath;
  };                            // 0x2C
  DWORD _3[5];                  // 0x30
  DWORD dwGfxFrame;             // 0x44
  DWORD dwFrameRemain;          // 0x48
  WORD wFrameRate;              // 0x4C
  WORD _4;                      // 0x4E
  BYTE* pGfxUnk;                // 0x50
  DWORD* pGfxInfo;              // 0x54
  DWORD _5;                     // 0x58
  D2StatListExStrc* pStats;     // 0x5C
  D2InventoryStrc* pInventory;  // 0x60
  D2GfxLightStrc* ptLight;      // 0x64
  DWORD dwStartLightRadius;     // 0x68
  WORD nPl2ShiftIdx;            // 0x6C
  WORD nUpdateType;             // 0x6E
  D2UnitStrc* pUpdateUnit;      // 0x70 - Used when updating unit.
  DWORD* pQuestRecord;          // 0x74
  DWORD bSparklyChest;          // 0x78 bool
  DWORD* pTimerArgs;            // 0x7C
  DWORD dwSoundSync;            // 0x80
  DWORD _6[2];                  // 0x84
  WORD wX;                      // 0x8C
  WORD wY;                      // 0x8E
  DWORD _7;                     // 0x90
  DWORD dwOwnerType;            // 0x94
  DWORD dwOwnerId;              // 0x98
  DWORD _8[2];                  // 0x9C
  OverheadMsg* pOMsg;           // 0xA4
  D2SkillListStrc* pInfo;       // 0xA8
  DWORD _9[6];                  // 0xAC
  DWORD dwFlags;                // 0xC4
  DWORD dwFlags2;               // 0xC8
  DWORD _10[5];                 // 0xCC
  D2UnitStrc* pChangedNext;     // 0xE0
  D2UnitStrc* pListNext;        // 0xE4 -> 0xD8
  D2UnitStrc* pRoomNext;        // 0xE8
};

struct UnitHashTable {
  D2UnitStrc* table[128];
};