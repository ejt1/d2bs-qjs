#pragma once

#include <cstdint>
#include <Windows.h> // POINT, HWND

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

POINT GetScreenSize();
int D2GetScreenSizeX();
int D2GetScreenSizeY();

D2CellFileStrc* LoadCellFile(const char* lpszPath, uint32_t bMPQ = TRUE);
D2CellFileStrc* LoadCellFile(const wchar_t* lpszPath, uint32_t bMPQ = 3);
D2CellFileStrc* LoadBmpCellFile(uint8_t* buf1, int width, int height);
D2CellFileStrc* LoadBmpCellFile(const char* filename);
D2CellFileStrc* LoadBmpCellFile(const wchar_t* filename);
D2CellFileStrc* myInitCellFile(D2CellFileStrc* cf);

POINT CalculateTextLen(const char* szwText, int Font);
POINT CalculateTextLen(const wchar_t* szwText, int Font);
void myDrawText(const char* szwText, int x, int y, int color, int font);
void myDrawText(const wchar_t* szwText, int x, int y, int color, int font);
void myDrawCenterText(const char* szwText, int x, int y, int color, int font, int div);
void myDrawCenterText(const wchar_t* szwText, int x, int y, int color, int font, int div);

void WorldToScreen(POINT* pPos);
void ScreenToWorld(POINT* ptPos);

inline void(__stdcall* D2CMP_InitCellFile)(void* File, D2CellFileStrc** Out, const char* SourceFile, uint32_t Line, uint32_t FileVersion, const char* Filename) = nullptr;
inline void(__stdcall* D2CMP_DeleteCellFile)(D2CellFileStrc* File) = nullptr;
inline D2CellFileStrc*(__fastcall* D2WIN_LoadCellFile)(const char* szFile, int Type) = nullptr;

inline void(__stdcall* D2GFX_DrawRectangle)(int X1, int Y1, int X2, int Y2, uint32_t dwColor, uint32_t dwTrans) = nullptr;
inline void(__stdcall* D2GFX_DrawLine)(int X1, int Y1, int X2, int Y2, uint32_t dwColor, uint32_t dwUnk) = nullptr;
inline void(__stdcall* D2GFX_DrawAutomapCell2)(D2GfxDataStrc* context, uint32_t xpos, uint32_t ypos, uint32_t bright2, uint32_t bright, uint8_t* coltab) = nullptr;
inline HWND(__stdcall* D2GFX_GetHwnd)(void) = nullptr;

inline void(__fastcall* D2WIN_DrawText)(const wchar_t* wStr, int xPos, int yPos, DWORD dwColor, DWORD dwUnk) = nullptr;
inline DWORD(__fastcall* D2WIN_GetTextSize)(const wchar_t* wStr, DWORD* dwWidth, DWORD* dwFileNo) = nullptr;
inline DWORD(__fastcall* D2WIN_SetTextSize)(DWORD dwSize) = nullptr;
inline DWORD(__fastcall* D2WIN_GetTextWidthFileNo)(wchar_t* wStr, DWORD* dwWidth, DWORD* dwFileNo) = nullptr;

inline uint32_t* D2CLIENT_ScreenSizeX = nullptr;
inline uint32_t* D2CLIENT_ScreenSizeY = nullptr;
inline uint32_t(__stdcall* D2GFX_GetScreenSize)() = nullptr;

inline uint32_t(__fastcall* D2CLIENT_GetMouseXOffset)(void) = nullptr;
inline uint32_t(__fastcall* D2CLIENT_GetMouseYOffset)(void) = nullptr;
inline void(__stdcall* D2COMMON_MapToAbsScreen)(long* pX, long* pY) = nullptr;
inline void(__stdcall* D2COMMON_AbsScreenToMap)(long* ptMouseX, long* ptMouseY) = nullptr;