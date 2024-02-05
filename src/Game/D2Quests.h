#pragma once

#include <cstdint>

// revisit and validate name etc
struct D2NPCMessageStrc {
  wchar_t text[120];           // 0x000
  uint32_t unk[6];             // 0x0F0
  void(__stdcall* handler)();  // 0x108
  uint32_t bMaybeSelectable;   // 0x10C
};

// TODO(ejt): revisit and validate name etc
struct D2NPCMessageTableStrc {
  uint32_t unk[0x14];                        // 0x000
  uint32_t numLines;                         // 0x050
  uint32_t unk_2[0x5];                       // 0x054
  D2NPCMessageStrc dialogLines[10];  // 0x068
  void* something;                           // 0xB08
};

#pragma pack(push, 1)
struct D2QuestInfoStrc {
  void* pBuffer;  // 0x00
  uint32_t _1;    // 0x04
};
#pragma pack(pop)

inline D2NPCMessageTableStrc** D2CLIENT_pTransactionDialogsInfo = nullptr;
inline void** D2CLIENT_TransactionDialog = nullptr;
inline uint32_t* D2CLIENT_TransactionDialogs = nullptr;
inline uint32_t* D2CLIENT_TransactionDialogs_2 = nullptr;
inline void* (*D2CLIENT_GetQuestInfo)() = nullptr;
