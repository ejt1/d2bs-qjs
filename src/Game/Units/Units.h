#pragma once

#include "Game/D2DataTbls.h"
#include "Game/Units/Item.h"
#include "Game/Units/Monster.h"
#include "Game/Units/Object.h"
#include "Game/Units/Player.h"
#include "Game/D2StatList.h"
#include "Game/Path/Path.h"
#include "Game/D2Skills.h"
#include "Game/Drlg/D2DrlgDrlg.h"
#include "Game/D2Chat.h"
#include "Game/D2Gfx.h"
#include "Game/D2Inventory.h"

#include <cstdint>

///////////////////////////////////////////////////
// Unit Types
///////////////////////////////////////////////////
#define UNIT_PLAYER 0
#define UNIT_MONSTER 1
#define UNIT_OBJECT 2
#define UNIT_MISSILE 3
#define UNIT_ITEM 4
#define UNIT_TILE 5

struct UnitInteraction {
  uint32_t dwMoveType;       // 0x00
  D2UnitStrc* lpPlayerUnit;  // 0x04
  D2UnitStrc* lpTargetUnit;  // 0x08
  uint32_t dwTargetX;        // 0x0C
  uint32_t dwTargetY;        // 0x10
  uint32_t _1;               // 0x14
  uint32_t _2;               // 0x18
  D2SkillStrc* pSkill;
};

struct D2UnitStrc {
  // TODO(ejt): these should return std::string instead of requiring an allocated char array
  const char* GetUnitName(char* szTmp, size_t bufSize);
  void GetItemCode(char* szBuf);

  static D2UnitStrc* FindUnit(uint32_t id, uint32_t type);
  static D2UnitStrc* FindUnit(const char* name, uint32_t classId, uint32_t type, uint32_t mode, uint32_t unitId);
  D2UnitStrc* GetNext(const char* name, uint32_t classId, uint32_t type, uint32_t mode);
  D2UnitStrc* FindItem(const char* name, uint32_t classId, uint32_t mode, uint32_t unitId);
  D2UnitStrc* GetNextItem(D2UnitStrc* pOwner, const char* name, uint32_t classId, uint32_t mode);
  bool ValidateUnit(const char* name, uint32_t classId, uint32_t type, uint32_t mode, uint32_t unitId);
  D2UnitStrc* FindMercUnit();

  D2SkillStrc* GetStartSkill();
  D2SkillStrc* GetLeftSkill();
  D2SkillStrc* GetRightSkill();
  D2SkillStrc* GetUsedSkill();
  D2SkillStrc* GetSkillFromSkillId(int skillId);
  D2SkillStrc* GetSkillFromSkillName(const char* name);

  int GetHealth();
  int GetMana();

  uint32_t GetX();
  uint32_t GetY();

  void Interact(uint32_t dwMoveType);

  int GetItemLocation();

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
  uint8_t* pGfxUnk;             // 0x50
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
  D2HoverTextStrc* pOMsg;       // 0xA4
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

inline D2UnitStrc*(__fastcall* D2CLIENT_FindClientSideUnit)(uint32_t dwId, uint32_t dwType) = nullptr;
inline D2UnitStrc*(__fastcall* D2CLIENT_FindServerSideUnit)(uint32_t dwId, uint32_t dwType) = nullptr;
inline D2UnitStrc*(__stdcall* D2CLIENT_GetPlayerUnit)() = nullptr;
inline D2UnitStrc*(__fastcall* D2CLIENT_GetCurrentInteractingNPC)() = nullptr;
inline D2UnitStrc*(__stdcall* D2CLIENT_GetSelectedUnit)() = nullptr;
inline D2UnitStrc*(__fastcall* D2CLIENT_GetCursorItem)() = nullptr;
// inline D2UnitStrc* (__fastcall *D2CLIENT_GetMercUnit)(void) = nullptr; //Updated 1.14d //00478A90-BASE 478F20 with 7 0 args
inline void(__fastcall* D2CLIENT_SetSelectedUnit_I)(D2UnitStrc* pUnit) = nullptr;
inline D2UnitStrc*(__stdcall* D2COMMON_GetItemFromInventory)(D2InventoryStrc* inv) = nullptr;
inline D2UnitStrc*(__stdcall* D2COMMON_GetNextItemFromInventory)(D2UnitStrc* pItem) = nullptr;
inline int(__fastcall* D2CLIENT_GetItemName)(D2UnitStrc* pItem, wchar_t* wBuffer, uint32_t dwSize) = nullptr;
inline D2ItemsTxt*(__stdcall* D2COMMON_GetItemText)(uint32_t itemno) = nullptr;
inline int(__stdcall* D2CLIENT_LoadItemDesc)(D2UnitStrc* pItem, int type) = nullptr;
inline uint32_t(__fastcall* D2CLIENT_GetMonsterOwner)(uint32_t nMonsterId) = nullptr;
inline uint32_t(__fastcall* D2CLIENT_GetUnitHPPercent)(uint32_t dwUnitId) = nullptr;
inline uint32_t(__stdcall* D2COMMON_GetUnitStat)(D2UnitStrc* pUnit, uint32_t dwStat, uint32_t dwStat2) = nullptr;
inline void(__fastcall* D2CLIENT_InitInventory)(void) = nullptr;
inline uint32_t(__fastcall* D2CLIENT_SetUIVar)(uint32_t varno, uint32_t howset, uint32_t unknown1) = nullptr;
inline int(__fastcall* D2CLIENT_GetUnitX)(D2UnitStrc* pUnit) = nullptr;
inline int(__fastcall* D2CLIENT_GetUnitY)(D2UnitStrc* pUnit) = nullptr;
inline void(__fastcall* D2CLIENT_ShopAction)(D2UnitStrc* pNpc, D2UnitStrc* pItem, uint32_t dwSell, uint32_t unk, uint32_t dwItemCost, uint32_t dwMode, uint32_t _2,
                                             uint32_t _3) = nullptr;
inline void(__stdcall* D2CLIENT_LeftClickItem_I)(D2UnitStrc* pPlayer, D2InventoryStrc* pInventory, int x, int y, uint32_t dwClickType, InventoryLayout* pLayout,
                                                 uint32_t Location) = nullptr;

inline UnitHashTable* D2CLIENT_ServerSideUnitHashTables = nullptr;
inline UnitHashTable* D2CLIENT_ClientSideUnitHashTables = nullptr;
inline D2UnitStrc** D2CLIENT_PlayerUnit = nullptr;
inline D2UnitStrc** D2CLIENT_SelectedInvItem = nullptr;

inline uint32_t D2CLIENT_GetUnitName_I = NULL;
uint32_t __fastcall D2CLIENT_GetUnitName_STUB(D2UnitStrc* UnitAny);
#define D2CLIENT_GetUnitName(x) (wchar_t*)D2CLIENT_GetUnitName_STUB(x)

inline uint32_t D2CLIENT_Interact_I = NULL;
void __fastcall D2CLIENT_Interact_ASM(UnitInteraction* Struct);
#define D2CLIENT_Interact_STUB(x) (D2CLIENT_Interact_ASM((UnitInteraction*)x))
