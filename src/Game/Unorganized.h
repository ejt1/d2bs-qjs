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

#pragma warning(push)
#pragma warning(disable : 4201)
#pragma optimize("", off)

struct D2UnitStrc;
struct D2ActiveRoomStrc;
struct D2DrlgRoomStrc;
struct D2DrlgLevelStrc;
struct D2DrlgActStrc;
struct D2DrlgStrc;
struct D2RosterUnitStrc;
struct OverheadMsg;
struct D2SkillStrc;

// rename D2AutomapCellStrc
struct AutomapCell {
  uint32_t fSaved;     // 0x00
  uint16_t nCellNo;    // 0x04
  uint16_t xPixel;     // 0x06
  uint16_t yPixel;     // 0x08
  uint16_t wWeight;    // 0x0A
  AutomapCell* pLess;  // 0x0C
  AutomapCell* pMore;  // 0x10
};

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

// rename D2AutomapLayerStrc
struct AutomapLayer {
  uint32_t nLayerNo;         // 0x00
  uint32_t fSaved;           // 0x04
  AutomapCell* pFloors;      // 0x08
  AutomapCell* pWalls;       // 0x0C
  AutomapCell* pObjects;     // 0x10
  AutomapCell* pExtras;      // 0x14
  AutomapLayer* pNextLayer;  // 0x18
};

struct AutomapLayer2 {
  uint32_t _1[2];     // 0x00
  uint32_t nLayerNo;  // 0x08
};

struct ControlText  // size = 0x20
{
  wchar_t* wText[5];   // 0x00 for each field
  uint32_t dwColor;    // 0x14
  uint32_t dwAlign;    // 0x18
  ControlText* pNext;  // 0x1C
};

struct Control {
  uint32_t dwType;      // 0x00
  uint32_t* _1;         // 0x04 // unsure? definitely a ptr but not obvious, usually points to 6 when dwType is 6 I think
  uint32_t dwDisabled;  // 0x08
  uint32_t dwPosX;      // 0x0C
  uint32_t dwPosY;      // 0x10
  uint32_t dwSizeX;     // 0x14
  uint32_t dwSizeY;     // 0x18
  // I think _2 thru _9 are a handler table of some sort
  uint32_t* _2;                // 0x1C // some sort of function (maybe click?)
  uint32_t _3;                 // 0x20
  uint32_t* _4;                // 0x24 // some sort of function
  uint32_t* _5;                // 0x28
  uint32_t _6;                 // 0x2C
  uint32_t* _7;                // 0x30 // ptr to something...
  uint32_t* _8;                // 0x34 // another random ptr... mostly dead ends when I examined them
  uint32_t _9;                 // 0x38
  Control* pNext;              // 0x3C
  uint32_t _10;                // 0x40
  uint32_t unkState;           // 0x44 _11 0 when button avail to be clicked 1 when greyed - still need to look at this more
  ControlText* pFirstText;     // 0x48
  ControlText* pLastText;      // 0x4C
  ControlText* pSelectedText;  // 0x50
  uint32_t dwSelectEnd;        // 0x54
  uint32_t dwSelectStart;      // 0x58
  union {
    struct {               // Textboxes
      wchar_t wText[256];  // 0x5C
      uint32_t dwCursorPos;
      uint32_t dwIsCloaked;
    };
    struct {                // Buttons
      uint32_t _12[2];      // 0x5C
      wchar_t wText2[256];  // 0x6C
    };
  };
};

#pragma pack(push)
#pragma pack(1)

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

struct WardenClientRegion_t {
  uint32_t cbAllocSize;              //+00
  uint32_t offsetFunc1;              //+04
  uint32_t offsetRelocAddressTable;  //+08
  uint32_t nRelocCount;              //+0c
  uint32_t offsetWardenSetup;        //+10
  uint32_t _2[2];
  uint32_t offsetImportAddressTable;  //+1c
  uint32_t nImportDllCount;           //+20
  uint32_t nSectionCount;             //+24
};

struct SMemBlock_t {
  uint32_t _1[6];
  uint32_t cbSize;  //+18
  uint32_t _2[31];
  uint8_t data[1];  //+98
};

struct WardenClient_t {
  WardenClientRegion_t* pWardenRegion;  //+00
  uint32_t cbSize;                      //+04
  uint32_t nModuleCount;                //+08
  uint32_t param;                       //+0c
  uint32_t fnSetupWarden;               //+10
};

struct WardenIATInfo_t {
  uint32_t offsetModuleName;
  uint32_t offsetImportTable;
};

#pragma pack(push)
#pragma pack(1)

struct NPCMenu {
  uint32_t dwNPCClassId;
  uint32_t dwEntryAmount;
  uint16_t wEntryId1;
  uint16_t wEntryId2;
  uint16_t wEntryId3;
  uint16_t wEntryId4;
  uint16_t _1;
  uint32_t dwEntryFunc1;
  uint32_t dwEntryFunc2;
  uint32_t dwEntryFunc3;
  uint32_t dwEntryFunc4;
  uint8_t _2[5];
};

struct OverheadMsg {
  uint32_t _1;
  uint32_t dwTrigger;
  uint32_t _2[2];
  char Msg[232];
};

#pragma pack(pop)

struct D2MSG {
  void* myHWND;
  char lpBuf[256];
};

struct MessageHandlerList {
  uint32_t message;
  uint32_t unk_4;
  uint32_t(__stdcall* handler)(void*);
  struct MessageHandlerList* next;
};

struct MessageHandlerHashTable {
  struct MessageHandlerList** table;
  uint32_t length;
};

struct WindowHandlerHashTable {
  struct WindowHandlerList** table;
  uint32_t length;
};

struct WindowHandlerList {
  uint32_t unk_0;
  void* hWnd;
  uint32_t unk_8;
  struct MessageHandlerHashTable* msgHandlers;
  struct WindowHandlerList* next;
};

// Not sure of the location of handler and this struct inside Info.
// Could be this struct is later and handler is earlier, but this is the safest
// for now.
struct TransactionDialogsLine_t {
  wchar_t text[120];           // 0x000
  uint32_t unk[6];             // 0x0F0
  void(__stdcall* handler)();  // 0x108
  uint32_t bMaybeSelectable;   // 0x10C
};

struct TransactionDialogsInfo_t {
  uint32_t unk[0x14];                        // 0x000
  uint32_t numLines;                         // 0x050
  uint32_t unk_2[0x5];                       // 0x054
  TransactionDialogsLine_t dialogLines[10];  // 0x068
  void* something;                           // 0xB08
};

#pragma warning(pop)
#pragma optimize("", on)

inline void(__fastcall* D2CLIENT_SubmitItem)(uint32_t dwItemId) = nullptr;
inline void(__fastcall* D2CLIENT_Transmute)(void) = nullptr;
inline void(__fastcall* D2CLIENT_CloseNPCInteract)(void) = nullptr;
inline void(__fastcall* D2CLIENT_CloseInteract)(void) = nullptr;
inline uint32_t(__stdcall* D2CLIENT_GetAutomapSize)(void) = nullptr;
inline AutomapCell*(__fastcall* D2CLIENT_NewAutomapCell)() = nullptr;
inline void(__fastcall* D2CLIENT_AddAutomapCell)(AutomapCell* aCell, AutomapCell** node) = nullptr;
inline void(__stdcall* D2CLIENT_RevealAutomapRoom)(D2ActiveRoomStrc* pRoom1, uint32_t dwClipFlag, AutomapLayer* aLayer) = nullptr;
inline AutomapLayer*(__fastcall* D2CLIENT_InitAutomapLayer_I)(uint32_t nLayerNo) = nullptr;
inline void(__fastcall* D2CLIENT_ClickMap)(uint32_t MouseFlag, uint32_t x, uint32_t y, uint32_t Type) = nullptr;
inline uint32_t(__fastcall* D2CLIENT_GetMouseXOffset)(void) = nullptr;
inline uint32_t(__fastcall* D2CLIENT_GetMouseYOffset)(void) = nullptr;
inline void(__fastcall* D2CLIENT_PrintGameString)(wchar_t* wMessage, int nColor) = nullptr;
inline void(__fastcall* D2CLIENT_PrintPartyString)(wchar_t* wMessage, int nColor) = nullptr;
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
inline uint32_t* D2CLIENT_MouseY = nullptr;
inline uint32_t* D2CLIENT_MouseX = nullptr;
inline int* D2CLIENT_MouseOffsetY = nullptr;
inline int* D2CLIENT_MouseOffsetZ = nullptr;
inline int* D2CLIENT_MouseOffsetX = nullptr;
inline uint32_t* D2CLIENT_AutomapOn = nullptr;
inline int* D2CLIENT_AutomapMode = nullptr;
inline POINT* D2CLIENT_Offset = nullptr;
inline AutomapLayer** D2CLIENT_AutomapLayer = nullptr;
inline uint32_t* D2CLIENT_MercReviveCost = nullptr;
inline int* D2CLIENT_ViewportY = nullptr;
inline int* D2CLIENT_ViewportX = nullptr;
inline uint32_t* D2CLIENT_GoldDialogAction = nullptr;
inline uint32_t* D2CLIENT_GoldDialogAmount = nullptr;
inline NPCMenu** D2CLIENT_NPCMenu = nullptr;
inline uint32_t* D2CLIENT_NPCMenuAmount = nullptr;
inline uint32_t* D2CLIENT_RegularCursorType = nullptr;
inline uint32_t* D2CLIENT_ShopCursorType = nullptr;
inline uint32_t* D2CLIENT_Ping = nullptr;
inline uint32_t* D2CLIENT_Skip = nullptr;
inline uint32_t* D2CLIENT_FPS = nullptr;
inline uint32_t* D2CLIENT_Lang = nullptr;
inline int* D2CLIENT_Divisor = nullptr;
inline uint32_t* D2CLIENT_OverheadTrigger = nullptr;
inline uint32_t* D2CLIENT_RecentInteractId = nullptr;
inline uint32_t* D2CLIENT_ItemPriceList = nullptr;
inline void** D2CLIENT_TransactionDialog = nullptr;
inline uint32_t* D2CLIENT_TransactionDialogs = nullptr;
inline uint32_t* D2CLIENT_TransactionDialogs_2 = nullptr;
inline TransactionDialogsInfo_t** D2CLIENT_pTransactionDialogsInfo = nullptr;
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
struct AutomapLayer2;
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
inline AutomapLayer2*(__fastcall* D2COMMON_GetLayer)(DWORD dwLevelNo) = nullptr;
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
inline OverheadMsg*(__stdcall* D2COMMON_GenerateOverheadMsg)(DWORD dwUnk, const char* szMsg, DWORD dwTrigger) = nullptr;
inline VOID(__stdcall* D2COMMON_FixOverheadMsg)(OverheadMsg* pMsg, DWORD dwUnk) = nullptr;
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

inline uint32_t D2COMMON_DisplayOverheadMsg_I = NULL;
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

inline char** D2MULTI_ChatBoxMsg = nullptr;
inline Control** D2MULTI_GameListControl = nullptr;

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

inline void*(__fastcall* D2WIN_SetControlText)(Control* box, const wchar_t* txt) = nullptr;
inline void(__fastcall* D2WIN_DrawSprites)(void) = nullptr;
inline D2CellFileStrc*(__fastcall* D2WIN_LoadCellFile)(const char* szFile, int Type) = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////
// D2Win Ordinals
////////////////////////////////////////////////////////////////////////////////////////////////

inline void(__fastcall* D2WIN_TakeScreenshot)() = nullptr;
inline void(__fastcall* D2WIN_DrawText)(const wchar_t* wStr, int xPos, int yPos, DWORD dwColor, DWORD dwUnk) = nullptr;
inline DWORD(__fastcall* D2WIN_GetTextSize)(wchar_t* wStr, DWORD* dwWidth, DWORD* dwFileNo) = nullptr;
inline DWORD(__fastcall* D2WIN_SetTextSize)(DWORD dwSize) = nullptr;
inline DWORD(__fastcall* D2WIN_GetTextWidthFileNo)(wchar_t* wStr, DWORD* dwWidth, DWORD* dwFileNo) = nullptr;
inline DWORD(__fastcall* D2WIN_DestroyEditBox)(Control* box) = nullptr;
inline VOID(__stdcall* D2WIN_DestroyControl)(Control* pControl) = nullptr;
inline VOID(__fastcall* D2WIN_SetEditBoxCallback)(Control* pControl, BOOL(__stdcall* FunCallBack)(Control* pControl, DWORD dwInputType, char* pChar)) = nullptr;
inline void(__fastcall* D2WIN_SetEditBoxProc)(Control* box, BOOL(__stdcall* FunCallBack)(Control*, DWORD, DWORD)) = nullptr;
inline void(__fastcall* D2WIN_SelectEditBoxText)(Control* box) = nullptr;
inline DWORD(__fastcall* D2WIN_InitMPQ)(const char* mpqfile, char* mpqname, int v4, int v5) = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////
// D2Win Globals
////////////////////////////////////////////////////////////////////////////////////////////////

inline Control** D2WIN_FirstControl = nullptr;
inline Control** D2WIN_FocusedControl = nullptr;
inline DWORD** D2MULTI_ChatInputBox = nullptr;

inline uint32_t BNCLIENT_DClass = NULL;
inline uint32_t BNCLIENT_DLod = NULL;

inline char** BNCLIENT_ClassicKey = nullptr;
inline char** BNCLIENT_XPacKey = nullptr;
inline char** BNCLIENT_KeyOwner = nullptr;
inline WindowHandlerHashTable* STORM_WindowHandlers = nullptr;
inline char(__cdecl* BNCLIENTDecodeAndLoadKeys)() = nullptr;

////////////////////////////////////////////////////////////////////////////////////////////////
// D2Game Functions
////////////////////////////////////////////////////////////////////////////////////////////////

inline uint32_t(__fastcall* D2GAME_Rand)(uint32_t* seed) = nullptr;
inline DWORD(__fastcall* D2GAME_exit0)() = nullptr;

#define D2MULTI_PrintChannelText(text, color) (D2MULTI_PrintChannelText_(0, text, color))
#define D2CLIENT_GetUIState(dwVarNo) (D2CLIENT_GetUIVar_STUB(dwVarNo))
#define D2CLIENT_SetUIState(dwVarNo, value) (D2CLIENT_SetUIVar(dwVarNo, value, 0))
#define D2CLIENT_InitAutomapLayer(layerlvl) ((AutomapLayer*)D2CLIENT_InitAutomapLayer_STUB(layerlvl))
#define D2CLIENT_GetUnitName(x) (wchar_t*)D2CLIENT_GetUnitName_STUB((DWORD)x)
#define D2CLIENT_SetSelectedUnit(x) (D2CLIENT_SetSelectedUnit_STUB((DWORD)x))
#define D2CLIENT_Interact_STUB(x) (D2CLIENT_Interact_ASM((DWORD)x))
#define D2CLIENT_ClickParty(x, y) (D2CLIENT_ClickParty_ASM((DWORD)x, (DWORD)y))
#define D2CLIENT_RightClickItem(x, y, loc, player, invdata) D2CLIENT_ClickItemRight_ASM(x, y, loc, (DWORD)player, (DWORD)invdata)
#define D2CLIENT_ClickBeltRight(pPlayer, pInventory, dwShift, dwPotPos) D2CLIENT_ClickBeltRight_ASM((DWORD)pPlayer, (DWORD)pInventory, dwShift, dwPotPos)
#define D2CLIENT_GetItemDesc(pUnit, pBuffer) D2CLIENT_GetItemDesc_ASM((DWORD)pUnit, pBuffer)
#define D2CLIENT_MercItemAction(bPacketType, dwSlotId) D2CLIENT_MercItemAction_ASM(bPacketType, dwSlotId)
#define D2CLIENT_SendGamePacket(dwLen, bPacket) D2CLIENT_SendGamePacket_ASM(dwLen, bPacket)

#define D2COMMON_DisplayOverheadMsg(pUnit) D2COMMON_DisplayOverheadMsg_ASM((DWORD)pUnit)

#define D2GFX_DrawFrame(Rect) D2GFX_DrawRectFrame_STUB(Rect)