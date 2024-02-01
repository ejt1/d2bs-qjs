#include "ScreenHook.h"
#include "JSScreenHook.h"
#include "D2BS.h"
#include "Console.h"
#include "D2Ptrs.h"

#include <vector>

bool Genhook::init = false;
HookList Genhook::visible = HookList();
HookList Genhook::invisible = HookList();
CRITICAL_SECTION Genhook::globalSection = {0};

void DrawLogo(void) {
  static wchar_t version[] = L"D2BS " D2BS_VERSION;
  static int len = CalculateTextLen(version, 0).x;
  int dx = GetScreenSize().x - len - 1;
  int dy = GetScreenSize().y - 1;
  myDrawText(version, dx, dy, 4, 0);
}

bool zOrderSort(Genhook* first, Genhook* second) {
  return first->GetZOrder() < second->GetZOrder();
}

bool __fastcall HoverHook(Genhook* hook, void* argv, uint /*argc*/) {
  HookClickHelper* helper = (HookClickHelper*)argv;
  hook->Hover(&helper->point);
  return true;
}

bool __fastcall ClickHook(Genhook* hook, void* argv, uint /*argc*/) {
  HookClickHelper* helper = (HookClickHelper*)argv;
  return hook->Click(helper->button, &helper->point);
}

bool __fastcall DrawHook(Genhook* hook, void* argv, uint /*argc*/) {
  if ((hook->GetGameState() == (ScreenhookState)(int)argv || hook->GetGameState() == Perm) && (!hook->GetIsAutomap() || (hook->GetIsAutomap() && *p_D2CLIENT_AutomapOn)))
    hook->Draw();
  return true;
}

bool __fastcall CleanHook(Genhook* hook, void* argv, uint /*argc*/) {
  if (hook->owner == (Script*)argv)
    hook->SetIsVisible(false);
  return true;
}

void Genhook::DrawAll(ScreenhookState type) {
  if (!init)
    return;
  ForEachVisibleHook(DrawHook, (void*)type, 1);
}

bool Genhook::ForEachHook(HookCallback proc, void* argv, uint argc) {
  // iterate the visible ones, then the invisible ones
  EnterCriticalSection(&globalSection);

  bool result = false;
  bool result2 = false;
  std::vector<Genhook*> list;
  for (HookIterator it = visible.begin(); it != visible.end(); it++) list.push_back(*it);
  int count = list.size();

  for (int i = 0; i < count; i++)
    if (proc(list[i], argv, argc))
      result = true;

  // if(!result)
  //{
  list.clear();
  for (HookIterator it = invisible.begin(); it != invisible.end(); it++) list.push_back(*it);
  count = list.size();

  for (int i = 0; i < count; i++)
    if (proc(list[i], argv, argc))
      result2 = true;
  //}

  LeaveCriticalSection(&globalSection);
  return (result ? true : (result2 ? true : false));
}

bool Genhook::ForEachVisibleHook(HookCallback proc, void* argv, uint argc) {
  // iterate the visible hooks
  EnterCriticalSection(&globalSection);

  bool result = false;
  std::vector<Genhook*> list;
  for (HookIterator it = visible.begin(); it != visible.end(); it++) list.push_back(*it);

  std::sort(list.begin(), list.end(), zOrderSort);
  int count = list.size();
  for (int i = 0; i < count; i++)
    if (proc(list[i], argv, argc))
      result = true;

  LeaveCriticalSection(&globalSection);
  return result;
}
bool Genhook::ForEachVisibleHookUnLocked(HookCallback proc, void* argv, uint argc) {
  // iterate the visible hooks  //unlocked to call funcs, locked to draw
  EnterCriticalSection(&globalSection);

  bool result = false;
  std::vector<Genhook*> list;
  for (HookIterator it = visible.begin(); it != visible.end(); it++) list.push_back(*it);

  LeaveCriticalSection(&globalSection);

  std::sort(list.begin(), list.end(), zOrderSort);
  int count = list.size();
  for (int i = 0; i < count; i++)
    if (proc(list[i], argv, argc))
      result = true;

  return result;
}
bool Genhook::ForEachInvisibleHook(HookCallback proc, void* argv, uint argc) {
  // iterate the invisible hooks
  EnterCriticalSection(&globalSection);

  bool result = false;
  std::vector<Genhook*> list;
  for (HookIterator it = invisible.begin(); it != invisible.end(); it++) list.push_back(*it);
  int count = list.size();

  for (int i = 0; i < count; i++)
    if (proc(list[i], argv, argc))
      result = true;

  LeaveCriticalSection(&globalSection);
  return result;
}

void Genhook::Clean(Script* owner) {
  if (!init)
    return;

  // ForEachHook(CleanHook, owner, 1);
  EnterCriticalSection(&globalSection);

  HookIterator it = visible.begin();
  while (it != visible.end()) {
    if ((*it)->owner->IsAborted()) {
      // Genhook* i = *it;
      it = invisible.erase(it);
      //	delete(i);
    } else
      it++;
  }

  it = invisible.begin();
  while (it != invisible.end()) {
    if ((*it)->owner == owner) {
      // Genhook* i = *it;
      it = invisible.erase(it);
      // delete(i);

    } else
      it++;
  }
  LeaveCriticalSection(&globalSection);
}

Genhook::Genhook(Script* nowner, JSObject* nself, uint x, uint y, ushort nopacity, bool nisAutomap, Align nalign, ScreenhookState ngameState)
    : owner(nowner), isAutomap(nisAutomap), isVisible(true), alignment(nalign), opacity(nopacity), gameState(ngameState), zorder(1) {
  // InitializeCriticalSection(&hookSection);
  clicked = JSVAL_VOID;
  hovered = JSVAL_VOID;
  self = nself;
  // JS_AddObjectRoot(owner->GetContext(),&self);
  // JS_AddRoot(&self);
  SetX(x);
  SetY(y);
  EnterCriticalSection(&globalSection);
  visible.push_back(this);
  LeaveCriticalSection(&globalSection);
}

Genhook::~Genhook(void) {
  Lock();
  // if(owner && !JSVAL_IS_FUNCTION(owner->GetContext(), clicked))
  //	JS_RemoveRoot(owner->GetContext(), &clicked);
  // if(owner && !JSVAL_IS_FUNCTION(owner->GetContext(), hovered))
  //	JS_RemoveRoot(owner->GetContext(), &hovered);
  // JS_RemoveObjectRoot(owner->GetContext(), &self);
  // JS_RemoveRoot(&self);

  EnterCriticalSection(&globalSection);

  owner = NULL;
  location.x = -1;
  location.y = -1;
  if (isVisible)
    visible.remove(this);
  else
    invisible.remove(this);

  LeaveCriticalSection(&globalSection);

  Unlock();
  // DeleteCriticalSection(&hookSection);
}

bool Genhook::Click(int button, POINT* loc) {
  if (!IsInRange(loc))
    return false;

  bool block = false;
  if (owner && JSVAL_IS_FUNCTION(owner->GetContext(), clicked)) {
    Event* evt = new Event;
    //evt->owner = owner;
    evt->argc = 3;
    evt->name = "ScreenHookClick";
    evt->arg1 = new DWORD((DWORD)button);
    evt->arg2 = new DWORD((DWORD)loc->x);
    evt->arg3 = new DWORD((DWORD)loc->y);
    evt->arg4 = new DWORD(false);

    ResetEvent(Vars.eventSignal);
    AutoRoot* root = new AutoRoot(owner->GetContext(), clicked);
    evt->functions.push_back(root);
    owner->FireEvent(evt);

    if (WaitForSingleObject(Vars.eventSignal, 3000) == WAIT_TIMEOUT)
      return false;
    bool* global = (bool*)evt->arg4;
    block = *global;
    delete evt->arg1;
    delete evt->arg2;
    delete evt->arg3;
    delete evt->arg4;
    delete root;
    delete evt;
  }
  return block;
}

void Genhook::Hover(POINT* loc) {
  if (!IsInRange(loc))
    return;

  if (owner && JSVAL_IS_FUNCTION(owner->GetContext(), hovered)) {
    Event* evt = new Event;
    //evt->owner = owner;
    evt->argc = 2;
    evt->functions.push_back(new AutoRoot(owner->GetContext(), hovered));
    evt->name = "ScreenHookHover";
    evt->arg1 = new DWORD((DWORD)loc->x);
    evt->arg2 = new DWORD((DWORD)loc->y);

    owner->FireEvent(evt);
  }
}
void Genhook::SetClickHandler(jsval handler) {
  if (!owner)
    return;
  if (JSVAL_IS_VOID(handler))
    return;
  Lock();

  //JSContext* cx = owner->GetContext();
  // if(JSVAL_IS_FUNCTION(cx, handler))
  //	JS_RemoveRoot(owner->GetContext(), &clicked);
  // JS_SetContextThread(cx);
  // JS_BeginRequest(cx);

  clicked = handler;
  //	if(!JSVAL_IS_VOID(clicked))
  //	{
  //		if(JS_AddRoot(owner->GetContext(),&clicked) == JS_FALSE)
  //		{
  //			Unlock();
  //			JS_EndRequest(cx);
  ////bob1.8.8			JS_ClearContextThread(cx);
  //			return;
  //		}
  //	}
  //	JS_EndRequest(cx);
  // JS_ClearContextThread(cx);
  Unlock();
}

void Genhook::SetHoverHandler(jsval handler) {
  if (!owner)
    return;
  if (JSVAL_IS_VOID(handler))
    return;
  Lock();
  if (!JSVAL_IS_VOID(hovered))
    JS_RemoveRoot(owner->GetContext(), &hovered);
  if (JSVAL_IS_FUNCTION(owner->GetContext(), handler))
    hovered = handler;
  if (!JSVAL_IS_VOID(hovered)) {
    if (JS_AddRoot(owner->GetContext(), &hovered) == JS_FALSE) {
      Unlock();
      return;
    }
  }
  Unlock();
}

void TextHook::Draw(void) {
  Lock();
  if (GetIsVisible() && GetX() != -1 && GetY() != -1 && text) {
    uint x = GetX(), y = GetY(), w = CalculateTextLen(text, font).x;
    x -= (alignment != Center ? (alignment != Right ? 0 : w) : w / 2);
    POINT loc = {static_cast<LONG>(x), static_cast<LONG>(y)};
    if (GetIsAutomap()) {
      loc = ScreenToAutomap(x, y);
    }
    EnterCriticalSection(&Vars.cTextHookSection);
    myDrawText((const wchar_t*)text, loc.x, loc.y, color, font);
    LeaveCriticalSection(&Vars.cTextHookSection);
  }
  Unlock();
}

bool TextHook::IsInRange(int dx, int dy) {
  Lock();
  POINT size = CalculateTextLen(text, font);
  int x = GetX(), y = GetY(), w = size.x, h = size.y, xp = x - (alignment != Center ? (alignment != Right ? 0 : w) : w / 2);
  Unlock();
  return (xp < dx && y > dy && (xp + w) > dx && (y - h) < dy);
}

void TextHook::SetText(const wchar_t* ntext) {
  EnterGlobalSection();
  free(text);
  text = NULL;
  if (ntext)
    text = _wcsdup(ntext);
  LeaveGlobalSection();
}

void ImageHook::Draw(void) {
  Lock();
  if (GetIsVisible() && GetX() != -1 && GetY() != -1 && GetImage() != NULL && image != NULL) {
    uint x = GetX(), y = GetY(), w = image->cells[0]->width;
    x += (alignment != Left ? (alignment != Right ? 0 : -1 * (w / 2)) : w / 2);
    POINT loc = {static_cast<LONG>(x), static_cast<LONG>(y)};
    if (GetIsAutomap()) {
      loc = ScreenToAutomap(x, y);
    }
    EnterCriticalSection(&Vars.cImageHookSection);
    myDrawAutomapCell(image, loc.x, loc.y, (BYTE)color);
    LeaveCriticalSection(&Vars.cImageHookSection);
  }
  Unlock();
}

bool ImageHook::IsInRange(int dx, int dy) {
  if (image) {
    Lock();
    int x = GetX();
    int y = GetY();
    int w = image->cells[0]->width;
    int h = image->cells[0]->height;
    int xp = x - (alignment != Left ? (alignment != Right ? w / 2 : w) : -1 * w);
    int yp = y - (h / 2);
    Unlock();
    return (xp < dx && yp < dy && (xp + w) > dx && (yp + h) > dy);
  }

  return false;
}

void ImageHook::SetImage(const wchar_t* nimage) {
  Lock();
  free(location);
  delete[] image;

  location = _wcsdup(nimage);
  image = LoadCellFile(location);
  Unlock();
}

void LineHook::Draw(void) {
  Lock();
  if (GetIsVisible() && GetX() != -1 && GetY() != -1) {
    uint x = GetX(), y = GetY(), _x2 = GetX2(), _y2 = GetY2();
    POINT loc = {static_cast<LONG>(x), static_cast<LONG>(y)};
    POINT sz = {static_cast<LONG>(_x2), static_cast<LONG>(_y2)};
    if (GetIsAutomap()) {
      loc = ScreenToAutomap(x, y);
      sz = ScreenToAutomap(_x2, _y2);
    }
    EnterCriticalSection(&Vars.cLineHookSection);
    D2GFX_DrawLine(loc.x, loc.y, sz.x, sz.y, color, 0xFF);
    LeaveCriticalSection(&Vars.cLineHookSection);
  }
  Unlock();
}

void BoxHook::Draw(void) {
  Lock();
  if (GetIsVisible() && GetX() != -1 && GetY() != -1) {
    uint x = GetX(), y = GetY(), x2 = GetXSize(), y2 = GetYSize();
    if (alignment == Center) {
      x -= x2 / 2;
    } else if (alignment == Right) {
      x += x2 / 2;
    }
    POINT loc = {static_cast<LONG>(x), static_cast<LONG>(y)};
    POINT sz = {static_cast<LONG>(x + x2), static_cast<LONG>(y + y2)};
    if (GetIsAutomap()) {
      loc = ScreenToAutomap(x, y);
      sz = ScreenToAutomap(x + x2, y + y2);
    }
    EnterCriticalSection(&Vars.cBoxHookSection);
    D2GFX_DrawRectangle(loc.x, loc.y, sz.x, sz.y, color, opacity);
    LeaveCriticalSection(&Vars.cBoxHookSection);
  }
  Unlock();
}

bool BoxHook::IsInRange(int dx, int dy) {
  Lock();
  int x = GetX(), y = GetY(), x2 = GetXSize(), y2 = GetYSize();
  Unlock();
  return (x < dx && y < dy && (x + x2) > dx && (y + y2) > dy);
}

void FrameHook::Draw(void) {
  Lock();
  if (GetIsVisible() && GetX() != -1 && GetY() != -1) {
    uint x = GetX(), y = GetY(), x2 = GetXSize(), y2 = GetYSize();
    if (alignment == Center) {
      x -= x2 / 2;
    } else if (alignment == Right) {
      x += x2 / 2;
    }
    RECT rect = {static_cast<LONG>(x), static_cast<LONG>(y), static_cast<LONG>(x + x2), static_cast<LONG>(y + y2)};
    EnterCriticalSection(&Vars.cFrameHookSection);
    D2GFX_DrawFrame(&rect);
    LeaveCriticalSection(&Vars.cFrameHookSection);
  }
  Unlock();
}

bool FrameHook::IsInRange(int dx, int dy) {
  Lock();
  int x = GetX(), y = GetY(), x2 = GetXSize(), y2 = GetYSize();
  Unlock();
  return (x < dx && y < dy && (x + x2) > dx && (y + y2) > dy);
}
