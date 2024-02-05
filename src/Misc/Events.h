#pragma once

#include "js32.h"

#include <windows.h>

typedef std::list<JSValue> FunctionList;

struct Event {
  Event()
      : argv(nullptr), argc(0), arg1(nullptr), arg2(nullptr), arg3(nullptr), arg4(nullptr), arg5(nullptr), data(nullptr), data_len(0), sab_tab(nullptr), sab_tab_len(0){};
  FunctionList functions;
  JSValue* argv;
  uint32_t argc;
  std::string name;
  void* arg1;
  void* arg2;
  void* arg3;
  void* arg4;
  void* arg5;

  // new
  uint8_t* data;
  size_t data_len;

  uint8_t** sab_tab;
  size_t sab_tab_len;
};

bool ChatEvent(const char* lpszNick, const char* lpszMsg);
bool ChatInputEvent(const char* lpszMsg);
void LifeEvent(DWORD dwLife);
void ManaEvent(DWORD dwMana);
void CopyDataEvent(DWORD dwMode, const char* lpszMsg);
void GameActionEvent(BYTE mode, DWORD param1, DWORD param2, const char* name1, const char* name2);
bool WhisperEvent(const char* lpszNick, const char* lpszMsg);
bool KeyDownUpEvent(WPARAM bByte, BYTE bUp);
void PlayerAssignEvent(DWORD dwUnitId);
void MouseClickEvent(int button, POINT pt, bool bUp);
void MouseMoveEvent(POINT pt);
void ScriptBroadcastEvent(JSContext* cx, uint32_t argc, JSValue* argv);
void ItemActionEvent(DWORD GID, char* Code, BYTE Mode, bool Global);
bool GamePacketEvent(BYTE* pPacket, DWORD dwSize);
bool GamePacketSentEvent(BYTE* pPacket, DWORD dwSize);
bool RealmPacketEvent(BYTE* pPacket, DWORD dwSize);

void ReleaseGameLock(void);
void TakeGameLock(void);

struct ChatEventHelper {
  const char *name, *nick;
  const char* msg;
};

struct CopyDataHelper {
  DWORD mode;
  const char* msg;
};

struct ItemEventHelper {
  DWORD id;
  char* code;
  WORD mode;
  bool global;
};

struct KeyEventHelper {
  BOOL up;
  WPARAM key;
};

struct GameActionEventHelper {
  BYTE mode;
  DWORD param1, param2;
  const char* name1;
  const char* name2;
};

struct SingleArgHelper {
  DWORD arg1;
};

struct DoubleArgHelper {
  DWORD arg1, arg2;
};

struct TripleArgHelper {
  DWORD arg1, arg2, arg3;
};
struct QuadArgHelper {
  DWORD arg1, arg2, arg3, arg4;
};

struct BCastEventHelper {
  JSContext* cx;
  JSValue* argv;
  uint32_t argc;
};
struct PacketEventHelper {
  const char* name;
  BYTE* pPacket;
  DWORD dwSize;
};
