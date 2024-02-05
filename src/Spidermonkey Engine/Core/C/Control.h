#pragma once

#include "Game/D2WinControl.h"

D2WinControlStrc* findControl(int Type, int LocaleID, int Disabled, int PosX, int PosY, int SizeX, int SizeY);
D2WinControlStrc* findControl(int Type, const char* Text, int Disabled, int PosX, int PosY, int SizeX, int SizeY);
bool clickControl(D2WinControlStrc* pControl, int x = -1, int y = -1);
void setControlText(D2WinControlStrc* pControl, const char* szText);
bool OOG_CreateCharacter(const char* szCharacter, int type, bool hardcore, bool ladder);
bool OOG_SelectCharacter(const char* szCharacter);
bool OOG_SelectGateway(const char* szGateway, size_t strSize);
void SetControlText(D2WinControlStrc* pControl, const char* Text);
OOG_Location OOG_GetLocation(void);
bool OOG_CreateGame(const char* name, const char* pass, int difficulty);
bool OOG_JoinGame(const char* name, const char* pass);
