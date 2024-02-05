#pragma once

#include "Game/Units/Units.h"

#include <Windows.h>

D2UnitStrc* GetUnit(const char* szName, DWORD dwClassId, DWORD dwType, DWORD dwMode, DWORD dwUnitId);
D2UnitStrc* GetNextUnit(D2UnitStrc* pUnit, const char* szName, DWORD dwClassId, DWORD dwType, DWORD dwMode);
D2UnitStrc* GetInvUnit(D2UnitStrc* pOwner, const char* szName, DWORD dwClassId, DWORD dwMode, DWORD dwUnitId);
D2UnitStrc* GetInvNextUnit(D2UnitStrc* pUnit, D2UnitStrc* pOwner, const char* szName, DWORD dwClassId, DWORD dwMode);
BOOL CheckUnit(D2UnitStrc* pUnit, const char* szName, DWORD dwClassId, DWORD dwType, DWORD dwMode, DWORD dwUnitId);
int GetUnitHP(D2UnitStrc* pUnit);
int GetUnitMP(D2UnitStrc* pUnit);
