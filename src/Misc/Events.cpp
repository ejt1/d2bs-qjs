#include "ScriptEngine.h"
#include "Engine.h"

bool __fastcall LifeEventCallback(Script* script, void* argv, uint32_t argc) {
  SingleArgHelper* helper = (SingleArgHelper*)argv;
  if (script->IsRunning() && script->GetListenerCount("melife") > 0) {
    Event* evt = new Event;
    // evt->owner = script;
    evt->argc = argc;
    evt->name = "melife";
    evt->arg1 = new DWORD(helper->arg1);

    script->DispatchEvent(evt);
  }
  return true;
}

void LifeEvent(DWORD dwLife) {
  SingleArgHelper helper = {dwLife};
  sScriptEngine->ForEachScript(LifeEventCallback, &helper, 1);
}

bool __fastcall ManaEventCallback(Script* script, void* argv, uint32_t argc) {
  SingleArgHelper* helper = (SingleArgHelper*)argv;
  if (script->IsRunning() && script->GetListenerCount("memana") > 0) {
    Event* evt = new Event;
    // evt->owner = script;
    evt->argc = argc;
    evt->name = "memana";
    evt->arg1 = new DWORD(helper->arg1);

    script->DispatchEvent(evt);
  }
  return true;
}

void ManaEvent(DWORD dwMana) {
  SingleArgHelper helper = {dwMana};
  sScriptEngine->ForEachScript(ManaEventCallback, &helper, 1);
}

bool __fastcall KeyEventCallback(Script* script, void* argv, uint32_t argc) {
  KeyEventHelper* helper = (KeyEventHelper*)argv;
  const char* name = (helper->up ? "keyup" : "keydown");
  if (script->IsRunning() && script->GetListenerCount(name) > 0) {
    Event* evt = new Event;
    // evt->owner = script;
    evt->argc = argc;
    evt->name = name;
    evt->arg1 = new DWORD((DWORD)helper->key);

    script->DispatchEvent(evt);
  }
  bool block = false;
  name = (helper->up ? "keyupblocker" : "keydownblocker");
  if (script->IsRunning() && script->GetListenerCount(name) > 0) {
    Event* evt = new Event;
    // evt->owner = script;
    evt->argc = argc;
    evt->name = name;
    evt->arg1 = new DWORD((DWORD)helper->key);
    evt->arg4 = new DWORD(false);
    ResetEvent(Vars.eventSignal);
    script->DispatchEvent(evt);

    if (WaitForSingleObject(Vars.eventSignal, 1000) == WAIT_TIMEOUT)
      return false;

    bool* global = (bool*)evt->arg4;
    block = *global;
    delete evt->arg1;
    delete evt->arg4;
    delete evt;
  }

  return block;
}

bool KeyDownUpEvent(WPARAM key, BYTE bUp) {
  KeyEventHelper helper = {bUp, key};
  return sScriptEngine->ForEachScript(KeyEventCallback, &helper, 2);
}

bool __fastcall PlayerAssignCallback(Script* script, void* argv, uint32_t argc) {
  SingleArgHelper* helper = (SingleArgHelper*)argv;
  if (script->IsRunning() && script->GetListenerCount("playerassign") > 0) {
    Event* evt = new Event;
    // evt->owner = script;
    evt->argc = argc;
    evt->name = "playerassign";
    evt->arg1 = new DWORD((DWORD)helper->arg1);

    script->DispatchEvent(evt);
  }
  return true;
}

void PlayerAssignEvent(DWORD dwUnitId) {
  SingleArgHelper helper = {dwUnitId};
  sScriptEngine->ForEachScript(PlayerAssignCallback, &helper, 1);
}

bool __fastcall MouseClickCallback(Script* script, void* argv, uint32_t argc) {
  QuadArgHelper* helper = (QuadArgHelper*)argv;
  if (script->IsRunning() && script->GetListenerCount("mouseclick") > 0) {
    Event* evt = new Event;
    // evt->owner = script;
    evt->argc = argc;
    evt->name = "mouseclick";
    evt->arg1 = new DWORD(helper->arg1);
    evt->arg2 = new DWORD(helper->arg2);
    evt->arg3 = new DWORD(helper->arg3);
    evt->arg4 = new DWORD(helper->arg4);

    script->DispatchEvent(evt);
  }
  return true;
}

void MouseClickEvent(int button, POINT pt, bool bUp) {
  QuadArgHelper helper = {static_cast<DWORD>(button), static_cast<DWORD>(pt.x), static_cast<DWORD>(pt.y), bUp};
  sScriptEngine->ForEachScript(MouseClickCallback, &helper, 4);
}

bool __fastcall MouseMoveCallback(Script* script, void* argv, uint32_t argc) {
  DoubleArgHelper* helper = (DoubleArgHelper*)argv;
  if (script->IsRunning() && script->GetListenerCount("mousemove") > 0) {
    Event* evt = new Event;
    // evt->owner = script;
    evt->argc = argc;
    evt->name = "mousemove";
    evt->arg1 = new DWORD(helper->arg1);
    evt->arg2 = new DWORD(helper->arg2);

    script->DispatchEvent(evt);
  }
  return true;
}

void MouseMoveEvent(POINT pt) {
  if (pt.x < 1 || pt.y < 1)
    return;
  DoubleArgHelper helper = {static_cast<DWORD>(pt.x), static_cast<DWORD>(pt.y)};
  sScriptEngine->ForEachScript(MouseMoveCallback, &helper, 2);
}

bool __fastcall BCastEventCallback(Script* script, void* argv, uint32_t /*argc*/) {
  BCastEventHelper* helper = (BCastEventHelper*)argv;

  if (script->IsRunning() && script->GetListenerCount("scriptmsg") > 0) {
    uint8_t* data;
    size_t data_len;
    uint8_t** sab_tab;
    size_t sab_tab_len;

    data = JS_WriteObject2(helper->cx, &data_len, helper->argv[0], JS_WRITE_OBJ_SAB | JS_WRITE_OBJ_REFERENCE, &sab_tab, &sab_tab_len);
    if (!data) {
      Log("could not write SAB");
      return true;
    }

    Event* evt = new Event;
    evt->name = "scriptmsg";

    // write data
    evt->data = static_cast<uint8_t*>(malloc(data_len));
    evt->data_len = data_len;
    if (!evt->data) {
      js_free(helper->cx, data);
      js_free(helper->cx, sab_tab);
      return true;
    }
    memcpy(evt->data, data, data_len);

    // write sab
    evt->sab_tab = static_cast<uint8_t**>(malloc(sizeof(uint8_t*) * sab_tab_len));
    if (!evt->sab_tab) {
      free(evt->data);
      js_free(helper->cx, data);
      js_free(helper->cx, sab_tab);
      return true;
    }
    memcpy(evt->sab_tab, sab_tab, sizeof(uint8_t*) * sab_tab_len);
    evt->sab_tab_len = sab_tab_len;

    // increase SAB reference counts
    for (size_t i = 0; i < evt->sab_tab_len; ++i) {
      js_sab_dup(NULL, evt->sab_tab[i]);
    }

    js_free(helper->cx, data);
    js_free(helper->cx, sab_tab);

    script->DispatchEvent(evt);
  }
  return true;
}

void ScriptBroadcastEvent(JSContext* cx, uint32_t argc, JSValue* args) {
  BCastEventHelper helper = {cx, args, argc};
  sScriptEngine->ForEachScript(BCastEventCallback, &helper, argc);
}

bool __fastcall ChatEventCallback(Script* script, void* argv, uint32_t argc) {
  ChatEventHelper* helper = (ChatEventHelper*)argv;
  if (script->IsRunning() && script->GetListenerCount(helper->name) > 0) {
    Event* evt = new Event;
    // evt->owner = script;
    evt->argc = argc;
    evt->name = helper->name;
    evt->arg1 = _strdup(helper->nick);
    evt->arg2 = _strdup(helper->msg);

    script->DispatchEvent(evt);
  }
  std::string evtname = helper->name;
  evtname = evtname + "blocker";
  bool block = false;

  if (script->IsRunning() && script->GetListenerCount(evtname.c_str()) > 0) {
    Event* evt = new Event;
    // evt->owner = script;
    evt->argc = argc;
    evt->name = evtname;
    evt->arg1 = _strdup(helper->nick);
    evt->arg2 = _strdup(helper->msg);
    evt->arg4 = new DWORD(false);
    ResetEvent(Vars.eventSignal);
    script->DispatchEvent(evt);

    if (WaitForSingleObject(Vars.eventSignal, 500) == WAIT_TIMEOUT)
      return false;

    block = (*(DWORD*)evt->arg4);
    free(evt->arg1);
    free(evt->arg2);
    delete evt->arg4;
    delete evt;
  }
  return block;
}

bool ChatEvent(const char* lpszNick, const char* lpszMsg) {
  ChatEventHelper helper = {"chatmsg", lpszNick, lpszMsg};
  return sScriptEngine->ForEachScript(ChatEventCallback, &helper, 2);
}

bool ChatInputEvent(const char* lpszMsg) {
  ChatEventHelper helper = {"chatinput", "me", lpszMsg};
  return sScriptEngine->ForEachScript(ChatEventCallback, &helper, 2);
}

bool WhisperEvent(const char* lpszNick, const char* lpszMsg) {
  ChatEventHelper helper = {"whispermsg", lpszNick, lpszMsg};
  return sScriptEngine->ForEachScript(ChatEventCallback, &helper, 2);
}

bool __fastcall CopyDataCallback(Script* script, void* argv, uint32_t argc) {
  CopyDataHelper* helper = (CopyDataHelper*)argv;
  if (script->IsRunning() && script->GetListenerCount("copydata") > 0) {
    Event* evt = new Event;
    // evt->owner = script;
    evt->argc = argc;
    evt->name = "copydata";
    evt->arg1 = new DWORD(helper->mode);
    evt->arg2 = _strdup(helper->msg);

    script->DispatchEvent(evt);
  }
  return true;
}

void CopyDataEvent(DWORD dwMode, const char* lpszMsg) {
  CopyDataHelper helper = {dwMode, lpszMsg};
  sScriptEngine->ForEachScript(CopyDataCallback, &helper, 2);
}

bool __fastcall ItemEventCallback(Script* script, void* argv, uint32_t argc) {
  ItemEventHelper* helper = (ItemEventHelper*)argv;
  if (script->IsRunning() && script->GetListenerCount("itemaction") > 0) {
    Event* evt = new Event;
    // evt->owner = script;
    evt->argc = argc;
    evt->name = "itemaction";
    evt->arg1 = new DWORD(helper->id);
    evt->arg2 = _strdup(helper->code);
    evt->arg3 = new DWORD(helper->mode);
    evt->arg4 = new bool(helper->global);

    script->DispatchEvent(evt);
  }
  return true;
}

void ItemActionEvent(DWORD GID, char* Code, BYTE Mode, bool Global) {
  ItemEventHelper helper = {GID, Code, Mode, Global};
  sScriptEngine->ForEachScript(ItemEventCallback, &helper, 4);
}

bool __fastcall GameActionEventCallback(Script* script, void* argv, uint32_t argc) {
  GameActionEventHelper* helper = (GameActionEventHelper*)argv;
  if (script->IsRunning() && script->GetListenerCount("gameevent") > 0) {
    Event* evt = new Event;
    // evt->owner = script;
    evt->argc = argc;
    evt->name = "gameevent";
    evt->arg1 = new BYTE(helper->mode);
    evt->arg2 = new DWORD(helper->param1);
    evt->arg3 = new DWORD(helper->param2);
    evt->arg4 = _strdup(helper->name1);
    evt->arg5 = _strdup(helper->name2);

    script->DispatchEvent(evt);
  }
  return true;
}

void GameActionEvent(BYTE mode, DWORD param1, DWORD param2, const char* name1, const char* name2) {
  GameActionEventHelper helper = {mode, param1, param2, name1, name2};
  sScriptEngine->ForEachScript(GameActionEventCallback, &helper, 5);
}

bool __fastcall PacketEventCallback(Script* script, void* argv, uint32_t argc) {
  PacketEventHelper* helper = (PacketEventHelper*)argv;

  if (script->IsRunning() && script->GetListenerCount(helper->name) > 0) {
    Event* evt = new Event;
    // evt->owner = script;
    evt->argc = argc;
    evt->name = helper->name;
    evt->arg1 = new BYTE[helper->dwSize];
    evt->arg2 = new DWORD(helper->dwSize);
    evt->arg4 = new DWORD(false);
    memcpy(evt->arg1, helper->pPacket, helper->dwSize);

    ResetEvent(Vars.eventSignal);
    script->DispatchEvent(evt);
    if (WaitForSingleObject(Vars.eventSignal, 500) == WAIT_TIMEOUT) {
      return false;
    }

    bool retval = (*(DWORD*)evt->arg4);
    delete[] evt->arg1;
    delete evt->arg2;
    delete evt->arg4;
    delete evt;
    return retval;
  }

  return false;
}

bool GamePacketEvent(BYTE* pPacket, DWORD dwSize) {
  PacketEventHelper helper = {"gamepacket", pPacket, dwSize};
  return sScriptEngine->ForEachScript(PacketEventCallback, &helper, 3);
}

bool GamePacketSentEvent(BYTE* pPacket, DWORD dwSize) {
  PacketEventHelper helper = {"gamepacketsent", pPacket, dwSize};
  return sScriptEngine->ForEachScript(PacketEventCallback, &helper, 3);
}

bool RealmPacketEvent(BYTE* pPacket, DWORD dwSize) {
  PacketEventHelper helper = {"realmpacket", pPacket, dwSize};
  return sScriptEngine->ForEachScript(PacketEventCallback, &helper, 3);
}