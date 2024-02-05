#pragma once

#include <cstdint>

struct D2InventoryStrc;
struct D2UnitStrc;

struct D2ItemDataStrc {
  uint32_t dwQuality;                // 0x00
  uint32_t dwSeed[2];                // 0x04
  uint32_t dwItemFlags;              // 0x0C 1 = Owned by player, 0xFFFFFFFF = Not owned
  uint32_t dwFingerPrint;            // 0x10 Initial seed
  uint32_t _1;                       // 0x14 CommandFlags?
  uint32_t dwFlags;                  // 0x18
  uint32_t _2[2];                    // 0x1C
  uint32_t dwActionStamp;            // 0x24 Changes when an item is changed
  uint32_t dwFileIndex;              // 0x28 Data file index UniqueItems.txt etc.
  uint32_t dwItemLevel;              // 0x2C
  uint16_t wItemFormat;              // 0x30
  uint16_t wRarePrefix;              // 0x32
  uint16_t wRareSuffix;              // 0x34
  uint16_t wAutoPrefix;              // 0x36
  uint16_t wMagicPrefix[3];          // 0x38
  uint16_t wMagicSuffix[3];          // 0x3E
  uint8_t BodyLocation;              // 0x44 Not always cleared
  uint8_t ItemLocation;              // 0x45 Non-body/belt location (Body/Belt == 0xFF)
  uint16_t _4;                       // 0x46
  uint8_t bEarLevel;                 // 0x48
  uint8_t bInvGfxIdx;                // 0x49
  char szPlayerName[16];             // 0x4A Personalized / Ear name
  D2InventoryStrc* pOwnerInventory;  // 0x5C Socketed Items owner Inv
  uint32_t _10;                      // 0x60
  D2UnitStrc* pNextInvItem;          // 0x64 Next item in socketed item if OwnerInventory is set
  uint8_t GameLocation;              // 0x68 Location per docs.d2bs.org (unit.location)
  uint8_t NodePage;                  // 0x69 Actual location, this is the most reliable by far
  uint16_t _12;                      // 0x6A
  uint16_t _13[12];                  // 0x6C
  D2UnitStrc* pOwner;                // 0x84
};