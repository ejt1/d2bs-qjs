#include "Game/D2Automap.h"

D2AutomapLayerStrc* InitAutomapLayer(DWORD levelno) {
  D2LevelDefBin* pLayer = D2COMMON_GetLayer(levelno);
  return D2CLIENT_InitAutomapLayer(pLayer->nLayerNo);
}

DWORD __declspec(naked) __fastcall D2CLIENT_InitAutomapLayer_STUB(DWORD /*nLayerNo*/) {
  __asm
  {
		push edi;  // Updated 1.14d Registers changed.
		mov edi, ecx;
		call D2CLIENT_InitAutomapLayer_I;
		pop edi;
		ret;
  }
}

void myDrawAutomapCell(D2CellFileStrc* cellfile, int xpos, int ypos, BYTE col) {
  if (!cellfile)
    return;
  D2GfxDataStrc ct;
  memset(&ct, 0, sizeof(ct));
  ct.pCellFile = cellfile;

  xpos -= (cellfile->cells[0]->width / 2);
  ypos += (cellfile->cells[0]->height / 2);

  int xpos2 = xpos - cellfile->cells[0]->xoffs, ypos2 = ypos - cellfile->cells[0]->yoffs;
  if ((xpos2 >= D2GetScreenSizeX()) || ((xpos2 + (int)cellfile->cells[0]->width) <= 0) || (ypos2 >= D2GetScreenSizeY()) ||
      ((ypos2 + (int)cellfile->cells[0]->height) <= 0))
    return;

  static BYTE coltab[2][256];  //, tabno = 0, lastcol = 0;
  if (!coltab[0][1])
    for (int k = 0; k < 255; k++) coltab[0][k] = coltab[1][k] = (BYTE)k;
  cellfile->UFlags.mylastcol = coltab[cellfile->UFlags.mytabno ^= (col != cellfile->UFlags.mylastcol)][255] = col;

  D2GFX_DrawAutomapCell2(&ct, xpos, ypos, (DWORD)-1, 5, coltab[cellfile->UFlags.mytabno]);
}

POINT ScreenToAutomap(int x, int y) {
  POINT result = {0, 0};
  x *= 32;
  y *= 32;
  result.x = ((x - y) / 2 / (*D2CLIENT_Divisor)) - (*D2CLIENT_Offset).x + 8;
  result.y = ((x + y) / 4 / (*D2CLIENT_Divisor)) - (*D2CLIENT_Offset).y - 8;

  if (D2CLIENT_GetAutomapSize()) {
    --result.x;
    result.y += 5;
  }
  return result;
}

void AutomapToScreen(POINT* pPos) {
  pPos->x = 8 - D2CLIENT_Offset->x + (pPos->x * (*D2CLIENT_AutomapMode));
  pPos->y = 8 + D2CLIENT_Offset->y + (pPos->y * (*D2CLIENT_AutomapMode));
}