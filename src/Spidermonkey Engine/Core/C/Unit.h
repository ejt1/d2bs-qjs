#pragma once

#include "D2Ptrs.h"

UnitAny* GetUnit(const char* szName, DWORD dwClassId, DWORD dwType, DWORD dwMode, DWORD dwUnitId);
UnitAny* GetNextUnit(UnitAny* pUnit, const char* szName, DWORD dwClassId, DWORD dwType, DWORD dwMode);
UnitAny* GetInvUnit(UnitAny* pOwner, const char* szName, DWORD dwClassId, DWORD dwMode, DWORD dwUnitId);
UnitAny* GetInvNextUnit(UnitAny* pUnit, UnitAny* pOwner, const char* szName, DWORD dwClassId, DWORD dwMode);
BOOL CheckUnit(UnitAny* pUnit, const char* szName, DWORD dwClassId, DWORD dwType, DWORD dwMode, DWORD dwUnitId);
int GetUnitHP(UnitAny* pUnit);
int GetUnitMP(UnitAny* pUnit);
