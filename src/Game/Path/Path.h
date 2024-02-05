#pragma once

struct D2ActiveRoomStrc;

struct D2DynamicPathStrc {
  WORD xOffset;             // 0x00
  WORD xPos;                // 0x02
  WORD yOffset;             // 0x04
  WORD yPos;                // 0x06
  DWORD _1[2];              // 0x08
  WORD xTarget;             // 0x10
  WORD yTarget;             // 0x12
  DWORD _2[2];              // 0x14
  D2ActiveRoomStrc* pRoom1;            // 0x1C
  D2ActiveRoomStrc* pRoomUnk;          // 0x20
  DWORD _3[3];              // 0x24
  D2UnitStrc* pUnit;        // 0x30
  DWORD dwFlags;            // 0x34
  DWORD _4;                 // 0x38
  DWORD dwPathType;         // 0x3C
  DWORD dwPrevPathType;     // 0x40
  DWORD dwUnitSize;         // 0x44
  DWORD _5[4];              // 0x48
  D2UnitStrc* pTargetUnit;  // 0x58
  DWORD dwTargetType;       // 0x5C
  DWORD dwTargetId;         // 0x60
  BYTE bDirection;          // 0x64
};

struct D2StaticPathStrc {
  D2ActiveRoomStrc* pRoom1;  // 0x00
  DWORD _1[2];    // 0x04
  DWORD dwPosX;   // 0x0C
  DWORD dwPosY;   // 0x10
                  // Use Path for the rest
};