#pragma once

#include <cstdint>

struct D2ActiveRoomStrc;

struct D2DynamicPathStrc {
  uint16_t xOffset;            // 0x00
  uint16_t xPos;               // 0x02
  uint16_t yOffset;            // 0x04
  uint16_t yPos;               // 0x06
  uint32_t _1[2];              // 0x08
  uint16_t xTarget;            // 0x10
  uint16_t yTarget;            // 0x12
  uint32_t _2[2];              // 0x14
  D2ActiveRoomStrc* pRoom1;    // 0x1C
  D2ActiveRoomStrc* pRoomUnk;  // 0x20
  uint32_t _3[3];              // 0x24
  D2UnitStrc* pUnit;           // 0x30
  uint32_t dwFlags;            // 0x34
  uint32_t _4;                 // 0x38
  uint32_t dwPathType;         // 0x3C
  uint32_t dwPrevPathType;     // 0x40
  uint32_t dwUnitSize;         // 0x44
  uint32_t _5[4];              // 0x48
  D2UnitStrc* pTargetUnit;     // 0x58
  uint32_t dwTargetType;       // 0x5C
  uint32_t dwTargetId;         // 0x60
  BYTE bDirection;             // 0x64
};

struct D2StaticPathStrc {
  D2ActiveRoomStrc* pRoom1;  // 0x00
  uint32_t _1[2];            // 0x04
  uint32_t dwPosX;           // 0x0C
  uint32_t dwPosY;           // 0x10
                             // Use Path for the rest
};