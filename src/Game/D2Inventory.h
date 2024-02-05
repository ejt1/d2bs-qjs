#pragma once

struct D2InventoryStrc {
  DWORD dwSignature;     // 0x00
  BYTE* bGame1C;         // 0x04
  D2UnitStrc* pOwner;       // 0x08
  D2UnitStrc* pFirstItem;   // 0x0C
  D2UnitStrc* pLastItem;    // 0x10
  DWORD _1[2];           // 0x14
  DWORD dwLeftItemUid;   // 0x1C
  D2UnitStrc* pCursorItem;  // 0x20
  DWORD dwOwnerId;       // 0x24
  DWORD dwItemCount;     // 0x28
};

// name???
struct InventoryLayout {
  BYTE SlotWidth;
  BYTE SlotHeight;
  BYTE unk1;
  BYTE unk2;
  DWORD Left;
  DWORD Right;
  DWORD Top;
  DWORD Bottom;
  BYTE SlotPixelWidth;
  BYTE SlotPixelHeight;
};