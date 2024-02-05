#pragma once

// file contains structs, functions and variables that needs organizing
// DO NOT ADD NEW THINGS HERE!
// DO NOT ADD NEW THINGS HERE!
// DO NOT ADD NEW THINGS HERE!
// DO NOT ADD NEW THINGS HERE!
// DO NOT ADD NEW THINGS HERE!
// DO NOT ADD NEW THINGS HERE!
// DO NOT ADD NEW THINGS HERE!
// DO NOT ADD NEW THINGS HERE!
// DO NOT ADD NEW THINGS HERE!
// DO NOT ADD NEW THINGS HERE!
// DO NOT ADD NEW THINGS HERE!
// DO NOT ADD NEW THINGS HERE!
// DO NOT ADD NEW THINGS HERE!
// DO NOT ADD NEW THINGS HERE!
// DO NOT ADD NEW THINGS HERE!
// DO NOT ADD NEW THINGS HERE!
// DO NOT ADD NEW THINGS HERE!
// DO NOT ADD NEW THINGS HERE!

#include "UnorganizedConstants.h"

#include <cstdint>

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

#pragma pack(push, 1)
struct BnetData {
  uint32_t dwId;                  // 0x00
  uint32_t dwId2;                 // 0x04
  uint8_t _1[0x10];               // 0x08
  uint32_t dwId3;                 // 0x18
  uint16_t Unk3;                  // 0x1C
  uint8_t _2;                     // 0x1E
  char szGameName[0x16];          // 0x1F
  uint16_t _3;                    // 0x35
  char szGameIP[0x10];            // 0x37
  uint8_t _5[0x42];               // 0x47
  uint32_t dwId4;                 // 0x89
  char szAccountName[0x30];       // 0x8D
  char szPlayerName[0x18];        // 0xBD
  char szRealmName[0x08];         // 0xD5
  uint8_t _8[0x111];              // 0xDD
  uint8_t nCharClass;             // 0x1EE
  uint8_t nCharFlags;             // 0x1EF
  uint8_t nMaxDiff;               // 0x1F0
  uint8_t _9[0x1F];               // 0x1F1
  uint8_t CreatedGameDifficulty;  // 0x210
  void* _10;                      // 0x211
  uint8_t _11[0x15];              // 0x215
  uint16_t _12;                   // 0x22A
  uint8_t _13;                    // 0x22C
  char szRealmName2[0x18];        // 0x22D
  char szGamePass[0x18];          // 0x245
  char szGameDesc[0x104];         // 0x25D
  char channelname[0x20];         //+0x35f
  uint8_t _14[0x40];              //+0x37f
  uint8_t charlevel;              //+0x3bf
  uint8_t ladderflag;             //+0x3c0
  uint32_t passhash;              //+0x3c1
  uint8_t passlength;             //+0x3c5
};
#pragma pack(pop)

inline void(__fastcall* D2CLIENT_SubmitItem)(uint32_t dwItemId) = nullptr;
inline void(__fastcall* D2CLIENT_Transmute)(void) = nullptr;
inline void(__fastcall* D2CLIENT_CloseNPCInteract)(void) = nullptr;
inline void(__fastcall* D2CLIENT_CloseInteract)(void) = nullptr;
inline void(__fastcall* D2CLIENT_ClickMap)(uint32_t MouseFlag, uint32_t x, uint32_t y, uint32_t Type) = nullptr;
inline uint32_t(__fastcall* D2CLIENT_GetMouseXOffset)(void) = nullptr;
inline uint32_t(__fastcall* D2CLIENT_GetMouseYOffset)(void) = nullptr;
inline void(__fastcall* D2CLIENT_PrintGameString)(const wchar_t* wMessage, int nColor) = nullptr;
inline void(__fastcall* D2CLIENT_PrintPartyString)(const wchar_t* wMessage, int nColor) = nullptr;
inline void(__fastcall* D2CLIENT_LeaveParty)(void) = nullptr;
inline void(__fastcall* D2CLIENT_AcceptTrade)(void) = nullptr;
inline void(__fastcall* D2CLIENT_CancelTrade)(void) = nullptr;
inline void(__stdcall* D2CLIENT_TradeOK)(void) = nullptr;
inline uint8_t(__stdcall* D2CLIENT_GetDifficulty)() = nullptr;
inline void(__fastcall* D2CLIENT_ExitGame)(void) = nullptr;
inline uint32_t(__fastcall* D2CLIENT_GetUiVar_I)(uint32_t dwVarNo) = nullptr;
inline void(__fastcall* D2CLIENT_DrawRectFrame)(uint32_t Rect) = nullptr;
inline void(__fastcall* D2CLIENT_PerformGoldDialogAction)(void) = nullptr;
inline D2UnitStrc*(__stdcall* D2CLIENT_GetPlayerUnit)() = nullptr;
inline void(__fastcall* D2CLIENT_ClearScreen)(void) = nullptr;
inline uint32_t(__stdcall* D2CLIENT_CloseNPCTalk)(void* unk) = nullptr;
inline uint32_t(__fastcall* D2CLIENT_TestPvpFlag)(uint32_t dwUnitId1, uint32_t dwUnitId2, uint32_t dwFlag) = nullptr;
inline uint32_t(__fastcall* D2CLIENT_GetGameLanguageCode)() = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////
// D2Client Globals
////////////////////////////////////////////////////////////////////////////////////////////////

#include <Windows.h>  // POINT

inline uint32_t* D2CLIENT_ScreenSizeX = nullptr;
inline uint32_t* D2CLIENT_ScreenSizeY = nullptr;

inline uint32_t* D2CLIENT_CursorHoverX = nullptr;
inline uint32_t* D2CLIENT_CursorHoverY = nullptr;
inline uint32_t* D2CLIENT_RegularCursorType = nullptr;
inline uint32_t* D2CLIENT_ShopCursorType = nullptr;
inline uint32_t* D2CLIENT_MouseY = nullptr;
inline uint32_t* D2CLIENT_MouseX = nullptr;
inline int* D2CLIENT_MouseOffsetY = nullptr;
inline int* D2CLIENT_MouseOffsetZ = nullptr;
inline int* D2CLIENT_MouseOffsetX = nullptr;

inline uint32_t* D2CLIENT_MercReviveCost = nullptr;

inline int* D2CLIENT_ViewportY = nullptr;
inline int* D2CLIENT_ViewportX = nullptr;

inline uint32_t* D2CLIENT_GoldDialogAction = nullptr;
inline uint32_t* D2CLIENT_GoldDialogAmount = nullptr;

inline uint32_t* D2CLIENT_Ping = nullptr;
inline uint32_t* D2CLIENT_Skip = nullptr;
inline uint32_t* D2CLIENT_FPS = nullptr;
inline uint32_t* D2CLIENT_Lang = nullptr;

inline uint32_t* D2CLIENT_OverheadTrigger = nullptr;
inline uint32_t* D2CLIENT_RecentInteractId = nullptr;
inline uint32_t* D2CLIENT_ItemPriceList = nullptr;
inline uint32_t* D2CLIENT_WaypointTable = nullptr;
inline uint32_t* D2CLIENT_bWeapSwitch = nullptr;
inline uint32_t* D2CLIENT_bTradeAccepted = nullptr;
inline uint32_t* D2CLIENT_bTradeBlock = nullptr;
inline uint32_t* D2CLIENT_RecentTradeId = nullptr;
inline uint32_t* D2CLIENT_ExpCharFlag = nullptr;
inline uint32_t* D2CLIENT_MapId = nullptr;
inline uint32_t* D2CLIENT_AlwaysRun = nullptr;
inline uint32_t* D2CLIENT_NoPickUp = nullptr;
inline wchar_t** D2CLIENT_ChatMsg = nullptr;
inline uint32_t* D2CLIENT_OrificeId = nullptr;
inline uint32_t* D2CLIENT_CursorItemMode = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////
// D2Client Stubs
////////////////////////////////////////////////////////////////////////////////////////////////

inline uint32_t D2CLIENT_TakeWaypoint_I = NULL;
inline uint32_t D2CLIENT_ClickShopItem_I = NULL;
inline uint32_t D2CLIENT_ClickBelt_I = NULL;
inline uint32_t D2CLIENT_ClickBeltRight_I = NULL;
inline uint32_t D2CLIENT_ClickItemRight_I = NULL;
inline uint32_t D2CLIENT_MercItemAction_I = NULL;
inline uint32_t D2CLIENT_Interact_I = NULL;
inline uint32_t D2CLIENT_ClickParty_I = NULL;
inline uint32_t D2CLIENT_ClickParty_II = NULL;
inline uint32_t D2CLIENT_ShopAction_I = NULL;
inline uint32_t D2CLIENT_GetUnitName_I = NULL;
inline uint32_t D2CLIENT_GetItemDesc_I = NULL;
inline uint32_t D2CLIENT_AssignPlayer_I = NULL;
inline uint32_t D2CLIENT_InputCall_I = NULL;
inline uint32_t D2CLIENT_Say_I = NULL;
inline uint32_t D2CLIENT_BodyClickTable = NULL;
inline uint32_t D2CLIENT_LoadUIImage_I = NULL;
inline uint32_t D2CLIENT_GetMinionCount_I = NULL;
inline uint32_t D2CLIENT_GameLeave_I = NULL;
inline uint32_t D2CLIENT_CongratsScreen_I = NULL;
inline uint32_t D2CLIENT_GameAddUnit_I = NULL;
inline uint32_t D2CLIENT_SendGamePacket_I = NULL;
inline uint32_t D2CLIENT_SendPacket_II = NULL;

////////////////////////////////////////////////////////////////////////////////////////////////
// D2Common Ordinals
////////////////////////////////////////////////////////////////////////////////////////////////

struct D2UnitStrc;
struct D2InventoryStrc;
struct D2ObjectsTxt;
struct D2LevelsTxt;
struct D2ItemsTxt;
struct D2DrlgActStrc;
struct D2DrlgLevelStrc;
struct D2StatListExStrc;
struct D2StatStrc;
struct D2ActiveRoomStrc;
struct D2DynamicPathStrc;

inline void(__stdcall* D2COMMON_InitLevel)(D2DrlgLevelStrc* pLevel) = nullptr;
inline unsigned(__stdcall* D2COMMON_UnloadAct)(D2DrlgActStrc* pAct) = nullptr;
inline D2ObjectsTxt*(__stdcall* D2COMMON_GetObjectTxt)(DWORD objno) = nullptr;
inline D2DrlgActStrc*(__stdcall* D2COMMON_LoadAct)(DWORD ActNumber, DWORD MapId, DWORD Unk, DWORD Unk_2, DWORD Unk_3, DWORD Unk_4, DWORD TownLevelId, DWORD Func_1,
                                                   DWORD Func_2) = nullptr;
inline D2LevelsTxt*(__stdcall* D2COMMON_GetLevelText)(DWORD levelno) = nullptr;
inline D2ObjectsTxt*(__stdcall* D2COMMON_GetObjectText)(DWORD objno) = nullptr;
inline D2ItemsTxt*(__stdcall* D2COMMON_GetItemText)(DWORD itemno) = nullptr;
inline D2DrlgLevelStrc*(__fastcall* D2COMMON_GetLevel)(D2DrlgStrc* pMisc, DWORD dwLevelNo) = nullptr;
inline D2StatListExStrc*(__stdcall* D2COMMON_GetStatList)(D2UnitStrc* pUnit, DWORD dwUnk, DWORD dwMaxEntries) = nullptr;
inline DWORD(__stdcall* D2COMMON_CopyStatList)(D2StatListExStrc* pStatList, D2StatStrc* pStatArray, DWORD dwMaxEntries) = nullptr;
inline DWORD(__stdcall* D2COMMON_GetUnitStat)(D2UnitStrc* pUnit, DWORD dwStat, DWORD dwStat2) = nullptr;
inline int(__stdcall* D2COMMON_GetUnitState)(D2UnitStrc* pUnit, DWORD dwStateNo) = nullptr;
inline DWORD(__stdcall* D2COMMON_CheckUnitCollision)(D2UnitStrc* pUnitA, D2UnitStrc* pUnitB, DWORD dwBitMask) = nullptr;
inline D2ActiveRoomStrc*(__stdcall* D2COMMON_GetRoomFromUnit)(D2UnitStrc* ptUnit) = nullptr;
inline D2DynamicPathStrc*(__stdcall* D2COMMON_GetTargetUnitType)(D2DynamicPathStrc* pPath) = nullptr;
inline INT(__stdcall* D2COMMON_GetSkillLevel)(D2UnitStrc* pUnit, D2SkillStrc* pSkill, BOOL bTotal) = nullptr;
inline DWORD(__stdcall* D2COMMON_GetItemLevelRequirement)(D2UnitStrc* pItem, D2UnitStrc* pPlayer) = nullptr;
inline DWORD(__stdcall* D2COMMON_GetItemPrice)(D2UnitStrc* MyUnit, D2UnitStrc* pItem, DWORD U1_, DWORD U2_, DWORD U3_, DWORD U4_) = nullptr;
inline DWORD(__stdcall* D2COMMON_GetRepairCost)(DWORD _1, D2UnitStrc* pUnit, DWORD dwNpcId, DWORD dwDifficulty, DWORD dwItemPriceList, DWORD _2) = nullptr;
inline char*(__stdcall* D2COMMON_GetItemMagicalMods)(WORD wPrefixNum) = nullptr;
inline D2UnitStrc*(__stdcall* D2COMMON_GetItemFromInventory)(D2InventoryStrc* inv) = nullptr;
inline D2UnitStrc*(__stdcall* D2COMMON_GetNextItemFromInventory)(D2UnitStrc* pItem) = nullptr;
inline void(__stdcall* D2COMMON_AddRoomData)(D2DrlgActStrc* ptAct, int LevelId, int Xpos, int Ypos, D2ActiveRoomStrc* pRoom) = nullptr;
inline void(__stdcall* D2COMMON_RemoveRoomData)(D2DrlgActStrc* ptAct, int LevelId, int Xpos, int Ypos, D2ActiveRoomStrc* pRoom) = nullptr;
inline int(__stdcall* D2COMMON_GetQuestFlag)(void* QuestInfo, DWORD dwAct, DWORD dwQuest) = nullptr;
inline void(__stdcall* D2COMMON_MapToAbsScreen)(long* pX, long* pY) = nullptr;
inline void(__stdcall* D2COMMON_AbsScreenToMap)(long* ptMouseX, long* ptMouseY) = nullptr;
inline DWORD(__stdcall* D2COMMON_CheckWaypoint)(DWORD WaypointTable, DWORD dwLevelId) = nullptr;
inline BOOL(__stdcall* D2COMMON_IsTownByLevelNo)(DWORD dwLevelNo) = nullptr;
inline BOOL(__stdcall* D2COMMON_GetLevelNoFromRoom)(D2ActiveRoomStrc* pRoom1) = nullptr;
inline D2ActiveRoomStrc*(__stdcall* D2COMMON_FindRoom1)(D2DrlgActStrc* pAct, int x, int y) = nullptr;
inline int(__stdcall* D2COMMON_GetItemPalette)(D2UnitStrc* pPlayer, D2UnitStrc* pItem, BYTE* pColor, int nTransType) = nullptr;
inline D2UnitStrc*(__fastcall* D2COMMON_GetMissileOwnerUnit)(D2UnitStrc* pMissile) = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////
// D2Common Globals
////////////////////////////////////////////////////////////////////////////////////////////////

inline DWORD* D2COMMON_sgptDataTable = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////
// D2Common Stubs
////////////////////////////////////////////////////////////////////////////////////////////////

inline uint32_t D2COMMON_GetLevelIdFromRoom_I = NULL;

////////////////////////////////////////////////////////////////////////////////////////////////
// D2Net Functions
////////////////////////////////////////////////////////////////////////////////////////////////

inline void(__stdcall* D2NET_SendPacket)(size_t aLen, DWORD arg1, BYTE* aPacket) = nullptr;
inline void(__fastcall* D2NET_ReceivePacket)(BYTE* aPacket, DWORD aLen) = nullptr;
inline void(__fastcall* D2NET_ReceivePacket_I)(BYTE* aPacket, DWORD aLen, int* arg3) = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////
// D2Net Globals
////////////////////////////////////////////////////////////////////////////////////////////////

inline CRITICAL_SECTION* D2NET_CriticalPacketSection = nullptr;  // 0xB400

////////////////////////////////////////////////////////////////////////////////////////////////
// D2Gfx Ordinals
////////////////////////////////////////////////////////////////////////////////////////////////

struct D2GfxDataStrc;

inline void(__stdcall* D2GFX_DrawRectangle)(int X1, int Y1, int X2, int Y2, DWORD dwColor, DWORD dwTrans) = nullptr;
inline void(__stdcall* D2GFX_DrawLine)(int X1, int Y1, int X2, int Y2, DWORD dwColor, DWORD dwUnk) = nullptr;
inline void(__stdcall* D2GFX_DrawAutomapCell2)(D2GfxDataStrc* context, DWORD xpos, DWORD ypos, DWORD bright2, DWORD bright, BYTE* coltab) = nullptr;
inline HWND(__stdcall* D2GFX_GetHwnd)(void) = nullptr;
inline DWORD(__stdcall* D2GFX_GetScreenSize)() = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////
// D2Multi Functions
////////////////////////////////////////////////////////////////////////////////////////////////

inline void(__fastcall* D2MULTI_DoChat)(void) = nullptr;
inline void(__fastcall* D2MULTI_PrintChannelText_)(int unused, char* szText, DWORD dwColor) = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////
// D2Multi Globals
////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////
// D2Multi Stubs
////////////////////////////////////////////////////////////////////////////////////////////////

inline uint32_t D2MULTI_ChannelChat_I = NULL;
inline uint32_t D2MULTI_ChannelEmote_I = NULL;
inline uint32_t D2MULTI_ChannelWhisper_I = NULL;
inline uint32_t D2MULTI_ChannelInput_I = NULL;

////////////////////////////////////////////////////////////////////////////////////////////////
// D2Cmp Ordinals
////////////////////////////////////////////////////////////////////////////////////////////////

struct D2CellFileStrc;

inline void(__stdcall* D2CMP_InitCellFile)(LPVOID File, D2CellFileStrc** Out, LPCSTR SourceFile, DWORD Line, DWORD FileVersion, LPCSTR Filename) = nullptr;
inline void(__stdcall* D2CMP_DeleteCellFile)(D2CellFileStrc* File) = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////
// D2Lang Ordinals
////////////////////////////////////////////////////////////////////////////////////////////////

inline wchar_t*(__fastcall* D2LANG_GetLocaleText)(WORD nLocaleTxtNo) = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////
// D2Lang Stubs
////////////////////////////////////////////////////////////////////////////////////////////////

inline uint32_t D2LANG_Say_II = NULL;

////////////////////////////////////////////////////////////////////////////////////////////////
// D2Launch Globals
////////////////////////////////////////////////////////////////////////////////////////////////

inline BnetData** D2LAUNCH_BnData = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////
// D2Win Functions
////////////////////////////////////////////////////////////////////////////////////////////////

inline void(__fastcall* D2WIN_DrawSprites)(void) = nullptr;
inline D2CellFileStrc*(__fastcall* D2WIN_LoadCellFile)(const char* szFile, int Type) = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////
// D2Win Ordinals
////////////////////////////////////////////////////////////////////////////////////////////////

inline void(__fastcall* D2WIN_TakeScreenshot)() = nullptr;
inline void(__fastcall* D2WIN_DrawText)(const wchar_t* wStr, int xPos, int yPos, DWORD dwColor, DWORD dwUnk) = nullptr;
inline DWORD(__fastcall* D2WIN_GetTextSize)(const wchar_t* wStr, DWORD* dwWidth, DWORD* dwFileNo) = nullptr;
inline DWORD(__fastcall* D2WIN_SetTextSize)(DWORD dwSize) = nullptr;
inline DWORD(__fastcall* D2WIN_GetTextWidthFileNo)(wchar_t* wStr, DWORD* dwWidth, DWORD* dwFileNo) = nullptr;
inline DWORD(__fastcall* D2WIN_InitMPQ)(const char* mpqfile, char* mpqname, int v4, int v5) = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////
// D2Win Globals
////////////////////////////////////////////////////////////////////////////////////////////////

inline DWORD** D2MULTI_ChatInputBox = nullptr;

inline uint32_t BNCLIENT_DClass = NULL;
inline uint32_t BNCLIENT_DLod = NULL;

inline char** BNCLIENT_ClassicKey = nullptr;
inline char** BNCLIENT_XPacKey = nullptr;
inline char** BNCLIENT_KeyOwner = nullptr;
inline char(__cdecl* BNCLIENTDecodeAndLoadKeys)() = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////
// D2Game Functions
////////////////////////////////////////////////////////////////////////////////////////////////

inline uint32_t(__fastcall* D2GAME_Rand)(uint32_t* seed) = nullptr;
inline DWORD(__fastcall* D2GAME_exit0)() = nullptr;

#define D2MULTI_PrintChannelText(text, color) (D2MULTI_PrintChannelText_(0, text, color))
#define D2CLIENT_GetUIState(dwVarNo) (D2CLIENT_GetUIVar_STUB(dwVarNo))
#define D2CLIENT_SetUIState(dwVarNo, value) (D2CLIENT_SetUIVar(dwVarNo, value, 0))
#define D2CLIENT_InitAutomapLayer(layerlvl) ((D2AutomapLayerStrc*)D2CLIENT_InitAutomapLayer_STUB(layerlvl))
#define D2CLIENT_GetUnitName(x) (wchar_t*)D2CLIENT_GetUnitName_STUB((DWORD)x)
#define D2CLIENT_SetSelectedUnit(x) (D2CLIENT_SetSelectedUnit_STUB((DWORD)x))
#define D2CLIENT_Interact_STUB(x) (D2CLIENT_Interact_ASM((DWORD)x))
#define D2CLIENT_ClickParty(x, y) (D2CLIENT_ClickParty_ASM((DWORD)x, (DWORD)y))
#define D2CLIENT_RightClickItem(x, y, loc, player, invdata) D2CLIENT_ClickItemRight_ASM(x, y, loc, (DWORD)player, (DWORD)invdata)
#define D2CLIENT_ClickBeltRight(pPlayer, pInventory, dwShift, dwPotPos) D2CLIENT_ClickBeltRight_ASM((DWORD)pPlayer, (DWORD)pInventory, dwShift, dwPotPos)
#define D2CLIENT_GetItemDesc(pUnit, pBuffer) D2CLIENT_GetItemDesc_ASM((DWORD)pUnit, pBuffer)
#define D2CLIENT_MercItemAction(bPacketType, dwSlotId) D2CLIENT_MercItemAction_ASM(bPacketType, dwSlotId)
#define D2CLIENT_SendGamePacket(dwLen, bPacket) D2CLIENT_SendGamePacket_ASM(dwLen, bPacket)

#define D2GFX_DrawFrame(Rect) D2GFX_DrawRectFrame_STUB(Rect)