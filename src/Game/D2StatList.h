#pragma once

#include <cstdint>

struct D2UnitStrc;

// split into D2StatStrc and D2SLayerStatIdStrc
struct D2StatStrc {
  uint16_t wSubIndex;    // 0x00
  uint16_t wStatIndex;   // 0x02
  uint32_t dwStatValue;  // 0x04
};

struct D2StatsArrayStrc {
  D2StatStrc* pStats;
  uint16_t wCount;
  uint16_t wSize;
};

// split into D2StatListExStrc and D2StatListStrc
// Credits to SVR, http://phrozenkeep.hugelaser.com/forum/viewtopic.php?f=8&t=31458&p=224066
struct D2StatListExStrc {
  uint32_t _1;                  // 0x00
  D2UnitStrc* pUnit;            // 0x04
  uint32_t dwUnitType;          // 0x08
  uint32_t dwUnitId;            // 0x0C
  uint32_t dwFlags;             // 0x10
  uint32_t _2[4];               // 0x14
  D2StatsArrayStrc StatVec;     // 0x24
  D2StatListExStrc* pPrevLink;  // 0x2C
  uint32_t _3;                  // 0x30
  D2StatListExStrc* pPrev;      // 0x34
  uint32_t _4;                  // 0x38
  D2StatListExStrc* pNext;      // 0x3C
  D2StatListExStrc* pSetList;   // 0x40
  uint32_t _5;                  // 0x44
  D2StatsArrayStrc SetStatVec;  // 0x48
  uint32_t _6[2];               // 0x50
  uint32_t StateBits[6];        // 0x58
};
