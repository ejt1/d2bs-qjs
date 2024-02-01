#pragma once

#include <windows.h>
#include "D2Structs.h"

DWORD __fastcall GameInput(wchar_t* wMsg);
DWORD __fastcall ChannelInput(wchar_t* wMsg);

void SetMaxDiff(void);
void __fastcall WhisperHandler(char* szAcc, char* szText);
DWORD __fastcall GameAttack(UnitInteraction* pAttack);
void __fastcall GamePlayerAssignment(UnitAny* pPlayer);
void GameLeave(void);

DWORD __fastcall GamePacketReceived(BYTE* pPacket, DWORD dwSize);
DWORD __fastcall GamePacketSent(BYTE* pPacket, DWORD dwSize);
BOOL __fastcall RealmPacketRecv(BYTE* pPacket, DWORD dwSize);
BOOL __fastcall ChatPacketRecv(BYTE* pPacket, int len);

