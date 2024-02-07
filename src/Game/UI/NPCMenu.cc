#include "Game/UI/NPCMenu.h"

bool NPCMenu::Click(uint32_t npcClassId, uint32_t menuId) {
  typedef void (*fnClickEntry)(void);
  NPCMenu* pMenu = (NPCMenu*)D2CLIENT_NPCMenu;
  fnClickEntry pClick = (fnClickEntry)NULL;

  for (uint32_t i = 0; i < *D2CLIENT_NPCMenuAmount; i++) {
    if (pMenu->dwNPCClassId == npcClassId) {
      if (pMenu->wEntryId1 == menuId) {
        pClick = (fnClickEntry)pMenu->dwEntryFunc1;
        if (pClick)
          pClick();
        else
          return false;
        return true;
      } else if (pMenu->wEntryId2 == menuId) {
        pClick = (fnClickEntry)pMenu->dwEntryFunc2;
        if (pClick)
          pClick();
        else
          return false;
        return true;
      } else if (pMenu->wEntryId3 == menuId) {
        pClick = (fnClickEntry)pMenu->dwEntryFunc3;
        if (pClick)
          pClick();
        else
          return false;
        return true;
      } else if (pMenu->wEntryId4 == menuId) {
        pClick = (fnClickEntry)pMenu->dwEntryFunc4;
        if (pClick)
          pClick();
        else
          return false;
        return true;
      }
    }
    pMenu = (NPCMenu*)((uint32_t)pMenu + sizeof(NPCMenu));
  }

  return false;
}
