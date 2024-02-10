#pragma once

#include "js32.h"

#include <condition_variable>
#include <mutex>
#include <windows.h>

class Script;
typedef std::list<JSValue> FunctionList;

struct Event {
  Event() : block(false){};
  virtual ~Event() {
  }

  std::string name;
  bool block;
};

struct HealthManaEvent : Event {
  uint32_t value;
};

struct KeyEvent : Event {
  uint32_t key;
};

struct PlayerAssignedEvent : Event {
  uint32_t id;
};

struct MouseEvent : Event {
  uint32_t x;
  uint32_t y;
  uint32_t button;
  uint32_t state;
};

struct ScriptMsgEvent : Event {
  uint8_t* data;
  size_t data_len;

  uint8_t** sab_tab;
  size_t sab_tab_len;
};

struct ChatMessageEvent : Event {
  std::string nickname;
  std::string msg;
};

struct CopyDataMessageEvent : Event {
  uint32_t mode;
  std::string msg;
};

struct ItemEvent : Event {
  uint32_t id;
  std::string code;  // could be char[4] too
  uint16_t mode;
  bool global;  // ?
};

struct GameEvent : Event {
  uint8_t mode;
  uint32_t param1;
  uint32_t param2;
  std::string name1;
  std::string name2;
};

struct PacketEvent : Event {
  std::vector<uint8_t> bytes;
};

struct GenHookEvent : Event {
  uint32_t x;
  uint32_t y;
  uint32_t button;
  JSValue callback;
};

struct CommandEvent : Event {
  std::string command;
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