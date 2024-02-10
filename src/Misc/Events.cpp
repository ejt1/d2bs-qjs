#include "ScriptEngine.h"
#include "Engine.h"

#include "Core.h"  // Print

bool __fastcall LifeEventCallback(Script* script, void* argv) {
  SingleArgHelper* helper = (SingleArgHelper*)argv;
  if (script->IsRunning() && script->GetListenerCount("melife") > 0) {
    std::shared_ptr<HealthManaEvent> evt = std::make_shared<HealthManaEvent>();
    evt->name = "melife";
    evt->value = helper->arg1;

    script->DispatchEvent(evt);
  }
  return true;
}

void LifeEvent(DWORD dwLife) {
  SingleArgHelper helper = {dwLife};
  sScriptEngine->ForEachScript(LifeEventCallback, &helper);
}

bool __fastcall ManaEventCallback(Script* script, void* argv) {
  SingleArgHelper* helper = (SingleArgHelper*)argv;
  if (script->IsRunning() && script->GetListenerCount("memana") > 0) {
    std::shared_ptr<HealthManaEvent> evt = std::make_shared<HealthManaEvent>();
    evt->name = "memana";
    evt->value = helper->arg1;

    script->DispatchEvent(evt);
  }
  return true;
}

void ManaEvent(DWORD dwMana) {
  SingleArgHelper helper = {dwMana};
  sScriptEngine->ForEachScript(ManaEventCallback, &helper);
}

bool __fastcall KeyEventCallback(Script* script, void* argv) {
  KeyEventHelper* helper = (KeyEventHelper*)argv;
  const char* name = (helper->up ? "keyup" : "keydown");
  if (script->IsRunning() && script->GetListenerCount(name) > 0) {
    std::shared_ptr<KeyEvent> evt = std::make_shared<KeyEvent>();
    evt->name = name;
    evt->key = static_cast<uint32_t>(helper->key);

    script->DispatchEvent(evt);
  }
  bool block = false;
  name = (helper->up ? "keyupblocker" : "keydownblocker");
  if (script->IsRunning() && script->GetListenerCount(name) > 0) {
    std::shared_ptr<KeyEvent> evt = std::make_shared<KeyEvent>();
    evt->name = name;
    evt->key = static_cast<uint32_t>(helper->key);

    {
      ResetEvent(Vars.eventSignal);
      script->DispatchEvent(evt);
      if (WaitForSingleObject(Vars.eventSignal, 1000) == WAIT_TIMEOUT)
        return false;
    }

    block = evt->block;
  }

  return block;
}

bool KeyDownUpEvent(WPARAM key, BYTE bUp) {
  KeyEventHelper helper = {bUp, key};
  return sScriptEngine->ForEachScript(KeyEventCallback, &helper);
}

bool __fastcall PlayerAssignCallback(Script* script, void* argv) {
  SingleArgHelper* helper = (SingleArgHelper*)argv;
  if (script->IsRunning() && script->GetListenerCount("playerassign") > 0) {
    std::shared_ptr<PlayerAssignedEvent> evt = std::make_shared<PlayerAssignedEvent>();
    evt->name = "playerassign";
    evt->id = helper->arg1;

    script->DispatchEvent(evt);
  }
  return true;
}

void PlayerAssignEvent(DWORD dwUnitId) {
  SingleArgHelper helper = {dwUnitId};
  sScriptEngine->ForEachScript(PlayerAssignCallback, &helper);
}

bool __fastcall MouseClickCallback(Script* script, void* argv) {
  QuadArgHelper* helper = (QuadArgHelper*)argv;
  if (script->IsRunning() && script->GetListenerCount("mouseclick") > 0) {
    std::shared_ptr<MouseEvent> evt = std::make_shared<MouseEvent>();
    evt->name = "mouseclick";
    evt->x = helper->arg2;
    evt->y = helper->arg3;
    evt->button = helper->arg1;
    evt->state = helper->arg4;

    script->DispatchEvent(evt);
  }
  return true;
}

void MouseClickEvent(int button, POINT pt, bool bUp) {
  QuadArgHelper helper = {static_cast<DWORD>(button), static_cast<DWORD>(pt.x), static_cast<DWORD>(pt.y), bUp};
  sScriptEngine->ForEachScript(MouseClickCallback, &helper);
}

bool __fastcall MouseMoveCallback(Script* script, void* argv) {
  DoubleArgHelper* helper = (DoubleArgHelper*)argv;
  if (script->IsRunning() && script->GetListenerCount("mousemove") > 0) {
    std::shared_ptr<MouseEvent> evt = std::make_shared<MouseEvent>();
    evt->name = "mousemove";
    evt->x = helper->arg1;
    evt->y = helper->arg2;

    script->DispatchEvent(evt);
  }
  return true;
}

void MouseMoveEvent(POINT pt) {
  if (pt.x < 1 || pt.y < 1)
    return;
  DoubleArgHelper helper = {static_cast<DWORD>(pt.x), static_cast<DWORD>(pt.y)};
  sScriptEngine->ForEachScript(MouseMoveCallback, &helper);
}

bool ScriptMessageEvent(JSContext* ctx, Script* script, JSValue obj) {
  if (script && script->IsRunning() && script->GetListenerCount("scriptmsg") > 0) {
    uint8_t* data;
    size_t data_len;
    uint8_t** sab_tab;
    size_t sab_tab_len;

    data = JS_WriteObject2(ctx, &data_len, obj, JS_WRITE_OBJ_SAB | JS_WRITE_OBJ_REFERENCE, &sab_tab, &sab_tab_len);
    if (!data) {
      return false;
    }

    std::shared_ptr<ScriptMsgEvent> evt = std::make_shared<ScriptMsgEvent>();
    evt->name = "scriptmsg";

    // write data
    evt->data = static_cast<uint8_t*>(malloc(data_len));
    evt->data_len = data_len;
    if (!evt->data) {
      js_free(ctx, data);
      js_free(ctx, sab_tab);
      return false;
    }
    memcpy(evt->data, data, data_len);

    // write sab
    evt->sab_tab = static_cast<uint8_t**>(malloc(sizeof(uint8_t*) * sab_tab_len));
    if (!evt->sab_tab) {
      free(evt->data);
      js_free(ctx, data);
      js_free(ctx, sab_tab);
      return false;
    }
    memcpy(evt->sab_tab, sab_tab, sizeof(uint8_t*) * sab_tab_len);
    evt->sab_tab_len = sab_tab_len;

    // increase SAB reference counts
    for (size_t i = 0; i < evt->sab_tab_len; ++i) {
      js_sab_dup(NULL, evt->sab_tab[i]);
    }

    js_free(ctx, data);
    js_free(ctx, sab_tab);

    script->DispatchEvent(evt);
  }
  return true;
}

bool __fastcall BCastEventCallback(Script* script, void* argv) {
  BCastEventHelper* helper = (BCastEventHelper*)argv;
  return ScriptMessageEvent(helper->cx, script, helper->argv[0]);
}

void ScriptBroadcastEvent(JSContext* cx, JSValue* args) {
  BCastEventHelper helper = {cx, args};
  sScriptEngine->ForEachScript(BCastEventCallback, &helper);
}

bool __fastcall ChatEventCallback(Script* script, void* argv) {
  ChatEventHelper* helper = (ChatEventHelper*)argv;
  if (script->IsRunning() && script->GetListenerCount(helper->name) > 0) {
    std::shared_ptr<ChatMessageEvent> evt = std::make_shared<ChatMessageEvent>();
    evt->name = helper->name;
    evt->nickname = helper->nick ? helper->nick : "";
    evt->msg = helper->msg ? helper->msg : "";

    script->DispatchEvent(evt);
  }
  std::string evtname = helper->name;
  evtname = evtname + "blocker";
  bool block = false;

  if (script->IsRunning() && script->GetListenerCount(evtname.c_str()) > 0) {
    std::shared_ptr<ChatMessageEvent> evt = std::make_shared<ChatMessageEvent>();
    evt->name = evtname;
    evt->nickname = helper->nick ? helper->nick : "";
    evt->msg = helper->msg ? helper->msg : "";

    {
      ResetEvent(Vars.eventSignal);
      script->DispatchEvent(evt);
      if (WaitForSingleObject(Vars.eventSignal, 500) == WAIT_TIMEOUT)
        return false;
    }

    block = evt->block;
  }
  return block;
}

bool ChatEvent(const char* lpszNick, const char* lpszMsg) {
  ChatEventHelper helper = {"chatmsg", lpszNick, lpszMsg};
  return sScriptEngine->ForEachScript(ChatEventCallback, &helper);
}

bool ChatInputEvent(const char* lpszMsg) {
  ChatEventHelper helper = {"chatinput", "me", lpszMsg};
  return sScriptEngine->ForEachScript(ChatEventCallback, &helper);
}

bool WhisperEvent(const char* lpszNick, const char* lpszMsg) {
  ChatEventHelper helper = {"whispermsg", lpszNick, lpszMsg};
  return sScriptEngine->ForEachScript(ChatEventCallback, &helper);
}

bool __fastcall CopyDataCallback(Script* script, void* argv) {
  CopyDataHelper* helper = (CopyDataHelper*)argv;
  if (script->IsRunning() && script->GetListenerCount("copydata") > 0) {
    std::shared_ptr<CopyDataMessageEvent> evt = std::make_shared<CopyDataMessageEvent>();
    evt->name = "copydata";
    evt->mode = helper->mode;
    evt->msg = helper->msg ? helper->msg : "";

    script->DispatchEvent(evt);
  }
  return true;
}

void CopyDataEvent(DWORD dwMode, const char* lpszMsg) {
  CopyDataHelper helper = {dwMode, lpszMsg};
  sScriptEngine->ForEachScript(CopyDataCallback, &helper);
}

bool __fastcall ItemEventCallback(Script* script, void* argv) {
  ItemEventHelper* helper = (ItemEventHelper*)argv;
  if (script->IsRunning() && script->GetListenerCount("itemaction") > 0) {
    std::shared_ptr<ItemEvent> evt = std::make_shared<ItemEvent>();
    evt->name = "itemaction";
    evt->id = helper->id;
    evt->code = helper->code;
    evt->mode = helper->mode;
    evt->global = helper->global;

    script->DispatchEvent(evt);
  }
  return true;
}

void ItemActionEvent(DWORD GID, char* Code, BYTE Mode, bool Global) {
  ItemEventHelper helper = {GID, Code, Mode, Global};
  sScriptEngine->ForEachScript(ItemEventCallback, &helper);
}

bool __fastcall GameActionEventCallback(Script* script, void* argv) {
  GameActionEventHelper* helper = (GameActionEventHelper*)argv;
  if (script->IsRunning() && script->GetListenerCount("gameevent") > 0) {
    std::shared_ptr<GameEvent> evt = std::make_shared<GameEvent>();
    evt->name = "gameevent";
    evt->mode = helper->mode;
    evt->param1 = helper->param1;
    evt->param2 = helper->param2;
    evt->name1 = helper->name1 ? helper->name1 : "";
    evt->name2 = helper->name2 ? helper->name2 : "";

    script->DispatchEvent(evt);
  }
  return true;
}

void GameActionEvent(BYTE mode, DWORD param1, DWORD param2, const char* name1, const char* name2) {
  GameActionEventHelper helper = {mode, param1, param2, name1, name2};
  sScriptEngine->ForEachScript(GameActionEventCallback, &helper);
}

bool __fastcall PacketEventCallback(Script* script, void* argv) {
  PacketEventHelper* helper = (PacketEventHelper*)argv;

  if (script->IsRunning() && script->GetListenerCount(helper->name) > 0) {
    std::shared_ptr<PacketEvent> evt = std::make_shared<PacketEvent>();
    evt->name = helper->name;
    evt->bytes.resize(helper->dwSize);
    memcpy(evt->bytes.data(), helper->pPacket, helper->dwSize);

    // TODO(ejt): this is a mess but can't fix until scripting is single-threaded
    if (GetCurrentThreadId() == script->GetThreadId()) {
      script->HandleEvent(evt, false);
    } else {
      ResetEvent(Vars.eventSignal);
      script->DispatchEvent(evt);
      static DWORD result;
      if (Vars.bGameLoopEntered && Vars.dwGameThreadId == GetCurrentThreadId())
        LeaveCriticalSection(&Vars.cGameLoopSection);
      result = WaitForSingleObject(Vars.eventSignal, 500);
      if (Vars.bGameLoopEntered && Vars.dwGameThreadId == GetCurrentThreadId())
        EnterCriticalSection(&Vars.cGameLoopSection);

      if (result == WAIT_TIMEOUT)
        return false;
    }

    return evt->block;
  }

  return false;
}

bool GamePacketEvent(BYTE* pPacket, DWORD dwSize) {
  PacketEventHelper helper = {"gamepacket", pPacket, dwSize};
  return sScriptEngine->ForEachScript(PacketEventCallback, &helper);
}

bool GamePacketSentEvent(BYTE* pPacket, DWORD dwSize) {
  PacketEventHelper helper = {"gamepacketsent", pPacket, dwSize};
  return sScriptEngine->ForEachScript(PacketEventCallback, &helper);
}

bool RealmPacketEvent(BYTE* pPacket, DWORD dwSize) {
  PacketEventHelper helper = {"realmpacket", pPacket, dwSize};
  return sScriptEngine->ForEachScript(PacketEventCallback, &helper);
}

bool GenhookClickEvent(Script* script, int button, POINT* loc, JSValue func) {
  bool block = false;
  if (script && JS_IsFunction(script->GetContext(), func)) {
    std::shared_ptr<GenHookEvent> evt = std::make_shared<GenHookEvent>();
    evt->name = "ScreenHookClick";
    evt->x = loc->x;
    evt->y = loc->y;
    evt->button = button;
    evt->callback = JS_DupValue(script->GetContext(), func);

    {
      ResetEvent(Vars.eventSignal);
      script->DispatchEvent(evt);
      if (WaitForSingleObject(Vars.eventSignal, 3000) == WAIT_TIMEOUT)
        return false;
    }

    block = evt->block;
  }
  return block;
}

void GenhookHoverEvent(Script* script, POINT* loc, JSValue func) {
  if (script && JS_IsFunction(script->GetContext(), func)) {
    std::shared_ptr<GenHookEvent> evt = std::make_shared<GenHookEvent>();
    evt->name = "ScreenHookHover";
    evt->x = loc->x;
    evt->y = loc->y;
    evt->callback = JS_DupValue(script->GetContext(), func);

    script->DispatchEvent(evt);
  }
}
