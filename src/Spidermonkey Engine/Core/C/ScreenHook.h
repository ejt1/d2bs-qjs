#pragma once

#include <list>
#include <algorithm>
#include <windows.h>

#include "Script.h"
#include "ScriptEngine.h"
#include "D2Helpers.h"

void DrawLogo(void);

typedef unsigned short ushort;

class Genhook;

typedef std::list<Genhook*> HookList;
typedef HookList::iterator HookIterator;

struct HookClickHelper {
  int button;
  POINT point;
};

typedef bool(__fastcall* HookCallback)(Genhook*, void*, uint32_t);

enum Align { Left, Right, Center };
enum ScreenhookState { OOG, IG, Perm };

bool __fastcall DrawHook(Genhook* hook, void* argv, uint32_t argc);
bool __fastcall CleanHook(Genhook* hook, void* argv, uint32_t argc);
bool __fastcall ClickHook(Genhook* hook, void* argv, uint32_t argc);
bool __fastcall HoverHook(Genhook* hook, void* argv, uint32_t argc);

class Genhook {
 private:
  static bool init;
  static HookList visible, invisible;
  static CRITICAL_SECTION globalSection;

 protected:
  Script* owner;
  ScreenhookState gameState;
  Align alignment;
  JS::PersistentRootedObject clicked, hovered;
  JS::PersistentRootedObject self;
  bool isAutomap, isVisible;
  ushort opacity, zorder;
  POINT location;
  // CRITICAL_SECTION hookSection;

  Genhook(const Genhook&) = delete;
  Genhook& operator=(const Genhook&) = delete;

 public:
  Genhook(Script* nowner, JS::HandleObject nself, uint32_t x, uint32_t y, ushort nopacity, bool nisAutomap = false, Align nalign = Left, ScreenhookState ngameState = Perm);
  virtual ~Genhook(void);

  friend bool __fastcall DrawHook(Genhook* hook, void* argv, uint32_t argc);
  friend bool __fastcall CleanHook(Genhook* hook, void* argv, uint32_t argc);
  friend bool __fastcall ClickHook(Genhook* hook, void* argv, uint32_t argc);
  friend bool __fastcall HoverHook(Genhook* hook, void* argv, uint32_t argc);

  static void DrawAll(ScreenhookState type);

  static bool ForEachHook(HookCallback proc, void* argv, uint32_t argc);
  static bool ForEachVisibleHook(HookCallback proc, void* argv, uint32_t argc);
  static bool ForEachInvisibleHook(HookCallback proc, void* argv, uint32_t argc);
  static bool ForEachVisibleHookUnLocked(HookCallback proc, void* argv, uint32_t argc);
  static void Clean(Script* owner);
  static void Initialize(void) {
    InitializeCriticalSection(&globalSection);
    init = true;
  }
  static void Destroy(void) {
    init = false;
    DeleteCriticalSection(&globalSection);
  }

 protected:
  virtual void Draw(void) = 0;

 public:
  bool Click(int button, POINT* loc);
  void Hover(POINT* loc);

  bool IsInRange(POINT* pt) {
    return IsInRange(pt->x, pt->y);
  }
  virtual bool IsInRange(int dx, int dy) = 0;

  void Move(POINT* dist) {
    Move(dist->x, dist->y);
  }
  void Move(uint32_t nx, uint32_t ny) {
    SetX(GetX() + nx);
    SetY(GetY() + ny);
  }

  void SetX(uint32_t x) {
    Lock();
    location.x = x;
    Unlock();
  }
  void SetY(uint32_t y) {
    Lock();
    location.y = y;
    Unlock();
  }
  void SetAlign(Align nalign) {
    Lock();
    alignment = nalign;
    Unlock();
  }
  void SetOpacity(ushort nopacity) {
    Lock();
    opacity = nopacity;
    Unlock();
  }
  void SetIsVisible(bool nisVisible) {
    Lock();
    EnterCriticalSection(&globalSection);
    if (!nisVisible) {
      if (isVisible) {
        visible.remove(this);
        invisible.push_back(this);
      }
    } else {
      if (!isVisible) {
        invisible.remove(this);
        visible.push_back(this);
      }
    }
    isVisible = nisVisible;
    Unlock();
    LeaveCriticalSection(&globalSection);
  }
  void SetZOrder(ushort norder) {
    Lock();
    zorder = norder;
    Unlock();
  }
  void SetClickHandler(JS::HandleObject handler);
  void SetHoverHandler(JS::HandleObject handler);

  POINT GetLocation(void) const {
    return location;
  }
  uint32_t GetX(void) const {
    return location.x;
  }
  uint32_t GetY(void) const {
    return location.y;
  }
  Align GetAlign(void) const {
    return alignment;
  }
  ushort GetOpacity(void) const {
    return opacity;
  }
  ScreenhookState GetGameState(void) const {
    return gameState;
  }
  bool GetIsAutomap(void) const {
    return isAutomap;
  }
  bool GetIsVisible(void) const {
    return isVisible;
  }
  ushort GetZOrder(void) const {
    return zorder;
  }
  JSObject* GetClickHandler(void) {
    return clicked;
  }
  JSObject* GetHoverHandler(void) {
    return hovered;
  }

  static void EnterGlobalSection() {
    EnterCriticalSection(&globalSection);
  }
  static void LeaveGlobalSection() {
    LeaveCriticalSection(&globalSection);
  }

  void Lock() { /* EnterCriticalSection(&hookSection);*/
  }
  void Unlock() { /* LeaveCriticalSection(&hookSection);*/
  }
};

class TextHook : public Genhook {
 private:
  wchar_t* text;
  ushort font, color;

  TextHook(const TextHook&);
  TextHook& operator=(const TextHook&);

 public:
  TextHook(Script* owner, JS::HandleObject nself, const wchar_t* text, uint32_t x, uint32_t y, ushort nfont, ushort ncolor, bool automap = false, Align align = Left,
           ScreenhookState state = Perm)
      : Genhook(owner, nself, x, y, 0, automap, align, state), text(NULL), font(nfont), color(ncolor) {
    this->text = _wcsdup(text);
  }
  ~TextHook(void) {
    free(text);
  }

 protected:
  void Draw(void);

 public:
  bool IsInRange(int dx, int dy);

  void SetFont(ushort nfont) {
    Lock();
    font = nfont;
    Unlock();
  }
  void SetColor(ushort ncolor) {
    Lock();
    color = ncolor;
    Unlock();
  }
  void SetText(const wchar_t* ntext);

  ushort GetFont(void) const {
    return font;
  }
  ushort GetColor(void) const {
    return color;
  }
  const wchar_t* GetText(void) const {
    return text;
  }
};

class ImageHook : public Genhook {
 private:
  wchar_t* location;
  D2CellFileStrc* image;
  ushort color;

  ImageHook(const ImageHook&);
  ImageHook& operator=(const ImageHook&);

 public:
  ImageHook(Script* owner, JS::HandleObject nself, const wchar_t* nloc, uint32_t x, uint32_t y, ushort ncolor, bool automap = false, Align align = Left,
            ScreenhookState state = Perm,
            bool fromFile = true)
      : Genhook(owner, nself, x, y, 0, automap, align, state), color(ncolor), image(NULL), location(NULL) {
    (fromFile);  // unreferenced formal parameter

    location = _wcsdup(nloc);
    image = LoadCellFile(location, 3);
  }
  ~ImageHook(void) {
    free(location);
    delete[] image;
  }

 protected:
  void Draw(void);

 public:
  bool IsInRange(int dx, int dy);

  void SetImage(const wchar_t* nimage);
  void SetColor(ushort ncolor) {
    Lock();
    color = ncolor;
    Unlock();
  }

  const wchar_t* GetImage(void) const {
    return location;
  }
  ushort GetColor(void) const {
    return color;
  }
};

class LineHook : public Genhook {
 private:
  uint32_t x2, y2;
  ushort color;

  LineHook(const LineHook&);
  LineHook& operator=(const LineHook&);

 public:
  LineHook(Script* owner, JS::HandleObject nself, uint32_t x, uint32_t y, uint32_t nx2, uint32_t ny2, ushort ncolor, bool automap = false, Align align = Left,
           ScreenhookState state = Perm)
      : Genhook(owner, nself, x, y, 0, automap, align, state), x2(nx2), y2(ny2), color(ncolor) {
  }
  ~LineHook(void) {
  }

 protected:
  void Draw(void);

 public:
  bool IsInRange(int dx, int dy) {
    (dx);  // unreferenced formal parameter
    (dy);  // unreferenced formal parameter

    return false;
  }

  void SetX2(uint32_t nx2) {
    Lock();
    x2 = nx2;
    Unlock();
  }
  void SetY2(uint32_t ny2) {
    Lock();
    y2 = ny2;
    Unlock();
  }
  void SetColor(ushort ncolor) {
    Lock();
    color = ncolor;
    Unlock();
  }

  uint32_t GetX2(void) const {
    return x2;
  }
  uint32_t GetY2(void) const {
    return y2;
  }
  ushort GetColor(void) const {
    return color;
  }
};

class BoxHook : public Genhook {
 private:
  uint32_t xsize, ysize;
  ushort color;

  BoxHook(const BoxHook&);
  BoxHook& operator=(const BoxHook&);

 public:
  BoxHook(Script* owner, JS::HandleObject nself, uint32_t x, uint32_t y, uint32_t nxsize, uint32_t nysize, ushort ncolor, ushort opacity, bool automap = false, Align align = Left,
          ScreenhookState state = Perm)
      : Genhook(owner, nself, x, y, opacity, automap, align, state), xsize(nxsize), ysize(nysize), color(ncolor) {
  }
  ~BoxHook(void) {
  }

 protected:
  void Draw(void);

 public:
  bool IsInRange(int dx, int dy);

  void SetXSize(uint32_t nxsize) {
    Lock();
    xsize = nxsize;
    Unlock();
  }
  void SetYSize(uint32_t nysize) {
    Lock();
    ysize = nysize;
    Unlock();
  }
  void SetColor(ushort ncolor) {
    Lock();
    color = ncolor;
    Unlock();
  }

  uint32_t GetXSize(void) const {
    return xsize;
  }
  uint32_t GetYSize(void) const {
    return ysize;
  }
  ushort GetColor(void) const {
    return color;
  }
};

class FrameHook : public Genhook {
 private:
  uint32_t xsize, ysize;

  FrameHook(const FrameHook&);
  FrameHook& operator=(const FrameHook&);

 public:
  FrameHook(Script* owner, JS::HandleObject nself, uint32_t x, uint32_t y, uint32_t nxsize, uint32_t nysize, bool automap = false, Align align = Left,
            ScreenhookState state = Perm)
      : Genhook(owner, nself, x, y, 0, automap, align, state), xsize(nxsize), ysize(nysize) {
  }
  ~FrameHook(void) {
  }

 protected:
  void Draw(void);

 public:
  bool IsInRange(int dx, int dy);

  void SetXSize(uint32_t nxsize) {
    Lock();
    xsize = nxsize;
    Unlock();
  }
  void SetYSize(uint32_t nysize) {
    Lock();
    ysize = nysize;
    Unlock();
  }

  uint32_t GetXSize(void) const {
    return xsize;
  }
  uint32_t GetYSize(void) const {
    return ysize;
  }
};
