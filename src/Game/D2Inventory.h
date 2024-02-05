#pragma once

#include <cstdint>

struct D2UnitStrc;

struct D2InventoryStrc {
  uint32_t dwSignature;     // 0x00
  uint8_t* bGame1C;         // 0x04
  D2UnitStrc* pOwner;       // 0x08
  D2UnitStrc* pFirstItem;   // 0x0C
  D2UnitStrc* pLastItem;    // 0x10
  uint32_t _1[2];           // 0x14
  uint32_t dwLeftItemUid;   // 0x1C
  D2UnitStrc* pCursorItem;  // 0x20
  uint32_t dwOwnerId;       // 0x24
  uint32_t dwItemCount;     // 0x28
};

// name???
struct InventoryLayout {
  uint8_t SlotWidth;
  uint8_t SlotHeight;
  uint8_t unk1;
  uint8_t unk2;
  uint32_t Left;
  uint32_t Right;
  uint32_t Top;
  uint32_t Bottom;
  uint8_t SlotPixelWidth;
  uint8_t SlotPixelHeight;
};

inline InventoryLayout** D2CLIENT_TradeLayout = nullptr;
inline InventoryLayout** D2CLIENT_StashLayout = nullptr;
inline InventoryLayout** D2CLIENT_StoreLayout = nullptr;
inline InventoryLayout** D2CLIENT_CubeLayout = nullptr;
inline InventoryLayout** D2CLIENT_InventoryLayout = nullptr;
inline InventoryLayout** D2CLIENT_MercLayout = nullptr;