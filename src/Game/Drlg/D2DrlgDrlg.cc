#include "D2DrlgDrlg.h"

#include "CriticalSections.h" // AutoCriticalRoom
#include "Game/D2Automap.h"
#include "Game/D2Unit.h"

void D2DrlgRoomStrc::AddRoomData() {
  D2COMMON_AddRoomData(pLevel->pMisc->pAct, pLevel->dwLevelNo, dwPosX, dwPosY, pRoom1);
}

void D2DrlgRoomStrc::RemoveRoomData() {
  D2COMMON_RemoveRoomData(pLevel->pMisc->pAct, pLevel->dwLevelNo, dwPosX, dwPosY, pRoom1);
}

uint32_t D2DrlgRoomStrc::GetTileLevelNo(uint32_t dwTileNo) {
  for (D2RoomTileStrc* pRoomTile = pRoomTiles; pRoomTile; pRoomTile = pRoomTile->pNext) {
    if (*(pRoomTile->nNum) == dwTileNo)
      return pRoomTile->pRoom2->pLevel->dwLevelNo;
  }

  return NULL;
}

bool D2DrlgRoomStrc::Reveal(bool revealPresets) {
  bool bAdded = false;
  bool bInit = false;

  DWORD dwLevelNo = D2CLIENT_GetPlayerUnit()->pPath->pRoom1->pRoom2->pLevel->dwLevelNo;

  AutoCriticalRoom cRoom;

  D2UnitStrc* player = D2CLIENT_GetPlayerUnit();
  // Check if we have D2ActiveRoomStrc(Needed in order to reveal)
  if (!(pLevel && pRoom1)) {
    D2COMMON_AddRoomData(pLevel->pMisc->pAct, pLevel->dwLevelNo, dwPosX, dwPosY, NULL);
    bAdded = true;
  }
  if (!(pRoom1)) {  // second check added to see if we DID indeed init the room!
    return false;
  }

  // If we are somewhere other then the given area, init automap layer to be drawn to.
  if (!(pLevel && pLevel->dwLevelNo && player->pPath && player->pPath->pRoom1 && player->pPath->pRoom1->pRoom2 &&
        player->pPath->pRoom1->pRoom2->pLevel && player->pPath->pRoom1->pRoom2->pLevel->dwLevelNo == pLevel->dwLevelNo)) {
    InitAutomapLayer(pLevel->dwLevelNo);
    bInit = true;
  }

  // Reveal this room!
  D2CLIENT_RevealAutomapRoom(pRoom1, TRUE, (*D2CLIENT_AutomapLayer));

  if (revealPresets)
    DrawPresets();

  // Remove room data if we have added.
  if (bAdded)
    D2COMMON_RemoveRoomData(pLevel->pMisc->pAct, pLevel->dwLevelNo, dwPosX, dwPosY, NULL);

  if (bInit)
    InitAutomapLayer(dwLevelNo);

  return true;
}

void D2DrlgRoomStrc::DrawPresets() {
  // D2UnitStrc* Player = D2CLIENT_GetPlayerUnit();
  // Grabs all the preset units in room.
  for (D2PresetUnitStrc* pUnit = pPreset; pUnit; pUnit = pUnit->pPresetNext) {
    int mCell = -1;
    if (pUnit->dwType == 1)  // Special NPCs.
    {
      if (pUnit->dwTxtFileNo == 256)  // Izzy
        mCell = 300;
      if (pUnit->dwTxtFileNo == 745)  // Hephasto
        mCell = 745;
    } else if (pUnit->dwType == 2) {  // Objects on Map

      // Add's a special Chest icon over the hidden uberchests in Lower Kurast
      if (pUnit->dwTxtFileNo == 580 && pLevel->dwLevelNo == 79)
        mCell = 318;

      // Special Units that require special checking:)
      if (pUnit->dwTxtFileNo == 371)
        mCell = 301;  // Countess Chest
      if (pUnit->dwTxtFileNo == 152)
        mCell = 300;  // A2 Orifice
      if (pUnit->dwTxtFileNo == 460)
        mCell = 1468;  // Frozen Anya
      if ((pUnit->dwTxtFileNo == 402) && (pLevel->dwLevelNo == 46))
        mCell = 0;  // Canyon/Arcane Waypoint
      if ((pUnit->dwTxtFileNo == 267) && (pLevel->dwLevelNo != 75) && (pLevel->dwLevelNo != 103))
        mCell = 0;
      if ((pUnit->dwTxtFileNo == 376) && (pLevel->dwLevelNo == 107))
        mCell = 376;

      if (pUnit->dwTxtFileNo > 574)
        mCell = 0;

      if (mCell == -1) {
        // Get the object cell
        D2ObjectsTxt* obj = D2COMMON_GetObjectText(pUnit->dwTxtFileNo);

        if (mCell == -1) {
          mCell = obj->nAutoMap;  // Set the cell number then.
        }
      }
    }

    // Draw the cell if wanted.
    if ((mCell > 0) && (mCell < 1258)) {
      D2AutomapCellStrc* pCell = D2CLIENT_NewAutomapCell();
      pCell->nCellNo = (WORD)mCell;
      int pX = (pUnit->dwPosX + (dwPosX * 5));
      int pY = (pUnit->dwPosY + (dwPosY * 5));
      pCell->xPixel = (WORD)((((pX - pY) * 16) / 10) + 1);
      pCell->yPixel = (WORD)((((pY + pX) * 8) / 10) - 3);

      D2CLIENT_AddAutomapCell(pCell, &((*D2CLIENT_AutomapLayer)->pObjects));
    }
  }
}

const char* D2DrlgLevelStrc::GetName() const {
  return D2COMMON_GetLevelText(dwLevelNo)->szName;
}

D2DrlgLevelStrc* D2DrlgLevelStrc::FindLevelFromLevelId(uint32_t levelId) {
  // TODO(ejt): remove dependency on AutoCriticalRoom
  AutoCriticalRoom cRoom;

  //if (!GameReady())
  //  return nullptr;
  D2UnitStrc* player = D2CLIENT_GetPlayerUnit();
  if (!player) {
    return nullptr;
  }

  D2DrlgLevelStrc* pLevel = player->pAct->pMisc->pLevelFirst;
  while (pLevel && pLevel->dwLevelNo != levelId) {
    pLevel = pLevel->pNextLevel;
  }

  if (pLevel && !pLevel->pRoom2First) {
    D2COMMON_InitLevel(pLevel);
  }

  // this crashes pretty much every time it's called
  // pLevel = D2COMMON_GetLevel(D2CLIENT_GetPlayerUnit()->pAct->pMisc, dwLevelNo);
  return pLevel;
}
