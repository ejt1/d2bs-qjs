#ifndef D2HELPERS_H
#define D2HELPERS_H

#include "D2Ptrs.h"
#include "js32.h"

#include "Game/D2Inventory.h"
#include "Game/D2Gfx.h"
#include "Game/Drlg/D2DrlgDrlg.h"

void Log(const wchar_t* szFormat, ...);
void LogNoFormat(const wchar_t* szString);

enum ClientGameState { ClientStateNull, ClientStateMenu, ClientStateInGame, ClientStateBusy };
ClientGameState ClientState(void);
bool GameReady(void);
bool WaitForGameReady(void);

DWORD GetPlayerArea(void);
void SendMouseClick(int x, int y, int clicktype);
void SendKeyPress(uint32_t type, uint32_t key, uint32_t ext);
POINT CalculateTextLen(const char* szwText, int Font);
POINT CalculateTextLen(const wchar_t* szwText, int Font);
void WorldToScreen(POINT* pPos);
void ScreenToWorld(POINT* ptPos);
POINT ScreenToAutomap(int x, int y);
void AutomapToScreen(POINT* pPos);
Level* GetLevel(DWORD dwLevelNo);
void D2CLIENT_Interact(UnitAny* pUnit, DWORD dwMoveType);
void myDrawText(const wchar_t* szwText, int x, int y, int color, int font);
void myDrawCenterText(const wchar_t* szwText, int x, int y, int color, int font, int div);
// UnitAny* FindItemByPosition(DWORD x, DWORD y, DWORD Location);
BYTE CalcPercent(DWORD dwVal, DWORD dwMaxVal, BYTE iMin = NULL);
DWORD GetTileLevelNo(Room2* lpRoom2, DWORD dwTileNo);
UnitAny* GetMercUnit(UnitAny* pUnit);

BOOL ClickNPCMenu(DWORD NPCClassId, DWORD MenuId);
int GetItemLocation(UnitAny* pItem);
// void	SelectInventoryItem(DWORD x, DWORD y, DWORD dwLocation);

int GetSkill(WORD wSkillId);
BOOL SetSkill(JSContext* cx, WORD wSkillId, BOOL bLeft, DWORD dwItemId = 0xFFFFFFFF);
char* GetSkillByID(WORD id);
WORD GetSkillByName(const char* szSkillName);

const char* GetUnitName(UnitAny* pUnit, char* szBuf, size_t bufSize);
void GetItemCode(UnitAny* pUnit, char* szBuf);

UnitAny* D2CLIENT_FindUnit(DWORD dwId, DWORD dwType);

POINT GetScreenSize();
int D2GetScreenSizeX();
int D2GetScreenSizeY();

AutomapLayer* InitAutomapLayer(DWORD levelno);
DWORD __fastcall D2CLIENT_InitAutomapLayer_STUB(DWORD nLayerNo);
void myDrawAutomapCell(CellFile* cellfile, int xpos, int ypos, BYTE col);

CellFile* LoadCellFile(const char* lpszPath, DWORD bMPQ = TRUE);
CellFile* LoadCellFile(const wchar_t* lpszPath, DWORD bMPQ = 3);
CellFile* LoadBmpCellFile(BYTE* buf1, int width, int height);
CellFile* LoadBmpCellFile(const char* filename);
CellFile* LoadBmpCellFile(const wchar_t* filename);
CellFile* myInitCellFile(CellFile* cf);

static inline UnitAny* GetPlayerUnit() {
  return D2CLIENT_GetPlayerUnit();
}
static inline void AddRoomData(Room2* room) {
  D2COMMON_AddRoomData(room->pLevel->pMisc->pAct, room->pLevel->dwLevelNo, room->dwPosX, room->dwPosY, room->pRoom1);
}
static inline void RemoveRoomData(Room2* room) {
  D2COMMON_RemoveRoomData(room->pLevel->pMisc->pAct, room->pLevel->dwLevelNo, room->dwPosX, room->dwPosY, room->pRoom1);
}
static inline char* __stdcall GetLevelName(const Level* level) {
  return D2COMMON_GetLevelText(level->dwLevelNo)->szName;
}
static inline char* __stdcall GetLevelIdName(DWORD level) {
  return D2COMMON_GetLevelText(level)->szName;
}

DWORD __fastcall D2CLIENT_GetUnitName_STUB(DWORD UnitAny);
DWORD __fastcall D2CLIENT_GetUIVar_STUB(DWORD varno);
void __fastcall D2CLIENT_SetSelectedUnit_STUB(DWORD UnitAny);
void __fastcall D2CLIENT_Interact_ASM(DWORD Struct);
DWORD __fastcall D2CLIENT_ClickParty_ASM(DWORD RosterUnit, DWORD Mode);
void __fastcall D2CLIENT_ClickShopItem_ASM(DWORD x, DWORD y, DWORD BuyOrSell);
void __fastcall D2CLIENT_ShopAction_ASM(DWORD pTable, DWORD pItem, DWORD pNPC, DWORD _1, DWORD pTable2 /* Could be also the ItemCost?*/, DWORD dwMode, DWORD _2,
                                        DWORD _3);
void __fastcall D2CLIENT_ClickItemRight_ASM(DWORD x, DWORD y, DWORD Location, DWORD pItem, DWORD pItemPath);
void __fastcall D2CLIENT_ClickBelt(DWORD x, DWORD y, Inventory* pInventoryData);
void __fastcall D2CLIENT_ClickBeltRight_ASM(DWORD pInventory, DWORD pPlayer, DWORD HoldShift, DWORD dwPotPos);
void __fastcall D2CLIENT_GetItemDesc_ASM(DWORD pUnit, wchar_t* pBuffer);
void __fastcall D2COMMON_DisplayOverheadMsg_ASM(DWORD pUnit);
void __fastcall D2CLIENT_MercItemAction_ASM(DWORD bPacketType, DWORD dwSlot);
void __fastcall D2CLIENT_PlaySound(DWORD dwSoundId);
void __fastcall D2GFX_DrawRectFrame_STUB(RECT* rect);
DWORD __cdecl D2CLIENT_GetMinionCount(UnitAny* pUnit, DWORD dwType);
void __fastcall D2CLIENT_HostilePartyUnit(RosterUnit* pUnit, DWORD dwButton);
void __stdcall D2CLIENT_TakeWaypoint(DWORD dwWaypointId, DWORD dwArea);
DWORD __fastcall D2CLIENT_SendGamePacket_ASM(DWORD dwLen, BYTE* bPacket);
void __stdcall D2CLIENT_LeftClickItem(DWORD Location, UnitAny* pPlayer, Inventory* pInventory, int x, int y, DWORD dwClickType, InventoryLayout* pLayout);

enum DistanceType { Euclidean, Chebyshev, Manhattan };
double GetDistance(long x1, long y1, long x2, long y2, DistanceType type = Euclidean);
bool IsScrollingText();

// TODO(ejt): used in JSUnit.cpp to read a string, rewrite so it's an offset loaded at start instead
void ReadProcessBYTES(HANDLE hProcess, DWORD lpAddress, void* buf, int len);

#endif
