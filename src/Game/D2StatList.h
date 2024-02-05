#pragma once

struct D2UnitStrc;

// split into D2StatStrc and D2SLayerStatIdStrc
struct D2StatStrc {
  WORD wSubIndex;     // 0x00
  WORD wStatIndex;    // 0x02
  DWORD dwStatValue;  // 0x04
};

struct D2StatsArrayStrc {
  D2StatStrc* pStats;
  WORD wCount;
  WORD wSize;
};

// split into D2StatListExStrc and D2StatListStrc
// Credits to SVR, http://phrozenkeep.hugelaser.com/forum/viewtopic.php?f=8&t=31458&p=224066
struct D2StatListExStrc {
  DWORD _1;                     // 0x00
  D2UnitStrc* pUnit;               // 0x04
  DWORD dwUnitType;             // 0x08
  DWORD dwUnitId;               // 0x0C
  DWORD dwFlags;                // 0x10
  DWORD _2[4];                  // 0x14
  D2StatsArrayStrc StatVec;     // 0x24
  D2StatListExStrc* pPrevLink;  // 0x2C
  DWORD _3;                     // 0x30
  D2StatListExStrc* pPrev;      // 0x34
  DWORD _4;                     // 0x38
  D2StatListExStrc* pNext;      // 0x3C
  D2StatListExStrc* pSetList;   // 0x40
  DWORD _5;                     // 0x44
  D2StatsArrayStrc SetStatVec;  // 0x48
  DWORD _6[2];                  // 0x50
  DWORD StateBits[6];           // 0x58
};
