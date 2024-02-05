#pragma once

#include <cstdint>

// rename D2HoverTextStrc
struct D2HoverTextStrc {
  uint32_t _1;
  uint32_t dwTrigger;
  uint32_t _2[2];
  char Msg[232];
};

inline uint32_t D2COMMON_DisplayOverheadMsg_I = NULL;

inline D2HoverTextStrc*(__stdcall* D2COMMON_GenerateOverheadMsg)(uint32_t dwUnk, const char* szMsg, uint32_t dwTrigger) = nullptr;
inline void(__stdcall* D2COMMON_FixOverheadMsg)(D2HoverTextStrc* pMsg, uint32_t dwUnk) = nullptr;

#define D2COMMON_DisplayOverheadMsg(pUnit) D2COMMON_DisplayOverheadMsg_ASM((uint32_t)pUnit)