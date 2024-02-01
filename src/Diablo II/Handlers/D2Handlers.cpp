#include <vector>
#include <algorithm>
#include <string>

#include "D2Handlers.h"
#include "D2NetHandlers.h"
#include "Script.h"
#include "ScreenHook.h"
#include "Unit.h"
#include "Helpers.h"
#include "Core.h"
#include "Constants.h"
#include "Events.h"
#include "ScriptEngine.h"
#include "Console.h"
#include "Engine.h"
#include "MapHeader.h"
#include "Offset.h"
#include "CommandLine.h"
#include "Control.h"

DWORD __fastcall GameInput(wchar_t* wMsg) {
  bool send = true;

  if (Vars.bDontCatchNextMsg)
    Vars.bDontCatchNextMsg = false;
  else {
    send = !((wMsg[0] == L'.' && ProcessCommand(wMsg + 1, false)) || ChatInputEvent(wMsg));
  }

  return send ? 0 : -1;  // -1 means block, 0 means send
}

DWORD __fastcall ChannelInput(wchar_t* wMsg) {
  bool send = true;

  if (Vars.bDontCatchNextMsg)
    Vars.bDontCatchNextMsg = false;
  else {
    send = !((wMsg[0] == L'.' && ProcessCommand(wMsg + 1, false)) || ChatInputEvent(wMsg));
  }

  return send;  // false means ignore, true means send
}

void SetMaxDiff(void) {
  if (D2CLIENT_GetDifficulty() == 1 && *p_D2CLIENT_ExpCharFlag) {
    BnetData* pData = *p_D2LAUNCH_BnData;
    if (pData)
      pData->nMaxDiff = 10;
  }
}

void __fastcall WhisperHandler(char* szAcc, char* szText) {
  if (!Vars.bDontCatchNextMsg) {
    wchar_t* szwText = AnsiToUnicode(szText, CP_ACP);
    WhisperEvent(szAcc, szwText);
    delete[] szwText;
  } else
    Vars.bDontCatchNextMsg = FALSE;
}

DWORD __fastcall GameAttack(UnitInteraction* pAttack) {
  if (!pAttack || !pAttack->lpTargetUnit || pAttack->lpTargetUnit->dwType != UNIT_MONSTER)
    return (DWORD)-1;

  if (pAttack->dwMoveType == ATTACKTYPE_UNITLEFT)
    pAttack->dwMoveType = ATTACKTYPE_SHIFTLEFT;

  if (pAttack->dwMoveType == ATTACKTYPE_RIGHT)
    pAttack->dwMoveType = ATTACKTYPE_SHIFTRIGHT;

  return NULL;
}

void __fastcall GamePlayerAssignment(UnitAny* pPlayer) {
  if (!pPlayer)
    return;

  PlayerAssignEvent(pPlayer->dwUnitId);
}

void GameLeave(void) {
  Vars.bQuitting = false;
  sScriptEngine->ForEachScript(StopIngameScript, NULL, 0);
  ActMap::ClearCache();
}

DWORD __fastcall GamePacketReceived(BYTE* pPacket, DWORD dwSize) {
  switch (pPacket[0]) {
    case 0xAE:
      Log(L"Warden activity detected! Terminating Diablo to ensure your safety :)");
      TerminateProcess(GetCurrentProcess(), 0);
      break;
    case 0x15:
      return !GamePacketEvent(pPacket, dwSize) && ReassignPlayerHandler(pPacket, dwSize);
    case 0x26:
      return !GamePacketEvent(pPacket, dwSize) && ChatEventHandler(pPacket, dwSize);
    case 0x2A:
      return !GamePacketEvent(pPacket, dwSize) && NPCTransactionHandler(pPacket, dwSize);
    case 0x5A:
      return !GamePacketEvent(pPacket, dwSize) && EventMessagesHandler(pPacket, dwSize);
    case 0x18:
    case 0x95:
      return !GamePacketEvent(pPacket, dwSize) && HPMPUpdateHandler(pPacket, dwSize);
    case 0x9C:
    case 0x9D:
      return !GamePacketEvent(pPacket, dwSize) && ItemActionHandler(pPacket, dwSize);
    case 0xA7:
      return !GamePacketEvent(pPacket, dwSize) && DelayedStateHandler(pPacket, dwSize);
  }

  return !GamePacketEvent(pPacket, dwSize);
}

DWORD __fastcall GamePacketSent(BYTE* pPacket, DWORD dwSize) {
  return !GamePacketSentEvent(pPacket, dwSize);
}

BOOL __fastcall RealmPacketRecv(BYTE* pPacket, DWORD dwSize) {
  return !RealmPacketEvent(pPacket, dwSize);
}

BOOL __fastcall ChatPacketRecv(BYTE* pPacket, int /*len*/) {
  bool blockPacket = false;

  if (pPacket[1] == 0xF) {
    // DWORD mode = pPacket[4];
    char* who = (char*)pPacket + 28;
    char* said = (char*)pPacket + 29 + strlen(who);
    wchar_t* wsaid = AnsiToUnicode(said, CP_ACP);

    switch (pPacket[4]) {
      case 0x02:  // channel join
        ChatEvent(who, L"joined the channel");
        break;
      case 0x03:  // channel leave
        ChatEvent(who, L"left the channel");
        break;
      case 0x04:  // whispers
      case 0x0A:
        WhisperEvent(who, wsaid);
        break;
      case 0x05:  // normal text
      case 0x12:  // info blue text
      case 0x13:  // error message
      case 0x17:  // emoted text
        ChatEvent(who, wsaid);
        break;
      default:
        break;
    }
    delete[] wsaid;
    // ChannelEvent(mode,who,said);
  }

  return !blockPacket;
}
