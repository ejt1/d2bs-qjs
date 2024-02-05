#include "Patch.h"
#include "Engine.h"

#include "Game/D2Game.h"
#include "Game/D2Quests.h"
#include "Game/D2Roster.h"
#include "Game/Unorganized.h"

#ifndef ArraySize
#define ArraySize(x) (sizeof((x)) / sizeof((x)[0]))
#endif

// deprecated, replaced by InitOffsets
void DefineOffsets() {
  //uint32_t** p = (uint32_t**)d2ptrs_list;
  //do {
  //  **p = GetDllOffset(**p);
  //} while (ptrdiff_t(++p) < ((ptrdiff_t)d2ptrs_list) + sizeof(d2ptrs_list));
}

template <typename T>
static void InitOffset(T* ptr, size_t new_ptr) {
  *reinterpret_cast<T**>(ptr) = reinterpret_cast<T*>(new_ptr);
}

void InitOffsets() {
  size_t base = reinterpret_cast<size_t>(GetModuleHandle(nullptr));

  // Game.h
  InitOffset(&D2CLIENT_GameInfo, base + 0x3A0438);

  // Quests.h
  InitOffset(&D2CLIENT_GetQuestInfo, base + 0xB32D0);

  // Units.h
  InitOffset(&D2CLIENT_FindClientSideUnit, base + 0x63990);
  InitOffset(&D2CLIENT_FindServerSideUnit, base + 0x639B0);
  InitOffset(&D2CLIENT_GetCurrentInteractingNPC, base + 0xB1620);
  InitOffset(&D2CLIENT_GetSelectedUnit, base + 0x67A10);
  InitOffset(&D2CLIENT_GetCursorItem, base + 0x680A0);
  // InitOffset(&D2CLIENT_GetMercUnit, base + 0x78A90);
  InitOffset(&D2CLIENT_SetSelectedUnit_I, base + 0x66DE0);
  InitOffset(&D2CLIENT_GetItemName, base + 0x8C060);
  InitOffset(&D2CLIENT_LoadItemDesc, base + 0x8DD90);
  InitOffset(&D2CLIENT_GetMonsterOwner, base + 0x79150);
  InitOffset(&D2CLIENT_GetUnitHPPercent, base + 0x79080);
  InitOffset(&D2CLIENT_InitInventory, base + 0x845A0);
  InitOffset(&D2CLIENT_SetUIVar, base + 0x55F20);
  InitOffset(&D2CLIENT_GetUnitX, base + 0x5ADF0);
  InitOffset(&D2CLIENT_GetUnitY, base + 0x5AE20);
  InitOffset(&D2CLIENT_ShopAction, base + 0xB3870);
  InitOffset(&D2CLIENT_LeftClickItem_I, base + 0x8FFE0);
  InitOffset(&D2CLIENT_ServerSideUnitHashTables, base + 0x3A5E70);
  InitOffset(&D2CLIENT_ClientSideUnitHashTables, base + 0x3A5270);
  InitOffset(&D2CLIENT_PlayerUnit, base + 0x3A6A70);
  InitOffset(&D2CLIENT_SelectedInvItem, base + 0x3BCBF4);

  // D2Inventory.h
  InitOffset(&D2CLIENT_TradeLayout, base + 0x3BCA30);
  InitOffset(&D2CLIENT_StashLayout, base + 0x3BCA78);
  InitOffset(&D2CLIENT_StoreLayout, base + 0x3BCB58);
  InitOffset(&D2CLIENT_CubeLayout, base + 0x3BCB70);
  InitOffset(&D2CLIENT_InventoryLayout, base + 0x3BCB88);
  InitOffset(&D2CLIENT_MercLayout, base + 0x3BCD4C);

  // D2Roster.h
  InitOffset(&D2CLIENT_PlayerUnitList, base + 0x3BB5C0);

  // unorganized
  InitOffset(&D2CLIENT_SubmitItem, base + 0xB2370);
  InitOffset(&D2CLIENT_Transmute, base + 0x8A0D0);
  InitOffset(&D2CLIENT_CloseNPCInteract, base + 0xB3F10);
  InitOffset(&D2CLIENT_CloseInteract, base + 0x4C6B0);
  InitOffset(&D2CLIENT_GetAutomapSize, base + 0x5A710);
  InitOffset(&D2CLIENT_NewAutomapCell, base + 0x57C30);
  InitOffset(&D2CLIENT_AddAutomapCell, base + 0x57B00);
  InitOffset(&D2CLIENT_RevealAutomapRoom, base + 0x58F40);
  InitOffset(&D2CLIENT_InitAutomapLayer_I, base + 0x58D40);
  InitOffset(&D2CLIENT_ClickMap, base + 0x62D00);
  InitOffset(&D2CLIENT_GetMouseXOffset, base + 0x5AFC0);
  InitOffset(&D2CLIENT_GetMouseYOffset, base + 0x5AFB0);
  InitOffset(&D2CLIENT_PrintGameString, base + 0x9E3A0);
  InitOffset(&D2CLIENT_PrintPartyString, base + 0x9E5C0);
  InitOffset(&D2CLIENT_LeaveParty, base + 0x79FC0);
  InitOffset(&D2CLIENT_AcceptTrade, base + 0xB9070);
  InitOffset(&D2CLIENT_CancelTrade, base + 0xB90B0);
  InitOffset(&D2CLIENT_TradeOK, base + 0xB8A30);
  InitOffset(&D2CLIENT_GetDifficulty, base + 0x4DCD0);
  InitOffset(&D2CLIENT_ExitGame, base + 0x4DD60);
  InitOffset(&D2CLIENT_GetUiVar_I, base + 0x538D0);
  InitOffset(&D2CLIENT_DrawRectFrame, base + 0x52E50);
  InitOffset(&D2CLIENT_PerformGoldDialogAction, base + 0x54080);
  InitOffset(&D2CLIENT_GetPlayerUnit, base + 0x63DD0);
  InitOffset(&D2CLIENT_ClearScreen, base + 0xB4620);
  InitOffset(&D2CLIENT_CloseNPCTalk, base + 0xA17D0);
  InitOffset(&D2CLIENT_TestPvpFlag, base + 0xDC440);
  InitOffset(&D2CLIENT_GetGameLanguageCode, base + 0x125150);

  InitOffset(&D2CLIENT_ScreenSizeX, base + 0x31146C);
  InitOffset(&D2CLIENT_ScreenSizeY, base + 0x311470);
  InitOffset(&D2CLIENT_CursorHoverX, base + 0x321E4C);
  InitOffset(&D2CLIENT_CursorHoverY, base + 0x321E50);
  InitOffset(&D2CLIENT_MouseY, base + 0x3A6AAC);
  InitOffset(&D2CLIENT_MouseX, base + 0x3A6AB0);
  InitOffset(&D2CLIENT_MouseOffsetY, base + 0x3A5208);
  InitOffset(&D2CLIENT_MouseOffsetZ, base + 0x3A5214);
  InitOffset(&D2CLIENT_MouseOffsetX, base + 0x3A520C);
  InitOffset(&D2CLIENT_AutomapOn, base + 0x3A27E8);
  InitOffset(&D2CLIENT_AutomapMode, base + 0x311254);
  InitOffset(&D2CLIENT_Offset, base + 0x3A5198);
  InitOffset(&D2CLIENT_AutomapLayer, base + 0x3A5164);
  InitOffset(&D2CLIENT_MercReviveCost, base + 0x3C0DD0);
  InitOffset(&D2CLIENT_ViewportY, base + 0x3A5208);
  InitOffset(&D2CLIENT_ViewportX, base + 0x3A520C);
  InitOffset(&D2CLIENT_GoldDialogAction, base + 0x3A279C);
  InitOffset(&D2CLIENT_GoldDialogAmount, base + 0x3A2A68);
  InitOffset(&D2CLIENT_NPCMenu, base + 0x326C48);
  InitOffset(&D2CLIENT_NPCMenuAmount, base + 0x325A74);
  InitOffset(&D2CLIENT_RegularCursorType, base + 0x3A6AF0);
  InitOffset(&D2CLIENT_ShopCursorType, base + 0x3BCBF0);
  InitOffset(&D2CLIENT_Ping, base + 0x3A04A4);
  InitOffset(&D2CLIENT_Skip, base + 0x3A04B0);
  InitOffset(&D2CLIENT_FPS, base + 0x3BB390);
  InitOffset(&D2CLIENT_Lang, base + 0x3BB5DC);
  InitOffset(&D2CLIENT_Divisor, base + 0x311254);
  InitOffset(&D2CLIENT_OverheadTrigger, base + 0x3BF20E);
  InitOffset(&D2CLIENT_RecentInteractId, base + 0x3C0D25);
  InitOffset(&D2CLIENT_ItemPriceList, base + 0x3C0D43);
  InitOffset(&D2CLIENT_TransactionDialog, base + 0x3C0D63);
  InitOffset(&D2CLIENT_TransactionDialogs, base + 0x3C0E5C);
  InitOffset(&D2CLIENT_TransactionDialogs_2, base + 0x3C0E58);
  InitOffset(&D2CLIENT_pTransactionDialogsInfo, base + 0x3C0E54);
  InitOffset(&D2CLIENT_WaypointTable, base + 0x3BF081);
  InitOffset(&D2CLIENT_bWeapSwitch, base + 0x3BCC4C);
  InitOffset(&D2CLIENT_bTradeAccepted, base + 0x3BCE18);
  InitOffset(&D2CLIENT_bTradeBlock, base + 0x3BCE28);
  InitOffset(&D2CLIENT_RecentTradeId, base + 0x3C0E7C);
  InitOffset(&D2CLIENT_ExpCharFlag, base + 0x3A04F4);
  InitOffset(&D2CLIENT_MapId, base + 0x3A0638);
  InitOffset(&D2CLIENT_AlwaysRun, base + 0x3A0660);
  InitOffset(&D2CLIENT_NoPickUp, base + 0x3A6A90);
  InitOffset(&D2CLIENT_ChatMsg, base + 0x3BB638);
  InitOffset(&D2CLIENT_OrificeId, base + 0x3C547C);
  InitOffset(&D2CLIENT_CursorItemMode, base + 0x3C5474);

  // D2Common Ordinals
  InitOffset(&D2COMMON_InitLevel, base + 0x2424A0);
  InitOffset(&D2COMMON_UnloadAct, base + 0x21AFD3);
  InitOffset(&D2COMMON_GetObjectTxt, base + 0x240E90);
  InitOffset(&D2COMMON_LoadAct, base + 0x2194A0);
  InitOffset(&D2COMMON_GetLevelText, base + 0x21DB70);
  InitOffset(&D2COMMON_GetObjectText, base + 0x240E90);
  InitOffset(&D2COMMON_GetItemText, base + 0x2335F0);
  InitOffset(&D2COMMON_GetLayer, base + 0x21E470);
  InitOffset(&D2COMMON_GetLevel, base + 0x242AE0);
  InitOffset(&D2COMMON_GetStatList, base + 0x2257D0);
  InitOffset(&D2COMMON_CopyStatList, base + 0x225C90);
  InitOffset(&D2COMMON_GetUnitStat, base + 0x225480);
  InitOffset(&D2COMMON_GetUnitState, base + 0x239DF0);
  InitOffset(&D2COMMON_CheckUnitCollision, base + 0x222AA0);
  InitOffset(&D2COMMON_GetRoomFromUnit, base + 0x220BB0);
  InitOffset(&D2COMMON_GetTargetUnitType, base + 0x773C);
  InitOffset(&D2COMMON_GetSkillLevel, base + 0x2442A0);
  InitOffset(&D2COMMON_GetItemLevelRequirement, base + 0x22BA60);
  InitOffset(&D2COMMON_GetItemPrice, base + 0x22FDC0);
  InitOffset(&D2COMMON_GetRepairCost, base + 0x22FE60);
  InitOffset(&D2COMMON_GetItemMagicalMods, base + 0x233EE0);
  InitOffset(&D2COMMON_GetItemFromInventory, base + 0x23B2C0);
  InitOffset(&D2COMMON_GetNextItemFromInventory, base + 0x23DFA0);
  InitOffset(&D2COMMON_GenerateOverheadMsg, base + 0x261110);
  InitOffset(&D2COMMON_FixOverheadMsg, base + 0x261230);
  InitOffset(&D2COMMON_AddRoomData, base + 0x21A070);
  InitOffset(&D2COMMON_RemoveRoomData, base + 0x21A0C0);
  InitOffset(&D2COMMON_GetQuestFlag, base + 0x25C310);
  InitOffset(&D2COMMON_MapToAbsScreen, base + 0x243260);
  InitOffset(&D2COMMON_AbsScreenToMap, base + 0x243510);
  InitOffset(&D2COMMON_CheckWaypoint, base + 0x260E50);
  InitOffset(&D2COMMON_IsTownByLevelNo, base + 0x21AAF0);
  InitOffset(&D2COMMON_GetLevelNoFromRoom, base + 0x21A1B0);
  InitOffset(&D2COMMON_FindRoom1, base + 0x219DA3);
  InitOffset(&D2COMMON_GetItemPalette, base + 0x22C100);
  InitOffset(&D2COMMON_GetMissileOwnerUnit, base + 0x639D0);

  InitOffset(&D2CLIENT_TakeWaypoint_I, base + 0x9D0F1);
  InitOffset(&D2CLIENT_ClickShopItem_I, base + 0xB2653);
  InitOffset(&D2CLIENT_ClickBelt_I, base + 0x98870);
  InitOffset(&D2CLIENT_ClickBeltRight_I, base + 0x98A90);
  InitOffset(&D2CLIENT_ClickItemRight_I, base + 0x87740);
  InitOffset(&D2CLIENT_MercItemAction_I, base + 0x785B0);
  InitOffset(&D2CLIENT_Interact_I, base + 0x2B2E0);
  InitOffset(&D2CLIENT_ClickParty_I, base + 0x79EB0);
  InitOffset(&D2CLIENT_ClickParty_II, base + 0x9B990);
  InitOffset(&D2CLIENT_ShopAction_I, base + 0x7D030);
  InitOffset(&D2CLIENT_GetUnitName_I, base + 0x64A60);
  InitOffset(&D2CLIENT_GetItemDesc_I, base + 0x2E380);
  InitOffset(&D2CLIENT_AssignPlayer_I, base + 0x66200);
  InitOffset(&D2CLIENT_InputCall_I, base + 0x787B0);
  InitOffset(&D2CLIENT_Say_I, base + 0x7CBDA);
  InitOffset(&D2CLIENT_BodyClickTable, base + 0x321E58);
  InitOffset(&D2CLIENT_LoadUIImage_I, base + 0x788B0);
  InitOffset(&D2CLIENT_GetMinionCount_I, base + 0x78EE0);
  InitOffset(&D2CLIENT_GameLeave_I, base + 0x81830);
  InitOffset(&D2CLIENT_CongratsScreen_I, base + 0xF6190);
  InitOffset(&D2CLIENT_GameAddUnit_I, base + 0x628E0);
  InitOffset(&D2CLIENT_SendGamePacket_I, base + 0x78350);
  InitOffset(&D2CLIENT_SendPacket_II, base + 0x12AE62);

  // D2Common Globals
  InitOffset(&D2COMMON_sgptDataTable, base + 0x344304);

  // D2Common Stubs
  InitOffset(&D2COMMON_DisplayOverheadMsg_I, base + 0x2611A0);
  InitOffset(&D2COMMON_GetLevelIdFromRoom_I, base + 0x21A1B0);

  // D2Net Functions
  InitOffset(&D2NET_SendPacket, base + 0x12AE50);
  InitOffset(&D2NET_ReceivePacket, base + 0x12AEB0);
  InitOffset(&D2NET_ReceivePacket_I, base + 0x12B920);

  // D2Net Globals
  InitOffset(&D2NET_CriticalPacketSection, base + 0xB400);

  // D2Gfx Ordinals
  InitOffset(&D2GFX_DrawRectangle, base + 0xF6300);
  InitOffset(&D2GFX_DrawLine, base + 0xF6380);
  InitOffset(&D2GFX_DrawAutomapCell2, base + 0xF6480);
  InitOffset(&D2GFX_GetHwnd, base + 0xF59A0);
  InitOffset(&D2GFX_GetScreenSize, base + 0xF5160);

  // D2Multi Functions
  InitOffset(&D2MULTI_DoChat, base + 0x42810);
  InitOffset(&D2MULTI_PrintChannelText_, base + 0x47AB0);

  // D2Multi Globals
  InitOffset(&D2MULTI_ChatBoxMsg, base + 0x37AE40);
  InitOffset(&D2MULTI_GameListControl, base + 0x398BC0);

  // D2Multi Stubs
  InitOffset(&D2MULTI_ChannelChat_I, base + 0x14BE6);
  InitOffset(&D2MULTI_ChannelEmote_I, base + 0x14856);
  InitOffset(&D2MULTI_ChannelWhisper_I, base + 0x142F0);
  InitOffset(&D2MULTI_ChannelInput_I, base + 0x428D0);

  InitOffset(&D2CMP_InitCellFile, base + 0x201340);
  InitOffset(&D2CMP_DeleteCellFile, base + 0x201A50);

  InitOffset(&D2LANG_GetLocaleText, base + 0x124A30);
  InitOffset(&D2LANG_Say_II, base + 0x126800);

  InitOffset(&D2LAUNCH_BnData, base + 0x3795D4);

  InitOffset(&D2WIN_SetControlText, base + 0xFF5A0);
  InitOffset(&D2WIN_DrawSprites, base + 0xF9870);
  InitOffset(&D2WIN_LoadCellFile, base + 0xFA9B0);

  InitOffset(&D2WIN_TakeScreenshot, base + 0xFA7A0);
  InitOffset(&D2WIN_DrawText, base + 0x102320);
  InitOffset(&D2WIN_GetTextSize, base + 0x102520);
  InitOffset(&D2WIN_SetTextSize, base + 0x102EF0);
  InitOffset(&D2WIN_GetTextWidthFileNo, base + 0x102520);
  InitOffset(&D2WIN_DestroyEditBox, base + 0xFDAA0);
  InitOffset(&D2WIN_DestroyControl, base + 0xf95c0);
  InitOffset(&D2WIN_SetEditBoxCallback, base + 0xFDAD0);
  InitOffset(&D2WIN_SetEditBoxProc, base + 0xFDAD0);
  InitOffset(&D2WIN_SelectEditBoxText, base + 0xfdd00);
  InitOffset(&D2WIN_InitMPQ, base + 0x117332);

  InitOffset(&D2WIN_FirstControl, base + 0x3D55BC);
  InitOffset(&D2WIN_FocusedControl, base + 0x3D55CC);
  InitOffset(&D2MULTI_ChatInputBox, base + 0x398C80);

  InitOffset(&BNCLIENT_DClass, base + 0x123673);
  InitOffset(&BNCLIENT_DLod, base + 0x12395D);

  InitOffset(&BNCLIENT_ClassicKey, base + 0x482744);
  InitOffset(&BNCLIENT_XPacKey, base + 0x48274C);
  InitOffset(&BNCLIENT_KeyOwner, base + 0x482750);
  InitOffset(&STORM_WindowHandlers, base + 0x379300);
  InitOffset(&BNCLIENTDecodeAndLoadKeys, base + 0x1234D0);

  InitOffset(&D2GAME_Rand, base + 0x5C370);
  InitOffset(&D2GAME_exit0, base + 0x576F);
}

uint32_t GetDllOffset(const char* /*DllName*/, int Offset) {
  HMODULE hMod = GetModuleHandle(NULL);

  if (Offset < 0)
    return (uint32_t)GetProcAddress(hMod, (LPCSTR)(-Offset));

  return ((uint32_t)hMod) + Offset;
}

uint32_t GetDllOffset(int num) {
  static const char* dlls[] = {"D2Client.DLL", "D2Common.DLL", "D2Gfx.DLL",    "D2Lang.DLL", "D2Win.DLL", "D2Net.DLL",  "D2Game.DLL",
                               "D2Launch.DLL", "Fog.DLL",      "BNClient.DLL", "Storm.DLL",  "D2Cmp.DLL", "D2Multi.DLL"};
  if ((num & 0xff) > 12)
    return 0;
  return GetDllOffset(dlls[num & 0xff], num >> 8);
}

void InstallPatches() {
  for (int x = 0; x < ArraySize(Patches); x++) {
    Patches[x].bOldCode = new BYTE[Patches[x].dwLen];
    ::ReadProcessMemory(GetCurrentProcess(), (void*)Patches[x].dwAddr, Patches[x].bOldCode, Patches[x].dwLen, NULL);
    Patches[x].pFunc(Patches[x].dwAddr, Patches[x].dwFunc, Patches[x].dwLen);
  }
}

void RemovePatches() {
  for (int x = 0; x < ArraySize(Patches); x++) {
    WriteBytes((void*)Patches[x].dwAddr, Patches[x].bOldCode, Patches[x].dwLen);
    delete[] Patches[x].bOldCode;
  }
}

void InstallConditional() {
  for (int x = 0; x < ArraySize(Conditional); x++) {
    if (Conditional[x].enabled == NULL || *Conditional[x].enabled != TRUE) {
      continue;
    }
    Conditional[x].bOldCode = new BYTE[Conditional[x].dwLen];
    ::ReadProcessMemory(GetCurrentProcess(), (void*)Conditional[x].dwAddr, Conditional[x].bOldCode, Conditional[x].dwLen, NULL);
    Conditional[x].pFunc(Conditional[x].dwAddr, Conditional[x].dwFunc, Conditional[x].dwLen);
  }
}

void RemoveConditional() {
  for (int x = 0; x < ArraySize(Conditional); x++) {
    if (Conditional[x].enabled == NULL || *Conditional[x].enabled != TRUE)
      continue;
    WriteBytes((void*)Conditional[x].dwAddr, Conditional[x].bOldCode, Conditional[x].dwLen);
    delete[] Conditional[x].bOldCode;
  }
}

bool WriteBytes(void* pAddr, void* pData, uint32_t dwLen) {
  DWORD dwOld;

  if (!VirtualProtect(pAddr, dwLen, PAGE_READWRITE, &dwOld))
    return FALSE;

  ::memcpy(pAddr, pData, dwLen);
  return VirtualProtect(pAddr, dwLen, dwOld, &dwOld);
}

void FillBytes(void* pAddr, uint8_t bFill, uint32_t dwLen) {
  BYTE* bCode = new BYTE[dwLen];
  ::memset(bCode, bFill, dwLen);

  WriteBytes(pAddr, bCode, dwLen);

  delete[] bCode;
}

void InterceptLocalCode(uint8_t bInst, uint32_t pAddr, uint32_t pFunc, uint32_t dwLen) {
  BYTE* bCode = new BYTE[dwLen];
  ::memset(bCode, 0x90, dwLen);
  DWORD dwFunc = pFunc - (pAddr + 5);

  bCode[0] = bInst;
  *(DWORD*)&bCode[1] = dwFunc;
  WriteBytes((void*)pAddr, bCode, dwLen);

  delete[] bCode;
}

void PatchCall(uint32_t dwAddr, uint32_t dwFunc, uint32_t dwLen) {
  InterceptLocalCode(INST_CALL, dwAddr, dwFunc, dwLen);
}

void PatchJmp(uint32_t dwAddr, uint32_t dwFunc, uint32_t dwLen) {
  InterceptLocalCode(INST_JMP, dwAddr, dwFunc, dwLen);
}

void PatchBytes(uint32_t dwAddr, uint32_t dwValue, uint32_t dwLen) {
  uint8_t* bCode = new uint8_t[dwLen];
  ::memset(bCode, (uint8_t)dwValue, dwLen);

  WriteBytes((void*)dwAddr, bCode, dwLen);

  delete[] bCode;
}

PatchHook* RetrievePatchHooks(int* pBuffer) {
  *pBuffer = ArraySize(Patches);
  return &Patches[0];
}
