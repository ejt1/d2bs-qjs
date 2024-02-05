#pragma once

struct UnitAny;

// rename D2StatStrc
// split into D2StatStrc and D2SLayerStatIdStrc
struct Stat {
  WORD wSubIndex;     // 0x00
  WORD wStatIndex;    // 0x02
  DWORD dwStatValue;  // 0x04
};

// rename D2StatsArrayStrc
struct StatVector {
  Stat* pStats;
  WORD wCount;
  WORD wSize;
};

// rename D2StatListExStrc
// split into D2StatListExStrc and D2StatListStrc
// Credits to SVR, http://phrozenkeep.hugelaser.com/forum/viewtopic.php?f=8&t=31458&p=224066
struct StatList {
  DWORD _1;               // 0x00
  UnitAny* pUnit;         // 0x04
  DWORD dwUnitType;       // 0x08
  DWORD dwUnitId;         // 0x0C
  DWORD dwFlags;          // 0x10
  DWORD _2[4];            // 0x14
  StatVector StatVec;     // 0x24
  StatList* pPrevLink;    // 0x2C
  DWORD _3;               // 0x30
  StatList* pPrev;        // 0x34
  DWORD _4;               // 0x38
  StatList* pNext;        // 0x3C
  StatList* pSetList;     // 0x40
  DWORD _5;               // 0x44
  StatVector SetStatVec;  // 0x48
  DWORD _6[2];            // 0x50
  DWORD StateBits[6];     // 0x58
};