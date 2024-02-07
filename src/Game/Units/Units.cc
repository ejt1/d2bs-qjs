#include "Game/Units/Units.h"

#include "Game/D2DataTbls.h"
#include "Game/UnorganizedConstants.h"
#include "Misc/Localization.h"

///////////////////////////////////////////////////
// Mercenary Class ID's
///////////////////////////////////////////////////
#define MERC_A1 0x010f
#define MERC_A2 0x0152
#define MERC_A3 0x0167
#define MERC_A5 0x0231

// TODO(ejt): see if we can remove this stub and call function directly
uint32_t __declspec(naked) __fastcall D2CLIENT_GetUnitName_STUB(D2UnitStrc* /*UnitAny*/) {
  __asm
  {
    // mov eax, ecx
    jmp D2CLIENT_GetUnitName_I
  }
}

// TODO(ejt): see if we can remove this stub and call function directly
void __declspec(naked) __fastcall D2CLIENT_Interact_ASM(UnitInteraction* /*Struct*/) {
  __asm {
    mov esi, ecx
    jmp D2CLIENT_Interact_I
  }
}

const char* D2UnitStrc::GetUnitName(char* szTmp, size_t bufSize) {
  // TODO(ejt): pointer validation should be done by caller
  if (!this) {
    strcpy_s(szTmp, bufSize, "Unknown");
    return szTmp;
  }
  if (dwType == UNIT_MONSTER) {
    wchar_t* wName = D2CLIENT_GetUnitName(this);
    WideCharToMultiByte(CP_UTF8, 0, wName, -1, szTmp, bufSize, 0, 0);
    return szTmp;
  }
  if (dwType == UNIT_PLAYER && pPlayerData) {
    //	return pUnit->pPlayerData->szName;
    strcpy_s(szTmp, bufSize, pPlayerData->szName);
    return szTmp;
  }
  if (dwType == UNIT_ITEM) {
    wchar_t wBuffer[256] = L"";
    D2CLIENT_GetItemName(this, wBuffer, _countof(wBuffer));
    std::string szBuffer = WideToAnsi(wBuffer);
    if (strchr(szBuffer.c_str(), '\n'))
      *strchr(&szBuffer[0], '\n') = 0x00;

    strcpy_s(szTmp, bufSize, szBuffer.c_str());
    return szTmp;
  }
  if (dwType == UNIT_OBJECT || dwType == UNIT_TILE) {
    if (pObjectData && pObjectData->pTxt) {
      strcpy_s(szTmp, bufSize, pObjectData->pTxt->szName);
      return szTmp;
    }
  }
  strcpy_s(szTmp, bufSize, "Unknown");
  return szTmp;
}

void D2UnitStrc::GetItemCode(char* szBuf) {
  if (dwType == UNIT_ITEM) {
    D2ItemsTxt* pTxt = D2COMMON_GetItemText(dwTxtFileNo);
    if (pTxt) {
      memcpy(szBuf, pTxt->szCode, 3);
      szBuf[3] = 0x00;
    }
  }
}

#define HAS_BIT(value, bit) ((((value) >> (bit)) & 0x1) == 0x1)

// TODO: If UnitId is the unique id of the unit, we can just look up that
// location in the table
static D2UnitStrc* GetUnitFromTables(UnitHashTable* unitTables, DWORD dwTypeLow, DWORD dwTypeHigh, const char* szName, DWORD dwClassId, DWORD dwType, DWORD dwMode,
                                     DWORD dwUnitId) {
  unsigned int i, j;
  unsigned int hashLow, hashHigh;
  D2UnitStrc* tmpUnit;

  if (dwUnitId != -1)
    hashLow = hashHigh = dwUnitId & 0x7F;  // % 128
  else {
    hashLow = 0;
    hashHigh = 127;
  }

  // Go through all the types
  for (i = dwTypeLow; i <= dwTypeHigh; ++i) {
    // Go through all the hash values
    for (j = hashLow; j <= hashHigh; ++j) {
      // Go through all the units in a given hash value
      for (tmpUnit = unitTables[i].table[j]; tmpUnit != NULL; tmpUnit = tmpUnit->pListNext)
        // Check if it matches
        if (tmpUnit->ValidateUnit(szName, dwClassId, dwType, dwMode, dwUnitId))
          return tmpUnit;
    }
  }

  return NULL;
}

static DWORD dwMax(DWORD a, DWORD b) {
  return a > b ? a : b;
}

static D2UnitStrc* GetNextUnitFromTables(D2UnitStrc* curUnit, UnitHashTable* unitTables, DWORD dwTypeLow, DWORD dwTypeHigh, const char* szName, DWORD dwClassId,
                                         DWORD dwType, DWORD dwMode) {
  unsigned int i, j;
  D2UnitStrc* tmpUnit;

  // If we're looking for the same type unit, or any type then finish off the
  // current inner iterations
  if (dwType == -1 || dwType == curUnit->dwType) {
    i = curUnit->dwType;

    // Finish off the current linked list
    for (tmpUnit = curUnit->pListNext; tmpUnit != NULL; tmpUnit = tmpUnit->pListNext)
      // Check if it matches
      if (tmpUnit->ValidateUnit(szName, dwClassId, dwType, dwMode, (DWORD)-1))
        return tmpUnit;

    // Finish off the current hash table
    for (j = (curUnit->dwUnitId & 0x7f) + 1; j <= 127; ++j)
      // Go through all the units in this linked list
      for (tmpUnit = unitTables[i].table[j]; tmpUnit != NULL; tmpUnit = tmpUnit->pListNext)
        // Check if it matches
        if (tmpUnit->ValidateUnit(szName, dwClassId, dwType, dwMode, (DWORD)-1))
          return tmpUnit;
  }

  // Go through all the remaining types
  for (i = dwMax(dwTypeLow, curUnit->dwType + 1); i <= dwTypeHigh; ++i) {
    // Go through all the hash values
    for (j = 0; j < 127; ++j) {
      // Go through all the units in a given hash value
      for (tmpUnit = unitTables[i].table[j]; tmpUnit != NULL; tmpUnit = tmpUnit->pListNext)
        // Check if it matches
        if (tmpUnit->ValidateUnit(szName, dwClassId, dwType, dwMode, (DWORD)-1))
          return tmpUnit;
    }
  }

  return NULL;
}

D2UnitStrc* D2UnitStrc::FindUnit(uint32_t id, uint32_t type) {
  if (id == -1)
    return nullptr;

  D2UnitStrc* pUnit = D2CLIENT_FindServerSideUnit(id, type);
  return pUnit ? pUnit : D2CLIENT_FindClientSideUnit(id, type);
}

D2UnitStrc* D2UnitStrc::FindUnit(const char* szName, uint32_t classId, uint32_t type, uint32_t mode, uint32_t unitId) {
  // do we need to validate player exists? It replaces the old "GameState == InGame"
  D2UnitStrc* pPlayer = D2CLIENT_GetPlayerUnit();
  if (!pPlayer) {
    return nullptr;
  }

  // If we have a valid type, just check that value, other wise, check all
  // values. There are 6 valid types, 0-5
  if (type == 3)
    return GetUnitFromTables(D2CLIENT_ClientSideUnitHashTables, type, type, szName, classId, type, mode, unitId);

  if (type >= 0 && type <= 5)
    return GetUnitFromTables(D2CLIENT_ServerSideUnitHashTables, type, type, szName, classId, type, mode, unitId);
  else
    return GetUnitFromTables(D2CLIENT_ServerSideUnitHashTables, 0, 5, szName, classId, type, mode, unitId);
}

D2UnitStrc* D2UnitStrc::GetNext(const char* name, uint32_t classId, uint32_t type, uint32_t mode) {
  // do we need to validate player exists? It replaces the old "GameState == InGame"
  D2UnitStrc* pPlayer = D2CLIENT_GetPlayerUnit();
  if (!pPlayer) {
    return nullptr;
  }

  if (type == 3)
    return GetNextUnitFromTables(this, D2CLIENT_ClientSideUnitHashTables, type, type, name, classId, type, mode);

  if (type >= 0 && type <= 5)
    return GetNextUnitFromTables(this, D2CLIENT_ServerSideUnitHashTables, type, type, name, classId, type, mode);
  else
    return GetNextUnitFromTables(this, D2CLIENT_ServerSideUnitHashTables, 0, 5, name, classId, type, mode);
}

D2UnitStrc* D2UnitStrc::FindItem(const char* name, uint32_t classId, uint32_t mode, uint32_t unitId) {
  for (D2UnitStrc* pItem = D2COMMON_GetItemFromInventory(pInventory); pItem; pItem = D2COMMON_GetNextItemFromInventory(pItem)) {
    if (pItem->ValidateUnit(name, classId, 4, mode, unitId))
      return pItem;
  }

  return NULL;
}

D2UnitStrc* D2UnitStrc::GetNextItem(D2UnitStrc* pOwner, const char* name, uint32_t classId, uint32_t mode) {
  if (dwType == UNIT_ITEM) {
    // Check first if it belongs to a person
    if (pItemData && pItemData->pOwnerInventory && pItemData->pOwnerInventory == pOwner->pInventory) {
      // Get the next matching unit from the owner's inventory
      for (D2UnitStrc* pItem = D2COMMON_GetNextItemFromInventory(this); pItem; pItem = D2COMMON_GetNextItemFromInventory(pItem)) {
        if (pItem->ValidateUnit(name, classId, 4, mode, (DWORD)-1))
          return pItem;
      }
    }
  }

  return NULL;
}

bool D2UnitStrc::ValidateUnit(const char* name, uint32_t classId, uint32_t type, uint32_t mode, uint32_t unitId) {
  if ((unitId != -1 && dwUnitId != unitId) || (type != -1 && dwType != type) || (classId != -1 && dwTxtFileNo != classId))
    return FALSE;

  if (mode != -1) {
    if (mode >= 100 && dwType == UNIT_ITEM) {
      if (pItemData && mode - 100 != pItemData->ItemLocation)
        return FALSE;
    } else {
      if (HAS_BIT(mode, 29)) {
        bool result = false;
        // mode is a mask
        for (unsigned int i = 0; i < 28; i++)
          if (HAS_BIT(mode, i) && dwMode == i)
            result = true;
        if (!result)
          return FALSE;
      } else if (dwMode != mode)
        return FALSE;
    }
  }

  if (name && name[0]) {
    char szBuf[512] = "";

    if (type == UNIT_ITEM)
      GetItemCode(szBuf);
    else
      GetUnitName(szBuf, 512);
    if (!!_stricmp(szBuf, name))
      return FALSE;
  }

  return TRUE;
}

D2UnitStrc* D2UnitStrc::FindMercUnit() {
  for (D2ActiveRoomStrc* pRoom = pAct->pRoom1; pRoom; pRoom = pRoom->pRoomNext)
    for (D2UnitStrc* pMerc = pRoom->pUnitFirst; pMerc; pMerc = pMerc->pRoomNext)
      if (pMerc->dwType == UNIT_MONSTER &&
          (pMerc->dwTxtFileNo == MERC_A1 || pMerc->dwTxtFileNo == MERC_A2 || pMerc->dwTxtFileNo == MERC_A3 || pMerc->dwTxtFileNo == MERC_A5) &&
          D2CLIENT_GetMonsterOwner(pMerc->dwUnitId) == dwUnitId)
        return pMerc;
  return NULL;

#if 0
	// Wanted way of doing things, but D2CLIENT_GetMercUnit does some wierd internal things (drawing, causing screen flicker)
	for(D2UnitStrc* pMerc = D2CLIENT_GetMercUnit(); pMerc; pMerc = pMerc->pRoomNext)
		if (D2CLIENT_GetMonsterOwner(pMerc->dwUnitId) == pUnit->dwUnitId)
			return pMerc;
	return NULL;
#endif
}

D2SkillStrc* D2UnitStrc::GetStartSkill() {
  if (pInfo) {
    return pInfo->pFirstSkill;
  }
  return nullptr;
}

D2SkillStrc* D2UnitStrc::GetLeftSkill() {
  if (pInfo) {
    return pInfo->pLeftSkill;
  }
  return nullptr;
}

D2SkillStrc* D2UnitStrc::GetRightSkill() {
  if (pInfo) {
    return pInfo->pRightSkill;
  }
  return nullptr;
}

D2SkillStrc* D2UnitStrc::GetUsedSkill() {
  return nullptr;
}

D2SkillStrc* D2UnitStrc::GetSkillFromSkillId(int skillId) {
  D2SkillStrc* skill = GetStartSkill();
  while (skill && skill->pSkillInfo->wSkillId != skillId) {
    skill = skill->GetNext();
  }
  return skill;
}

D2SkillStrc* D2UnitStrc::GetSkillFromSkillName(const char* name) {
  D2SkillStrc* skill = GetStartSkill();
  while (skill && _stricmp(skill->GetName(), name) != 0) {
    skill = skill->GetNext();
  }
  return skill;
}

int D2UnitStrc::GetHealth() {
  return (int)(D2COMMON_GetUnitStat(this, STAT_HP, 0) >> 8);
}

int D2UnitStrc::GetMana() {
  return (int)(D2COMMON_GetUnitStat(this, STAT_MANA, 0) >> 8);
}

uint32_t D2UnitStrc::GetX() {
  return D2CLIENT_GetUnitX(this);
}

uint32_t D2UnitStrc::GetY() {
  return D2CLIENT_GetUnitY(this);
}

void D2UnitStrc::Interact(uint32_t dwMoveType) {
  if (!D2UnitStrc::FindUnit(dwUnitId, dwType))
    return;

  UnitInteraction pInteract = {
      dwMoveType, D2CLIENT_GetPlayerUnit(), this, GetX(), GetY(), 0, 0,
  };

  D2CLIENT_Interact_STUB(&pInteract);
}

int D2UnitStrc::GetItemLocation() {
  if (!pItemData)
    return -1;

  return (pItemData->GameLocation);
}
