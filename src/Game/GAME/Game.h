#pragma once

#include <Windows.h>

// rename D2GameInfoStrc
struct GameStructInfo {
  BYTE _1[0x1F];              // 0x00 // was 0x1B
  char szGameName[0x18];      // 0x1F
  char szGameServerIp[0x56];  // 0x37
  char szAccountName[0x30];   // 0x8D
  char szCharName[0x18];      // 0xBD
  char szRealmName[0x18];     // 0xD5
  BYTE _2[0x158];             // 0xED
  char szGamePassword[0x18];  // 0x245
};