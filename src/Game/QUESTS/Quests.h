#pragma once

#include <Window.h>

// rename D2QuestInfoStrc
#pragma pack(push, 1)
struct QuestInfo {
  void* pBuffer;  // 0x00
  DWORD _1;       // 0x04
};
#pragma pack(pop)