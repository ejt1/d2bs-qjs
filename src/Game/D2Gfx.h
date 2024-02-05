#pragma once

// rename D2GfxCellStrc
struct GfxCell {
  DWORD flags;     // 0x00
  DWORD width;     // 0x04
  DWORD height;    // 0x08
  DWORD xoffs;     // 0x0C
  DWORD yoffs;     // 0x10
  DWORD _2;        // 0x14
  DWORD lpParent;  // 0x18
  DWORD length;    // 0x1C
  BYTE cols;       // 0x20
};

// rename D2CellFileStrc
struct CellFile {
  DWORD dwVersion;  // 0x00
  struct {
    WORD dwFlags;
    BYTE mylastcol;
    BYTE mytabno : 1;
  } UFlags;                    // 0x04
  DWORD eFormat;        // 0x08
  DWORD termination;    // 0x0C
  DWORD numdirs;        // 0x10
  DWORD numcells;       // 0x14
  GfxCell* cells[255];  // 0x18
};

// maybe D2GfxDataStrc
struct CellContext {
  DWORD _1[13];         // 0x00
  CellFile* pCellFile;  // 0x34
  DWORD _2[4];          // 0x38
};

// rename D2GfxLightStrc
struct Light {
  DWORD _1[3];          // 0x00
  DWORD dwType;         // 0x0C
  DWORD _2[7];          // 0x10
  DWORD dwStaticValid;  // 0x2C
  int* pnStaticMap;     // 0x30
};