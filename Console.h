#pragma once
#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <deque>
#include <string>
#include <sstream>
#include <windows.h>

#include "D2BS.h"

class Console {
 private:
  static bool visible, enabled;
  static std::deque<std::wstring> lines, commands, history;
  static unsigned int lineCount, lineWidth, commandPos, height, scrollIndex;
  static std::wstringstream cmd;

 public:
  static void Toggle(void);
  static void TogglePrompt(void);
  static void ToggleBuffer(void);
  static void Hide(void);
  static void HidePrompt(void);
  static void HideBuffer(void);
  static void Show(void);
  static void ShowPrompt(void);
  static void ShowBuffer(void);
  static bool IsVisible(void) {
    return visible;
  }
  static bool IsEnabled(void) {
    return enabled;
  }
  static unsigned int MaxWidth(void) {
    return lineWidth;
  }
  static unsigned int Console::GetHeight(void) {
    return height;
  }

  static void AddKey(unsigned int key);
  static void ExecuteCommand(void);
  static void RemoveLastKey(void);
  static void PrevCommand(void);
  static void NextCommand(void);
  static void ScrollUp(void);
  static void ScrollDown(void);
  static void AddLine(std::wstring line);
  static void UpdateLines(void);
  static void Clear(void);
  static void Draw(void);
};

#endif
