#ifndef D2HELPERS_H
#define D2HELPERS_H

#include "js32.h"

#include "Game/D2Automap.h"
#include "Game/D2Inventory.h"
#include "Game/D2Gfx.h"
#include "Game/Drlg/D2DrlgDrlg.h"
#include "Game/Units/Units.h"
#include "Game/D2DataTbls.h"
#include "Game/D2Roster.h"
#include "Game/Unorganized.h"

#include <Windows.h> // POINT

void Log(const char* szFormat, ...);
void LogNoFormat(const char* szString);

enum ClientGameState { ClientStateNull, ClientStateMenu, ClientStateInGame, ClientStateBusy };
ClientGameState ClientState(void);
bool GameReady(void);
bool WaitForGameReady(void);
DWORD GetPlayerArea(void);

void SendMouseClick(int x, int y, int clicktype);
void SendKeyPress(uint32_t type, uint32_t key, uint32_t ext);

BYTE CalcPercent(DWORD dwVal, DWORD dwMaxVal, BYTE iMin = NULL);

DWORD __fastcall D2CLIENT_GetUIVar_STUB(DWORD varno);
void __fastcall D2CLIENT_SetSelectedUnit_STUB(DWORD UnitAny);
DWORD __fastcall D2CLIENT_ClickParty_ASM(DWORD RosterUnit, DWORD Mode);
void __fastcall D2CLIENT_ClickShopItem_ASM(DWORD x, DWORD y, DWORD BuyOrSell);
void __fastcall D2CLIENT_ShopAction_ASM(DWORD pTable, DWORD pItem, DWORD pNPC, DWORD _1, DWORD pTable2 /* Could be also the ItemCost?*/, DWORD dwMode, DWORD _2,
                                        DWORD _3);
void __fastcall D2CLIENT_ClickItemRight_ASM(DWORD x, DWORD y, DWORD Location, DWORD pItem, DWORD pItemPath);
void __fastcall D2CLIENT_ClickBelt(DWORD x, DWORD y, D2InventoryStrc* pInventoryData);
void __fastcall D2CLIENT_ClickBeltRight_ASM(DWORD pInventory, DWORD pPlayer, DWORD HoldShift, DWORD dwPotPos);
void __fastcall D2CLIENT_GetItemDesc_ASM(DWORD pUnit, wchar_t* pBuffer);
void __fastcall D2COMMON_DisplayOverheadMsg_ASM(DWORD pUnit);
void __fastcall D2CLIENT_MercItemAction_ASM(DWORD bPacketType, DWORD dwSlot);
void __fastcall D2CLIENT_PlaySound(DWORD dwSoundId);
void __fastcall D2GFX_DrawRectFrame_STUB(RECT* rect);
DWORD __cdecl D2CLIENT_GetMinionCount(D2UnitStrc* pUnit, DWORD dwType);
void __fastcall D2CLIENT_HostilePartyUnit(D2RosterUnitStrc* pUnit, DWORD dwButton);
void __stdcall D2CLIENT_TakeWaypoint(DWORD dwWaypointId, DWORD dwArea);
DWORD __fastcall D2CLIENT_SendGamePacket_ASM(DWORD dwLen, BYTE* bPacket);
void __stdcall D2CLIENT_LeftClickItem(DWORD Location, D2UnitStrc* pPlayer, D2InventoryStrc* pInventory, int x, int y, DWORD dwClickType, InventoryLayout* pLayout);

enum DistanceType { Euclidean, Chebyshev, Manhattan };
double GetDistance(long x1, long y1, long x2, long y2, DistanceType type = Euclidean);
bool IsScrollingText();

// TODO(ejt): used in JSUnit.cpp to read a string, rewrite so it's an offset loaded at start instead
void ReadProcessBYTES(HANDLE hProcess, DWORD lpAddress, void* buf, int len);

#endif
