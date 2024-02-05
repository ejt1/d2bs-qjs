#pragma once

#include <cstdint>

///////////////////////////////////////////////////
// Out of Game Locations (From ControlInfo.dbl)
///////////////////////////////////////////////////
enum OOG_Location {
  OOG_NONE = 0,
  OOG_LOBBY,
  OOG_INLINE,
  OOG_CHAT,
  OOG_CREATE,
  OOG_JOIN,
  OOG_LADDER,
  OOG_CHANNEL,
  OOG_MAIN_MENU,
  OOG_LOGIN,
  OOG_LOGIN_ERROR,
  OOG_UNABLE_TO_CONNECT,
  OOG_CHAR_SELECT,
  OOG_REALM_DOWN,
  OOG_DISCONNECTED,
  OOG_NEW_CHARACTER,
  OOG_CHARACTER_SELECT_PLEASE_WAIT,
  OOG_LOST_CONNECTION,
  OOG_D2SPLASH,
  OOG_CDKEY_IN_USE,
  OOG_DIFFICULTY,
  OOG_MAIN_MENU_CONNECTING,
  OOG_INVALID_CDKEY,
  OOG_CONNECTING,
  OOG_SERVER_DOWN,
  OOG_PLEASE_WAIT,
  OOG_GAME_EXIST,
  OOG_GATEWAY,
  OOG_GAME_DOES_NOT_EXIST,
  OOG_CHARACTER_CREATE,
  OOG_CHARACTER_CREATE_ALREADY_EXISTS,
  OOG_AGREE_TO_TERMS,
  OOG_NEW_ACCOUNT,
  OOG_PLEASE_READ,
  OOG_REGISTER_EMAIL,
  OOG_CREDITS,
  OOG_CINEMATICS,
  OOG_CHARACTOR_CHANGE_REALM,
  OOG_GAME_IS_FULL,
  OOG_OTHER_MULTIPLAYER,
  OOG_TCP_IP,
  OOG_ENTER_IP_ADDRESS,
  OOG_CHARACTER_SELECT_NO_CHARS,
  OOG_CHARACTER_SELECT_CHANGE_REALM,
  OOG_UNABLE_TO_CONNECT_TCPIP
};

// rename D2WinTextBoxLineStrc
struct D2WinTextBoxLineStrc {  // size = 0x20
  wchar_t* wText[5];            // 0x00 for each field
  uint32_t dwColor;             // 0x14
  uint32_t dwAlign;             // 0x18
  D2WinTextBoxLineStrc* pNext;  // 0x1C
};

struct D2WinControlStrc {
  uint32_t dwType;      // 0x00
  uint32_t* _1;         // 0x04 // unsure? definitely a ptr but not obvious, usually points to 6 when dwType is 6 I think
  uint32_t dwDisabled;  // 0x08
  uint32_t dwPosX;      // 0x0C
  uint32_t dwPosY;      // 0x10
  uint32_t dwSizeX;     // 0x14
  uint32_t dwSizeY;     // 0x18
  // I think _2 thru _9 are a handler table of some sort
  uint32_t* _2;                         // 0x1C // some sort of function (maybe click?)
  uint32_t _3;                          // 0x20
  uint32_t* _4;                         // 0x24 // some sort of function
  uint32_t* _5;                         // 0x28
  uint32_t _6;                          // 0x2C
  uint32_t* _7;                         // 0x30 // ptr to something...
  uint32_t* _8;                         // 0x34 // another random ptr... mostly dead ends when I examined them
  uint32_t _9;                          // 0x38
  D2WinControlStrc* pNext;              // 0x3C
  uint32_t _10;                         // 0x40
  uint32_t unkState;                    // 0x44 _11 0 when button avail to be clicked 1 when greyed - still need to look at this more
  D2WinTextBoxLineStrc* pFirstText;     // 0x48
  D2WinTextBoxLineStrc* pLastText;      // 0x4C
  D2WinTextBoxLineStrc* pSelectedText;  // 0x50
  uint32_t dwSelectEnd;                 // 0x54
  uint32_t dwSelectStart;               // 0x58
  union {
    // TODO(ejt): make this into D2WinTextBoxStrc look at D2Moo to figure it out
    struct {               // Textboxes
      wchar_t wText[256];  // 0x5C
      uint32_t dwCursorPos;
      uint32_t dwIsCloaked;
    } TextBox;
    // TODO(ejt): make this into D2WinButtonStrc look at D2Moo to figure it out
    struct {                // Buttons
      uint32_t _12[2];      // 0x5C
      wchar_t wText2[256];  // 0x6C
    } Button;
  };
};

inline char** D2MULTI_ChatBoxMsg = nullptr;
inline D2WinControlStrc** D2WIN_FirstControl = nullptr;
inline D2WinControlStrc** D2WIN_FocusedControl = nullptr;
inline D2WinControlStrc** D2MULTI_GameListControl = nullptr;

inline void*(__fastcall* D2WIN_SetControlText)(D2WinControlStrc* box, const wchar_t* txt) = nullptr;
inline uint32_t(__fastcall* D2WIN_DestroyEditBox)(D2WinControlStrc* box) = nullptr;
inline void(__stdcall* D2WIN_DestroyControl)(D2WinControlStrc* pControl) = nullptr;
inline void(__fastcall* D2WIN_SetEditBoxCallback)(D2WinControlStrc* pControl,
                                                  int(__stdcall* FunCallBack)(D2WinControlStrc* pControl, uint32_t dwInputType, char* pChar)) = nullptr;
inline void(__fastcall* D2WIN_SetEditBoxProc)(D2WinControlStrc* box, int(__stdcall* FunCallBack)(D2WinControlStrc*, uint32_t, uint32_t)) = nullptr;
inline void(__fastcall* D2WIN_SelectEditBoxText)(D2WinControlStrc* box) = nullptr;