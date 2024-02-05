#include "Room.h"
#include "CriticalSections.h"
#include "D2Helpers.h"
#include "Game/D2DataTbls.h"
#include "Game/Units/Units.h"
#include "Game/Unorganized.h"

bool RevealRoom(D2DrlgRoomStrc* pRoom2, bool revealPresets) {
  bool bAdded = false;
  bool bInit = false;

  DWORD dwLevelNo = D2CLIENT_GetPlayerUnit()->pPath->pRoom1->pRoom2->pLevel->dwLevelNo;
  // Make sure we have the room.
  if (!pRoom2)
    return false;

  AutoCriticalRoom cRoom;

  D2UnitStrc* player = D2CLIENT_GetPlayerUnit();
  // Check if we have D2ActiveRoomStrc(Needed in order to reveal)
  if (!(pRoom2 && pRoom2->pLevel && pRoom2->pRoom1)) {
    D2COMMON_AddRoomData(pRoom2->pLevel->pMisc->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, NULL);
    bAdded = true;
  }
  if (!(pRoom2 && pRoom2->pRoom1)) {  // second check added to see if we DID indeed init the room!
    return false;
  }

  // If we are somewhere other then the given area, init automap layer to be drawn to.
  if (!(pRoom2 && pRoom2->pLevel && pRoom2->pLevel->dwLevelNo && player->pPath && player->pPath->pRoom1 && player->pPath->pRoom1->pRoom2 &&
        player->pPath->pRoom1->pRoom2->pLevel && player->pPath->pRoom1->pRoom2->pLevel->dwLevelNo == pRoom2->pLevel->dwLevelNo)) {
    InitAutomapLayer(pRoom2->pLevel->dwLevelNo);
    bInit = true;
  }

  // Reveal this room!
  D2CLIENT_RevealAutomapRoom(pRoom2->pRoom1, TRUE, (*D2CLIENT_AutomapLayer));

  if (revealPresets)
    DrawPresets(pRoom2);

  // Remove room data if we have added.
  if (bAdded)
    D2COMMON_RemoveRoomData(pRoom2->pLevel->pMisc->pAct, pRoom2->pLevel->dwLevelNo, pRoom2->dwPosX, pRoom2->dwPosY, NULL);

  if (bInit)
    InitAutomapLayer(dwLevelNo);

  return true;
}

/*void DrawPresets(D2DrlgRoomStrc *pRoom2)
 *	This will find all the shrines, special automap icons, and level names and place on map.
 */
void DrawPresets(D2DrlgRoomStrc* pRoom2) {
  // D2UnitStrc* Player = D2CLIENT_GetPlayerUnit();
  // Grabs all the preset units in room.
  for (D2PresetUnitStrc* pUnit = pRoom2->pPreset; pUnit; pUnit = pUnit->pPresetNext) {
    int mCell = -1;
    if (pUnit->dwType == 1)  // Special NPCs.
    {
      if (pUnit->dwTxtFileNo == 256)  // Izzy
        mCell = 300;
      if (pUnit->dwTxtFileNo == 745)  // Hephasto
        mCell = 745;
    } else if (pUnit->dwType == 2) {  // Objects on Map

      // Add's a special Chest icon over the hidden uberchests in Lower Kurast
      if (pUnit->dwTxtFileNo == 580 && pRoom2->pLevel->dwLevelNo == 79)
        mCell = 318;

      // Special Units that require special checking:)
      if (pUnit->dwTxtFileNo == 371)
        mCell = 301;  // Countess Chest
      if (pUnit->dwTxtFileNo == 152)
        mCell = 300;  // A2 Orifice
      if (pUnit->dwTxtFileNo == 460)
        mCell = 1468;  // Frozen Anya
      if ((pUnit->dwTxtFileNo == 402) && (pRoom2->pLevel->dwLevelNo == 46))
        mCell = 0;  // Canyon/Arcane Waypoint
      if ((pUnit->dwTxtFileNo == 267) && (pRoom2->pLevel->dwLevelNo != 75) && (pRoom2->pLevel->dwLevelNo != 103))
        mCell = 0;
      if ((pUnit->dwTxtFileNo == 376) && (pRoom2->pLevel->dwLevelNo == 107))
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
      int pX = (pUnit->dwPosX + (pRoom2->dwPosX * 5));
      int pY = (pUnit->dwPosY + (pRoom2->dwPosY * 5));
      pCell->xPixel = (WORD)((((pX - pY) * 16) / 10) + 1);
      pCell->yPixel = (WORD)((((pY + pX) * 8) / 10) - 3);

      D2CLIENT_AddAutomapCell(pCell, &((*D2CLIENT_AutomapLayer)->pObjects));
    }
  }
}
