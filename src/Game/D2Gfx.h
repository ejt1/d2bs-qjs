#pragma once

#include <cstdint>

struct D2GfxCellStrc {
  uint32_t flags;     // 0x00
  uint32_t width;     // 0x04
  uint32_t height;    // 0x08
  uint32_t xoffs;     // 0x0C
  uint32_t yoffs;     // 0x10
  uint32_t _2;        // 0x14
  uint32_t lpParent;  // 0x18
  uint32_t length;    // 0x1C
  uint8_t cols;       // 0x20
};

struct D2CellFileStrc {
  uint32_t dwVersion;  // 0x00
  struct {
    uint16_t dwFlags;
    uint8_t mylastcol;
    uint8_t mytabno : 1;
  } UFlags;                   // 0x04
  uint32_t eFormat;           // 0x08
  uint32_t termination;       // 0x0C
  uint32_t numdirs;           // 0x10
  uint32_t numcells;          // 0x14
  D2GfxCellStrc* cells[255];  // 0x18
};

struct D2GfxDataStrc {
  uint32_t _1[13];            // 0x00
  D2CellFileStrc* pCellFile;  // 0x34
  uint32_t _2[4];             // 0x38
};

struct D2GfxLightStrc {
  uint32_t _1[3];          // 0x00
  uint32_t dwType;         // 0x0C
  uint32_t _2[7];          // 0x10
  uint32_t dwStaticValid;  // 0x2C
  int* pnStaticMap;        // 0x30
};