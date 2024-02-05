#pragma once

// file contains structs that needs organizing
// DO NOT ADD NEW STRUCTS HERE!

#include "UnorganizedConstants.h"

#include <Windows.h>

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
  DWORD fSaved;        // 0x00
  WORD nCellNo;        // 0x04
  WORD xPixel;         // 0x06
  WORD yPixel;         // 0x08
  WORD wWeight;        // 0x0A
  AutomapCell* pLess;  // 0x0C
  AutomapCell* pMore;  // 0x10
};

struct UnitInteraction {
  DWORD dwMoveType;       // 0x00
  D2UnitStrc* lpPlayerUnit;  // 0x04
  D2UnitStrc* lpTargetUnit;  // 0x08
  DWORD dwTargetX;        // 0x0C
  DWORD dwTargetY;        // 0x10
  DWORD _1;               // 0x14
  DWORD _2;               // 0x18
  D2SkillStrc* pSkill;
};

// rename D2AutomapLayerStrc
struct AutomapLayer {
  DWORD nLayerNo;            // 0x00
  DWORD fSaved;              // 0x04
  AutomapCell* pFloors;      // 0x08
  AutomapCell* pWalls;       // 0x0C
  AutomapCell* pObjects;     // 0x10
  AutomapCell* pExtras;      // 0x14
  AutomapLayer* pNextLayer;  // 0x18
};

struct AutomapLayer2 {
  DWORD _1[2];     // 0x00
  DWORD nLayerNo;  // 0x08
};

struct ControlText  // size = 0x20
{
  wchar_t* wText[5];   // 0x00 for each field
  DWORD dwColor;       // 0x14
  DWORD dwAlign;       // 0x18
  ControlText* pNext;  // 0x1C
};

struct Control {
  DWORD dwType;      // 0x00
  DWORD* _1;         // 0x04 // unsure? definitely a ptr but not obvious, usually points to 6 when dwType is 6 I think
  DWORD dwDisabled;  // 0x08
  DWORD dwPosX;      // 0x0C
  DWORD dwPosY;      // 0x10
  DWORD dwSizeX;     // 0x14
  DWORD dwSizeY;     // 0x18
  // I think _2 thru _9 are a handler table of some sort
  DWORD* _2;                   // 0x1C // some sort of function (maybe click?)
  DWORD _3;                    // 0x20
  DWORD* _4;                   // 0x24 // some sort of function
  DWORD* _5;                   // 0x28
  DWORD _6;                    // 0x2C
  DWORD* _7;                   // 0x30 // ptr to something...
  DWORD* _8;                   // 0x34 // another random ptr... mostly dead ends when I examined them
  DWORD _9;                    // 0x38
  Control* pNext;              // 0x3C
  DWORD _10;                   // 0x40
  DWORD unkState;              // 0x44 _11 0 when button avail to be clicked 1 when greyed - still need to look at this more
  ControlText* pFirstText;     // 0x48
  ControlText* pLastText;      // 0x4C
  ControlText* pSelectedText;  // 0x50
  DWORD dwSelectEnd;           // 0x54
  DWORD dwSelectStart;         // 0x58
  union {
    struct {               // Textboxes
      wchar_t wText[256];  // 0x5C
      DWORD dwCursorPos;
      DWORD dwIsCloaked;
    };
    struct {                // Buttons
      DWORD _12[2];         // 0x5C
      wchar_t wText2[256];  // 0x6C
    };
  };
};

#pragma pack(push)
#pragma pack(1)

struct BnetData {
  DWORD dwId;                  // 0x00
  DWORD dwId2;                 // 0x04
  BYTE _1[0x10];               // 0x08
  DWORD dwId3;                 // 0x18
  WORD Unk3;                   // 0x1C
  BYTE _2;                     // 0x1E
  char szGameName[0x16];       // 0x1F
  WORD _3;                     // 0x35
  char szGameIP[0x10];         // 0x37
  BYTE _5[0x42];               // 0x47
  DWORD dwId4;                 // 0x89
  char szAccountName[0x30];    // 0x8D
  char szPlayerName[0x18];     // 0xBD
  char szRealmName[0x08];      // 0xD5
  BYTE _8[0x111];              // 0xDD
  BYTE nCharClass;             // 0x1EE
  BYTE nCharFlags;             // 0x1EF
  BYTE nMaxDiff;               // 0x1F0
  BYTE _9[0x1F];               // 0x1F1
  BYTE CreatedGameDifficulty;  // 0x210
  void* _10;                   // 0x211
  BYTE _11[0x15];              // 0x215
  WORD _12;                    // 0x22A
  BYTE _13;                    // 0x22C
  char szRealmName2[0x18];     // 0x22D
  char szGamePass[0x18];       // 0x245
  char szGameDesc[0x104];      // 0x25D
  char channelname[0x20];      //+0x35f
  BYTE _14[0x40];              //+0x37f
  BYTE charlevel;              //+0x3bf
  BYTE ladderflag;             //+0x3c0
  DWORD passhash;              //+0x3c1
  BYTE passlength;             //+0x3c5
};

#pragma pack(pop)

struct WardenClientRegion_t {
  DWORD cbAllocSize;              //+00
  DWORD offsetFunc1;              //+04
  DWORD offsetRelocAddressTable;  //+08
  DWORD nRelocCount;              //+0c
  DWORD offsetWardenSetup;        //+10
  DWORD _2[2];
  DWORD offsetImportAddressTable;  //+1c
  DWORD nImportDllCount;           //+20
  DWORD nSectionCount;             //+24
};

struct SMemBlock_t {
  DWORD _1[6];
  DWORD cbSize;  //+18
  DWORD _2[31];
  BYTE data[1];  //+98
};

struct WardenClient_t {
  WardenClientRegion_t* pWardenRegion;  //+00
  DWORD cbSize;                         //+04
  DWORD nModuleCount;                   //+08
  DWORD param;                          //+0c
  DWORD fnSetupWarden;                  //+10
};

struct WardenIATInfo_t {
  DWORD offsetModuleName;
  DWORD offsetImportTable;
};

#pragma pack(push)
#pragma pack(1)

struct NPCMenu {
  DWORD dwNPCClassId;
  DWORD dwEntryAmount;
  WORD wEntryId1;
  WORD wEntryId2;
  WORD wEntryId3;
  WORD wEntryId4;
  WORD _1;
  DWORD dwEntryFunc1;
  DWORD dwEntryFunc2;
  DWORD dwEntryFunc3;
  DWORD dwEntryFunc4;
  BYTE _2[5];
};

struct OverheadMsg {
  DWORD _1;
  DWORD dwTrigger;
  DWORD _2[2];
  char Msg[232];
};

#pragma pack(pop)

struct D2MSG {
  HWND myHWND;
  char lpBuf[256];
};

struct MessageHandlerList {
  DWORD message;
  DWORD unk_4;
  DWORD(__stdcall* handler)(void*);
  struct MessageHandlerList* next;
};

struct MessageHandlerHashTable {
  struct MessageHandlerList** table;
  DWORD length;
};

struct WindowHandlerHashTable {
  struct WindowHandlerList** table;
  DWORD length;
};

struct WindowHandlerList {
  DWORD unk_0;
  HWND hWnd;
  DWORD unk_8;
  struct MessageHandlerHashTable* msgHandlers;
  struct WindowHandlerList* next;
};

// Not sure of the location of handler and this struct inside Info.
// Could be this struct is later and handler is earlier, but this is the safest
// for now.
struct TransactionDialogsLine_t {
  wchar_t text[120];           // 0x000
  DWORD unk[6];                // 0x0F0
  void(__stdcall* handler)();  // 0x108
  DWORD bMaybeSelectable;      // 0x10C
};

struct TransactionDialogsInfo_t {
  DWORD unk[0x14];                           // 0x000
  DWORD numLines;                            // 0x050
  DWORD unk_2[0x5];                          // 0x054
  TransactionDialogsLine_t dialogLines[10];  // 0x068
  void* something;                           // 0xB08
};

#pragma warning(pop)
#pragma optimize("", on)