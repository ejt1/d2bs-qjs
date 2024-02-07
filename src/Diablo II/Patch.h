#pragma once

#include "Offset.h"
#include "D2Intercepts.h"
#include "D2Handlers.h"
#include "Engine.h"

PatchHook Patches[] = {
    {PatchCall, 0x7C89D, (DWORD)GameInput_Intercept, 5},           // Updated 1.14d //0047C89D-BASE
    {PatchCall, 0x5F802, (DWORD)GamePacketReceived_Intercept, 5},  // Updated 1.14d //0045F802-BASE
    {PatchJmp, 0x12AE5A, (DWORD)GamePacketSent_Interception, 5},   // Updated 1.14d //0052AE5A-BASE
    {PatchCall, 0x62D72, (DWORD)GetSelectedUnit_Intercept, 5},     // Updated 1.14d //00462D72-BASE
    {PatchJmp, 0x5E4D5, (DWORD)PlayerAssignment_Intercept, 5},     // Updated 1.14d //0045E4D5-BASE
    {PatchBytes, 0x68770, (DWORD)0xc3, 1},                         // Updated 1.14d //00468770-BASE
    {PatchBytes, 0x4EBFB, (BYTE)0x75, 1},                          // Congrats Screen //0044EBFB-BASE
    {PatchCall, 0x4EBEF, (DWORD)CongratsScreen_Intercept, 5},      // Updated 1.14d //0044EBEF-BASE
    {PatchCall, 0x547B6, (DWORD)GameActChange_Intercept, 5},       // Updated 1.14d //004547B6-BASE
    {PatchJmp, 0x567EB, (DWORD)GameActChange2_Intercept, 5},       // Updated 1.14d //004567EB-BASE
    {PatchCall, 0x4F57C, (DWORD)GameLeave_Intercept, 5},           // Updated 1.14d //0044F57C-BASE
    {PatchCall, 0x61704, (DWORD)GameAttack_Intercept, 5},          // Updated 1.14d //00461704-BASE

    // this patch needs a constant in ChatPacketRecv_Interception updated also
    {PatchCall, 0x121B20, (DWORD)ChatPacketRecv_Interception, 7},  // Updated 1.14d //00521B20-BASE

    {PatchCall, 0x48813, (DWORD)Whisper_Intercept, 7},       // Updated 1.14d //00448813-BASE
    {PatchCall, 0x42A61, (DWORD)ChannelInput_Intercept, 5},  // Updated 1.14d //00442A61-BASE

    {PatchCall, 0x2091E5, (DWORD)GameCrashFix_Intercept, 10},  // 1.14d //006091E5-BASE

    {PatchCall, 0x4B1AE, (DWORD)RealmPacketRecv_Interception, 6},

    {PatchCall, 0x8CC4, (DWORD)LogMessageBoxA_Intercept, 6},  // Updated 1.14d //00408CC4-BASE
    {PatchJmp, 0x82E0, (DWORD)D2GAME_exit0, 6},               // NEW 1.14d //004082E0-BASE
    {PatchJmp, 0x1790, (DWORD)ErrorReportLaunch, 6},          // NEW 1.14d //00401790-BASE

    // Dont fade when changing area
    {PatchBytes, 0x476d28, (uint32_t)0x90909090, 4},
    {PatchBytes, 0x476d28 + 4, (uint8_t)0x90, 1},
    // Speed up exiting game by removing sound delay
    {PatchBytes, 0x515FB1, (uint8_t)0x01, 1},
    {PatchBytes, 0x11FE20, (DWORD)0x9090, 2},
};

PatchHook Conditional[] = {
    {PatchJmp, 0x12366C, (DWORD)ClassicSTUB, 5, &Vars.bUseRawCDKey},  // Updated 1.14d //0052366C-BASE
    {PatchJmp, 0x123958, (DWORD)LodSTUB, 5, &Vars.bUseRawCDKey},      // Updated 1.14d //00523958-BASE
    {PatchCall, 0x4EF28, (DWORD)FailToJoin, 6, &Vars.bReduceFTJ},     // FTJ Reducer
    {PatchCall, 0xF5623, (DWORD)Multi, 6, &Vars.bMulti},
    {PatchCall, 0xF5831, (DWORD)Windowname, 6, &Vars.bMulti},
    {PatchCall, 0x11944E, (DWORD)CacheFix, 6, &Vars.bCacheFix},
    {PatchCall, 0x119434, (DWORD)CacheFix, 6, &Vars.bCacheFix},
    {PatchBytes, 0x51C31, (WORD)0x9090, 2, &Vars.bSleepy},  // Sleep
    {PatchBytes, 0x51C40, (WORD)0x9090, 2, &Vars.bSleepy},
    {PatchBytes, 0xFA66F, (BYTE)0xEB, 1, &Vars.bSleepy}  // OOG Sleep
};