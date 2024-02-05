#pragma once

// rename D2InventoryStrc
struct Inventory {
  DWORD dwSignature;     // 0x00
  BYTE* bGame1C;         // 0x04
  UnitAny* pOwner;       // 0x08
  UnitAny* pFirstItem;   // 0x0C
  UnitAny* pLastItem;    // 0x10
  DWORD _1[2];           // 0x14
  DWORD dwLeftItemUid;   // 0x1C
  UnitAny* pCursorItem;  // 0x20
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