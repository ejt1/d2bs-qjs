#pragma once

#include <Windows.h>

struct Inventory;
struct UnitAny;

// rename D2ItemDataStrc
struct ItemData {
  DWORD dwQuality;             // 0x00
  DWORD dwSeed[2];             // 0x04
  DWORD dwItemFlags;           // 0x0C 1 = Owned by player, 0xFFFFFFFF = Not owned
  DWORD dwFingerPrint;         // 0x10 Initial seed
  DWORD _1;                    // 0x14 CommandFlags?
  DWORD dwFlags;               // 0x18
  DWORD _2[2];                 // 0x1C
  DWORD dwActionStamp;         // 0x24 Changes when an item is changed
  DWORD dwFileIndex;           // 0x28 Data file index UniqueItems.txt etc.
  DWORD dwItemLevel;           // 0x2C
  WORD wItemFormat;            // 0x30
  WORD wRarePrefix;            // 0x32
  WORD wRareSuffix;            // 0x34
  WORD wAutoPrefix;            // 0x36
  WORD wMagicPrefix[3];        // 0x38
  WORD wMagicSuffix[3];        // 0x3E
  BYTE BodyLocation;           // 0x44 Not always cleared
  BYTE ItemLocation;           // 0x45 Non-body/belt location (Body/Belt == 0xFF)
  WORD _4;                     // 0x46
  BYTE bEarLevel;              // 0x48
  BYTE bInvGfxIdx;             // 0x49
  char szPlayerName[16];       // 0x4A Personalized / Ear name
  Inventory* pOwnerInventory;  // 0x5C Socketed Items owner Inv
  DWORD _10;                   // 0x60
  UnitAny* pNextInvItem;       // 0x64 Next item in socketed item if OwnerInventory is set
  BYTE GameLocation;           // 0x68 Location per docs.d2bs.org (unit.location)
  BYTE NodePage;               // 0x69 Actual location, this is the most reliable by far
  WORD _12;                    // 0x6A
  WORD _13[12];                // 0x6C
  UnitAny* pOwner;             // 0x84
};