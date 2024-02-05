#pragma once

// TODO(ejt): throwing these in here for now to figure out name later

#include <cstdint>

struct MessageHandlerList {
  uint32_t message;
  uint32_t unk_4;
  uint32_t(__stdcall* handler)(void*);
  struct MessageHandlerList* next;
};

struct MessageHandlerHashTable {
  struct MessageHandlerList** table;
  uint32_t length;
};

struct WindowHandlerHashTable {
  struct WindowHandlerList** table;
  uint32_t length;
};

struct WindowHandlerList {
  uint32_t unk_0;
  void* hWnd;
  uint32_t unk_8;
  struct MessageHandlerHashTable* msgHandlers;
  struct WindowHandlerList* next;
};

inline WindowHandlerHashTable* STORM_WindowHandlers = nullptr;