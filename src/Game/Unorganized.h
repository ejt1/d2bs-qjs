#pragma once

// file contains structs that needs organizing
// DO NOT ADD NEW STRUCTS HERE!

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