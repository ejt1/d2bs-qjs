#pragma once

#include "Game/Unorganized.h"

Control* findControl(int Type, int LocaleID, int Disabled, int PosX, int PosY, int SizeX, int SizeY);
Control* findControl(int Type, const char* Text, int Disabled, int PosX, int PosY, int SizeX, int SizeY);
bool clickControl(Control* pControl, int x = -1, int y = -1);
void setControlText(Control* pControl, const char* szText);
BOOL OOG_CreateCharacter(const char* szCharacter, int type, bool hardcore, bool ladder);
BOOL OOG_SelectCharacter(const char* szCharacter);
BOOL OOG_SelectGateway(const char* szGateway, size_t strSize);
void SetControlText(Control* pControl, const char* Text);
OOG_Location OOG_GetLocation(void);
bool OOG_CreateGame(const char* name, const char* pass, int difficulty);
bool OOG_JoinGame(const char* name, const char* pass);
