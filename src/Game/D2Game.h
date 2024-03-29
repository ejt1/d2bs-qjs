#pragma once

#include <Windows.h>

struct D2GameInfoStrc {
  uint8_t _1[0x1F];           // 0x00 // was 0x1B
  char szGameName[0x18];      // 0x1F
  char szGameServerIp[0x56];  // 0x37
  char szAccountName[0x30];   // 0x8D
  char szCharName[0x18];      // 0xBD
  char szRealmName[0x18];     // 0xD5
  uint8_t _2[0x158];          // 0xED
  char szGamePassword[0x18];  // 0x245
};

inline D2GameInfoStrc** D2CLIENT_GameInfo = nullptr;