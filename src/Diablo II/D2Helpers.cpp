#include <io.h>
#include <errno.h>
#include <ctime>
#include <cmath>
#include <sstream>
#include <string>

#include "D2Helpers.h"
#include "Helpers.h"
#include "D2Intercepts.h"
#include "Engine.h"
#include "stringhash.h"
#include "CriticalSections.h"

#include "Game/UI/NPCMenu.h"
#include "Game/D2Chat.h"
#include "Game/D2WinControl.h"
#include "Game/D2WinUnknown.h"

void Log(const char* szFormat, ...) {
  va_list vaArgs;

  va_start(vaArgs, szFormat);
  int len = _vscprintf(szFormat, vaArgs);
  char* szString = new char[len + 1];
  vsprintf_s(szString, len + 1, szFormat, vaArgs);
  va_end(vaArgs);

  if (len > 0 && szString[len - 1] == L'\n')
    szString[len - 1] = 0;

  LogNoFormat(szString);
  delete[] szString;
}

void LogNoFormat(const char* szString) {
  time_t tTime;
  struct tm timestamp;
  char szTime[128] = "";
  char path[_MAX_PATH + _MAX_FNAME] = "";
  time(&tTime);
  localtime_s(&timestamp, &tTime);
  strftime(szTime, sizeof(szTime), "%Y%m%d", &timestamp);
  sprintf_s(path, _countof(path), "%sd2bs-%s-%s.log", Vars.szLogPath, Vars.szProfile, szTime);
//#ifdef DEBUG
  //FILE* log = stderr;
//#else
  FILE* log = _fsopen(path, "a+", _SH_DENYNO);
//#endif
  static DWORD id = GetProcessId(GetCurrentProcess());
  DWORD tid = GetCurrentThreadId();
  strftime(szTime, sizeof(szTime), "%x %X", &timestamp);
  fprintf(log, "[%s pid %d tid %d] D2BS: %s\n", szTime, id, tid, szString);
  fflush(log);
  fclose(log);
}

// Do not edit without the express consent of bsdunx or lord2800
ClientGameState ClientState(void) {
  ClientGameState state = ClientStateNull;
  D2UnitStrc* player = D2CLIENT_GetPlayerUnit();
  D2WinControlStrc* firstControl = *D2WIN_FirstControl;

  if (player && !firstControl) {
    if (player && player->pUpdateUnit) {
      state = ClientStateBusy;
      return state;
    }
    if (player->pInventory && player->pPath &&
        // player->pPath->xPos &&
        player->pPath->pRoom1 && player->pPath->pRoom1->pRoom2 && player->pPath->pRoom1->pRoom2->pLevel && player->pPath->pRoom1->pRoom2->pLevel->dwLevelNo)
      state = ClientStateInGame;
    else
      state = ClientStateBusy;
  } else if (!player && firstControl)
    state = ClientStateMenu;
  else if (!player && !firstControl)
    state = ClientStateNull;

  return state;
}

bool GameReady(void) {
  return (ClientState() == ClientStateInGame ? true : false);
}

bool WaitForGameReady(void) {
  DWORD start = GetTickCount();
  do {
    switch (ClientState()) {
      case ClientStateNull:
      case ClientStateMenu:
        return false;
      case ClientStateInGame:
        return true;
    }
    Sleep(10);
  } while ((Vars.dwGameTimeout == 0) || (Vars.dwGameTimeout > 0 && (GetTickCount() - start) < Vars.dwGameTimeout));
  return false;
}

DWORD GetPlayerArea(void) {
  return (ClientState() == ClientStateInGame ? D2CLIENT_GetPlayerUnit()->pPath->pRoom1->pRoom2->pLevel->dwLevelNo : NULL);
}

void SendMouseClick(int x, int y, int clicktype) {
  // HACK: Using PostMessage instead of SendMessage--need to fix this ASAP!
  LPARAM lp = x + (y << 16);
  switch (clicktype) {
    case 0:
      PostMessage(D2GFX_GetHwnd(), WM_LBUTTONDOWN, 0, lp);
      break;
    case 1:
      PostMessage(D2GFX_GetHwnd(), WM_LBUTTONUP, 0, lp);
      break;
    case 2:
      PostMessage(D2GFX_GetHwnd(), WM_RBUTTONDOWN, 0, lp);
      break;
    case 3:
      PostMessage(D2GFX_GetHwnd(), WM_RBUTTONUP, 0, lp);
      break;
  }
}

void SendKeyPress(uint32_t type, uint32_t key, uint32_t ext) {
  LPARAM lp = 1;
  lp |= ext << 24;
  lp |= (MapVirtualKey(key, MAPVK_VK_TO_VSC) << 16);

  if (type == WM_KEYUP) {
    lp |= 0xC0000000;
  }

  PostMessage(D2GFX_GetHwnd(), type, key, lp);
}

BYTE CalcPercent(DWORD dwVal, DWORD dwMaxVal, BYTE iMin) {
  if (dwVal == 0 || dwMaxVal == 0)
    return 0;

  BYTE iRes = (BYTE)((double)dwVal / (double)dwMaxVal * 100.0);
  if (iRes > 100)
    iRes = 100;

  return std::max(iRes, iMin);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// End of Sting's or Mousepad's
///////////////////////////////////////////////////////////////////////////////////////////////////

DWORD __declspec(naked) __fastcall D2CLIENT_GetUIVar_STUB(DWORD /*varno*/) {
  __asm
  {
		mov eax, ecx;
		jmp D2CLIENT_GetUiVar_I;
  }
}

void __declspec(naked) __fastcall D2CLIENT_SetSelectedUnit_STUB(DWORD /*UnitAny*/) {
  __asm
  {
		mov eax, ecx
			jmp D2CLIENT_SetSelectedUnit_I
  }
}

DWORD __declspec(naked) __fastcall D2CLIENT_ClickParty_ASM(DWORD /*RosterUnit*/, DWORD /*Mode*/) {
  __asm
  {
		mov eax, ecx
			jmp D2CLIENT_ClickParty_I
  }
}

// obsoleted - use D2CLIENT_ShopAction instead
// This isn't finished anyway!
void __declspec(naked) __fastcall D2CLIENT_ClickShopItem_ASM(DWORD /*x*/, DWORD /*y*/, DWORD /*BuyOrSell*/) {
  __asm
  {
		mov esi, ecx
			mov edi, edx
			pop eax  // Save return address to eax
			pop ecx  // Buy or sell to ecx
			push ecx
			push 1
			push eax
			jmp D2CLIENT_ClickShopItem_I
  }
}

void __declspec(naked) __fastcall D2CLIENT_ShopAction_ASM(DWORD /*pItem*/, DWORD /*pNpc*/, DWORD /*pNPC*/, DWORD /*_1*/,
                                                          DWORD /*pTable2*/ /* Could be also the ItemCost?*/, DWORD /*dwMode*/, DWORD /*_2*/
                                                          ,
                                                          DWORD /*_3*/) {
  __asm {
		jmp D2CLIENT_ShopAction_I
  }
}

void __declspec(naked) __fastcall D2CLIENT_ClickBelt(DWORD /*x*/, DWORD /*y*/, D2InventoryStrc* /*pInventoryData*/) {
  __asm {
		mov eax, edx
			jmp D2CLIENT_ClickBelt_I
  }
}

void __declspec(naked) __stdcall D2CLIENT_LeftClickItem(DWORD /*Location*/, D2UnitStrc* /*pPlayer*/, D2InventoryStrc* /*pInventory*/, int /*x*/, int /*y*/, DWORD /*dwClickType*/,
                                                        InventoryLayout* /*pLayout*/) {
  __asm
  {
		pop eax  // pop return address
		xchg eax, [esp]  // return address to stack, location to eax
		jmp D2CLIENT_LeftClickItem_I
  }
}

void __declspec(naked) __fastcall D2CLIENT_ClickItemRight_ASM(DWORD /*x*/, DWORD /*y*/, DWORD /*Location*/, DWORD /*Player*/, DWORD /*pUnitInventory*/) {
  __asm
  {
		xchg edx, ecx  // x, y -> y, x
		pop eax  // pop return address
		xchg eax, [esp]  // return address to stack, location to eax
		jmp D2CLIENT_ClickItemRight_I
  }
}

void __declspec(naked) __fastcall D2CLIENT_ClickBeltRight_ASM(DWORD /*pInventory*/, DWORD /*pPlayer*/, DWORD /*HoldShift*/, DWORD /*dwPotPos*/) {
  __asm
  {
		pop eax  // pop return address
		xchg eax, [esp]  // return address to stack, location to eax
		jmp D2CLIENT_ClickBeltRight_I
  }
}

void __declspec(naked) __fastcall D2CLIENT_GetItemDesc_ASM(DWORD /*pUnit*/, wchar_t* /*pBuffer*/) {
  __asm
  {
		PUSH EDI
			MOV EDI, EDX
			PUSH NULL
			PUSH 1  // TRUE = New lines, FALSE = Comma between each stat value
			PUSH ECX
			CALL D2CLIENT_GetItemDesc_I
			POP EDI
			RETN
  }
}

void __declspec(naked) __fastcall D2COMMON_DisplayOverheadMsg_ASM(DWORD /*pUnit*/) {
  __asm
  {
		LEA ESI, [ECX+0xA4]
		MOV EAX, [ECX+0xA4]

		PUSH EAX
			PUSH 0
			call D2COMMON_DisplayOverheadMsg_I

			RETN
  }
}

void __declspec(naked) __fastcall D2CLIENT_MercItemAction_ASM(DWORD /*bPacketType*/, DWORD /*dwSlot*/) {
  __asm
  {
    // mov eax, ecx
    // mov ecx, edx
			jmp D2CLIENT_MercItemAction_I
  }
}

void __declspec(naked) __fastcall D2CLIENT_PlaySound(DWORD /*dwSoundId*/) {
  __asm
  {
		MOV EBX, ECX
			PUSH NULL
			PUSH NULL
			PUSH NULL
			MOV EAX, D2CLIENT_PlayerUnit
			MOV EAX, [EAX]
		MOV ECX, EAX
			MOV EDX, EBX
        // CALL D2CLIENT_PlaySound_I
			RETN
  }
}

__declspec(naked) void __stdcall D2CLIENT_TakeWaypoint(DWORD /*dwWaypointId*/, DWORD /*dwArea*/) {
  __asm {
		PUSH EBP
			MOV EBP, ESP
			SUB ESP, 0x20
			PUSH EBX
			PUSH ESI
			PUSH EDI
			AND DWORD PTR SS:[EBP-0x20],0
			PUSH 0
			CALL _TakeWaypoint
			POP EDI
			POP ESI
			POP EBX
			LEAVE
			RETN 8

_TakeWaypoint:
		PUSH EBP
			PUSH EBX
			PUSH ESI
			PUSH EDI

			XOR EDI, EDI
			MOV EBX, 1
			MOV EDX, DWORD PTR SS : [EBP + 8]  // waypointId
			MOV ECX, DWORD PTR SS : [EBP + 0xC]  // area
			PUSH ECX
			LEA ESI, DWORD PTR SS : [EBP - 0x20]  // struct
		JMP [D2CLIENT_TakeWaypoint_I]

  }
}

void __declspec(naked) __fastcall D2GFX_DrawRectFrame_STUB(RECT* /*rect*/) {
  __asm
  {
		mov eax, ecx;
		jmp D2CLIENT_DrawRectFrame;
  }
}

DWORD __cdecl D2CLIENT_GetMinionCount(D2UnitStrc* pUnit, DWORD dwType) {
  DWORD dwResult;

  __asm
  {
		push eax
			push ecx
			push edx
			mov ecx, pUnit
			mov edx, dwType
			call D2CLIENT_GetMinionCount_I
			mov [dwResult], eax
			pop edx
			pop ecx
			pop eax
  }

  return dwResult;
}

__declspec(naked) void __fastcall D2CLIENT_HostilePartyUnit(D2RosterUnitStrc* /*pUnit*/, DWORD /*dwButton*/) {
  __asm
  {
		mov eax, edx
			jmp [D2CLIENT_ClickParty_II]
  }
}

__declspec(naked) DWORD __fastcall D2CLIENT_SendGamePacket_ASM(DWORD /*dwLen*/, BYTE* /*bPacket*/) {
  __asm
  {
		push edi
			mov edi, ecx
			push edx
			call D2CLIENT_SendGamePacket_I
			pop edi
			ret
  }
}

double GetDistance(long x1, long y1, long x2, long y2, DistanceType type) {
  double dist = 0;
  switch (type) {
    case Euclidean: {
      double dx = (double)(x2 - x1);
      double dy = (double)(y2 - y1);
      dx = pow(dx, 2);
      dy = pow(dy, 2);
      dist = sqrt(dx + dy);
    } break;
    case Chebyshev: {
      long dx = (x2 - x1);
      long dy = (y2 - y1);
      dx = abs(dx);
      dy = abs(dy);
      dist = std::max(dx, dy);
    } break;
    case Manhattan: {
      long dx = (x2 - x1);
      long dy = (y2 - y1);
      dx = abs(dx);
      dy = abs(dy);
      dist = (dx + dy);
    } break;
    default:
      dist = -1;
      break;
  }
  return dist;
}

bool IsScrollingText() {
  if (!WaitForGameReady())
    return false;

  HWND d2Hwnd = D2GFX_GetHwnd();
  WindowHandlerList* whl = STORM_WindowHandlers->table[(0x534D5347 ^ (DWORD)d2Hwnd) % STORM_WindowHandlers->length];
  MessageHandlerHashTable* mhht;
  MessageHandlerList* mhl;

  while (whl) {
    if (whl->unk_0 == 0x534D5347 && whl->hWnd == d2Hwnd) {
      mhht = whl->msgHandlers;
      if (mhht != NULL && mhht->table != NULL && mhht->length != 0) {
        // 0x201 - WM_something click
        mhl = mhht->table[0x201 % mhht->length];

        if (mhl != NULL) {
          while (mhl) {
            if (mhl->message && mhl->unk_4 < 0xffffffff && mhl->handler == D2CLIENT_CloseNPCTalk) {
              return true;
            }
            mhl = mhl->next;
          }
        }
      }
    }
    whl = whl->next;
  }

  return false;
}

void ReadProcessBYTES(HANDLE hProcess, DWORD lpAddress, void* buf, int len) {
  DWORD oldprot, dummy = 0;
  VirtualProtectEx(hProcess, (void*)lpAddress, len, PAGE_READWRITE, &oldprot);
  ReadProcessMemory(hProcess, (void*)lpAddress, buf, len, 0);
  VirtualProtectEx(hProcess, (void*)lpAddress, len, oldprot, &dummy);
}
