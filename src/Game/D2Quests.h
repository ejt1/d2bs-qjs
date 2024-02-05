#pragma once

#include <cstdint>

#pragma pack(push, 1)
struct D2QuestInfoStrc {
  void* pBuffer;  // 0x00
  uint32_t _1;    // 0x04
};
#pragma pack(pop)

inline void* (*D2CLIENT_GetQuestInfo)() = nullptr;