#include "D2Gfx.h"

// TODO(ejt): remove dependency on these includes
#include "Misc/Localization.h"
#include "Misc/stringhash.h"
#include "Globals.h"

#include "Game/D2Unit.h"        // D2CLIENT_GetPlayerUnit for hack
#include "Game/D2WinControl.h"  // D2WIN_FirstControl for hack

POINT GetScreenSize() {
  // HACK: p_D2CLIENT_ScreenSize is wrong for out of game, which is hardcoded to 800x600
  D2UnitStrc* pPlayer = D2CLIENT_GetPlayerUnit();
  D2WinControlStrc* pControl = *D2WIN_FirstControl;
  if (pPlayer && !pControl) {
    return {static_cast<LONG>(*D2CLIENT_ScreenSizeX), static_cast<LONG>(*D2CLIENT_ScreenSizeY)};
  } else if (!pPlayer && pControl) {
    return {800, 600};
  }
  return {0, 0};
}

int D2GetScreenSizeX() {
  return GetScreenSize().x;
}

int D2GetScreenSizeY() {
  return GetScreenSize().y;
}

static HANDLE OpenFileRead(const char* filename) {
  return CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}

static HANDLE OpenFileRead(const wchar_t* filename) {
  return CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}

static DWORD ReadFile(HANDLE hFile, void* buf, DWORD len)
// NOTE :- validates len bytes of buf
{
  DWORD numdone = 0;
  return ::ReadFile(hFile, buf, len, &numdone, NULL) != 0 ? numdone : -1;
}

static BYTE* AllocReadFile(const char* filename) {
  HANDLE hFile = OpenFileRead(filename);
  int filesize = GetFileSize(hFile, 0);
  if (filesize <= 0)
    return 0;
  BYTE* buf = new BYTE[filesize];
  ReadFile(hFile, buf, filesize);
  CloseHandle(hFile);
  return buf;
}

static BYTE* AllocReadFile(const wchar_t* filename) {
  HANDLE hFile = OpenFileRead(filename);
  int filesize = GetFileSize(hFile, 0);
  if (filesize <= 0)
    return 0;
  BYTE* buf = new BYTE[filesize];
  ReadFile(hFile, buf, filesize);
  CloseHandle(hFile);
  return buf;
}

// NOTE: only used by LoadBmpCellFile, lets keep it that way!
static void* memcpy2(void* dest, const void* src, size_t count) {
  return (char*)memcpy(dest, src, count) + count;
}

// TODO: Rewrite this and split it into two functions
D2CellFileStrc* LoadCellFile(const char* lpszPath, uint32_t bMPQ) {
  if (bMPQ == TRUE) {
    unsigned __int32 hash = sfh((char*)lpszPath, (int)strlen((char*)lpszPath));
    if (Vars.mCachedCellFiles.count(hash) > 0)
      return Vars.mCachedCellFiles[hash];
    D2CellFileStrc* result = (D2CellFileStrc*)D2WIN_LoadCellFile((char*)lpszPath, 0);
    Vars.mCachedCellFiles[hash] = result;
    return result;
  } else {
    std::string ansi = UTF8ToANSI(lpszPath);
    std::wstring path = AnsiToWide(ansi);
    D2CellFileStrc* ret = LoadCellFile(path.c_str(), bMPQ);
    return ret;
  }
}

D2CellFileStrc* LoadCellFile(const wchar_t* lpszPath, uint32_t bMPQ) {
  if (bMPQ != 0) {
    return NULL;
  }

  // AutoDetect the Cell File
  if (bMPQ == 3) {
    // Check in our directory first
    wchar_t path[_MAX_FNAME + _MAX_PATH];
    swprintf_s(path, _countof(path), L"%S\\%s", Vars.szScriptPath, lpszPath);

    HANDLE hFile = OpenFileRead(path);

    if (hFile != INVALID_HANDLE_VALUE) {
      CloseHandle(hFile);
      return LoadCellFile(path, FALSE);
    } else {
      return LoadCellFile(lpszPath, TRUE);
    }
  }

  unsigned __int32 hash = sfh((char*)lpszPath, (int)strlen((char*)lpszPath));
  if (Vars.mCachedCellFiles.count(hash) > 0)
    return Vars.mCachedCellFiles[hash];

  // see if the file exists first
  if (!(_waccess(lpszPath, 0) != 0 && errno == ENOENT)) {
    D2CellFileStrc* result = myInitCellFile((D2CellFileStrc*)LoadBmpCellFile(lpszPath));
    Vars.mCachedCellFiles[hash] = result;
    return result;
  }

  return NULL;
}

D2CellFileStrc* LoadBmpCellFile(uint8_t* buf1, int width, int height) {
  BYTE *buf2 = new BYTE[(width * height * 2) + height], *dest = buf2;

  for (int i = 0; i < height; i++) {
    BYTE *src = buf1 + (i * ((width + 3) & -4)), *limit = src + width;
    while (src < limit) {
      BYTE *start = src, *limit2 = std::min(limit, src + 0x7f), trans = !*src;
      do src++;
      while ((trans == (BYTE) !*src) && (src < limit2));
      if (!trans || (src < limit))
        *dest++ = (BYTE)((trans ? 0x80 : 0) + (src - start));
      if (!trans)
        while (start < src) *dest++ = *start++;
    }
    *dest++ = 0x80;
  }

  static DWORD dc6head[] = {6, 1, 0, 0xeeeeeeee, 1, 1, 0x1c, 0, (DWORD)-1, (DWORD)-1, 0, 0, 0, (DWORD)-1, (DWORD)-1};
  dc6head[8] = width;
  dc6head[9] = height;
  dc6head[14] = dest - buf2;
  dc6head[13] = sizeof(dc6head) + (dc6head[14]) + 3;
  BYTE* ret = new BYTE[dc6head[13]];
  memset(memcpy2(memcpy2(ret, dc6head, sizeof(dc6head)), buf2, dc6head[14]), 0xee, 3);
  delete[] buf2;

  return (D2CellFileStrc*)ret;
}

D2CellFileStrc* LoadBmpCellFile(const char* filename) {
  BYTE* ret = 0;

  BYTE* buf1 = AllocReadFile(filename);
  BITMAPFILEHEADER* bmphead1 = (BITMAPFILEHEADER*)buf1;
  BITMAPINFOHEADER* bmphead2 = (BITMAPINFOHEADER*)(buf1 + sizeof(BITMAPFILEHEADER));
  if (buf1 && (bmphead1->bfType == 'MB') && (bmphead2->biBitCount == 8) && (bmphead2->biCompression == BI_RGB)) {
    ret = (BYTE*)LoadBmpCellFile(buf1 + bmphead1->bfOffBits, bmphead2->biWidth, bmphead2->biHeight);
  }
  delete[] buf1;

  return (D2CellFileStrc*)ret;
}

D2CellFileStrc* LoadBmpCellFile(const wchar_t* filename) {
  BYTE* ret = 0;

  BYTE* buf1 = AllocReadFile(filename);
  BITMAPFILEHEADER* bmphead1 = (BITMAPFILEHEADER*)buf1;
  BITMAPINFOHEADER* bmphead2 = (BITMAPINFOHEADER*)(buf1 + sizeof(BITMAPFILEHEADER));
  if (buf1 && (bmphead1->bfType == 'MB') && (bmphead2->biBitCount == 8) && (bmphead2->biCompression == BI_RGB)) {
    ret = (BYTE*)LoadBmpCellFile(buf1 + bmphead1->bfOffBits, bmphead2->biWidth, bmphead2->biHeight);
  }
  delete[] buf1;

  return (D2CellFileStrc*)ret;
}

D2CellFileStrc* myInitCellFile(D2CellFileStrc* cf) {
  if (cf)
    D2CMP_InitCellFile(cf, &cf, "?", 0, (DWORD)-1, "?");
  return cf;
}

// TODO: make this use SIZE for clarity
POINT CalculateTextLen(const char* szwText, int Font) {
  POINT ret = {0, 0};

  if (!szwText)
    return ret;

  std::string ansi(UTF8ToANSI(szwText));
  std::wstring buf = AnsiToWide(ansi);
  ret = CalculateTextLen(buf.c_str(), Font);
  return ret;
}

POINT CalculateTextLen(const wchar_t* szwText, int Font) {
  POINT ret = {0, 0};

  if (!szwText)
    return ret;

  DWORD dwWidth, dwFileNo;
  DWORD dwOldSize = D2WIN_SetTextSize(Font);
  ret.y = D2WIN_GetTextSize((wchar_t*)szwText, &dwWidth, &dwFileNo);
  ret.x = dwWidth;
  D2WIN_SetTextSize(dwOldSize);

  return ret;
}

void myDrawText(const char* szwText, int x, int y, int color, int font) {
  // TODO(ejt): this is kinda fucked. we expect to supply an UTF8 encoded narrow string
  // D2 expects a ANSI encoded wide string.
  std::string ansi(UTF8ToANSI(szwText));
  std::wstring str = AnsiToWide(ansi);
  myDrawText(str.c_str(), x, y, color, font);
}

void myDrawText(const wchar_t* szwText, int x, int y, int color, int font) {
  DWORD dwOld = D2WIN_SetTextSize(font);
  D2WIN_DrawText(szwText, x, y, color, 0);
  D2WIN_SetTextSize(dwOld);
}

void myDrawCenterText(const char* szText, int x, int y, int color, int font, int div) {
  std::string ansi(UTF8ToANSI(szText));
  std::wstring str = AnsiToWide(ansi);
  myDrawCenterText(str.c_str(), x, y, color, font, div);
}

void myDrawCenterText(const wchar_t* szText, int x, int y, int color, int font, int div) {
  DWORD dwWidth = NULL, dwFileNo = NULL, dwOldSize = NULL;

  dwOldSize = D2WIN_SetTextSize(font);
  D2WIN_GetTextSize(szText, &dwWidth, &dwFileNo);
  D2WIN_SetTextSize(dwOldSize);
  myDrawText(szText, x - (dwWidth >> div), y, color, font);
}

void WorldToScreen(POINT* pPos) {
  D2COMMON_MapToAbsScreen(&pPos->x, &pPos->y);
  pPos->x -= D2CLIENT_GetMouseXOffset();
  pPos->y -= D2CLIENT_GetMouseYOffset();
}

void ScreenToWorld(POINT* pPos) {
  D2COMMON_AbsScreenToMap(&pPos->x, &pPos->y);
  pPos->x += D2CLIENT_GetMouseXOffset();
  pPos->y += D2CLIENT_GetMouseYOffset();
}
