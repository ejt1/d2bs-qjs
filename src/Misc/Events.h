#pragma once

#include "js32.h"

#include <windows.h>

class Script;
typedef std::list<JSValue> FunctionList;

struct Event {
  Event()
      :  arg1(nullptr), arg2(nullptr), arg3(nullptr), arg4(nullptr), arg5(nullptr), data(nullptr), data_len(0), sab_tab(nullptr), sab_tab_len(0){};
  FunctionList functions;
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
bool ScriptMessageEvent(JSContext* ctx, Script* script, JSValue obj);
void ScriptBroadcastEvent(JSContext* cx, JSValue* argv);
void ItemActionEvent(DWORD GID, char* Code, BYTE Mode, bool Global);
bool GamePacketEvent(BYTE* pPacket, DWORD dwSize);
bool GamePacketSentEvent(BYTE* pPacket, DWORD dwSize);
bool RealmPacketEvent(BYTE* pPacket, DWORD dwSize);
bool GenhookClickEvent(Script* script, int button, POINT* loc, JSValue func);
void GenhookHoverEvent(Script* script, POINT* loc, JSValue func);

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
};
struct PacketEventHelper {
  const char* name;
  BYTE* pPacket;
  DWORD dwSize;
};

struct GenhookClickEventHelper {
  const char* name;
  int button;
  POINT* loc;
};