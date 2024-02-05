#pragma once

#include <cstdint>

#pragma pack(push, 1)
struct NPCMenu {
  uint32_t dwNPCClassId;
  uint32_t dwEntryAmount;
  uint16_t wEntryId1;
  uint16_t wEntryId2;
  uint16_t wEntryId3;
  uint16_t wEntryId4;
  uint16_t _1;
  uint32_t dwEntryFunc1;
  uint32_t dwEntryFunc2;
  uint32_t dwEntryFunc3;
  uint32_t dwEntryFunc4;
  uint8_t _2[5];
};
#pragma pack(pop)

inline NPCMenu** D2CLIENT_NPCMenu = nullptr;
inline uint32_t* D2CLIENT_NPCMenuAmount = nullptr;