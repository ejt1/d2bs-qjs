#pragma once

#include "js32.h"

bool my_login(JSContext* ctx, JS::CallArgs& args);
bool my_selectChar(JSContext* ctx, JS::CallArgs& args);
bool my_createGame(JSContext* ctx, JS::CallArgs& args);
bool my_joinGame(JSContext* ctx, JS::CallArgs& args);
bool my_addProfile(JSContext* ctx, JS::CallArgs& args);
bool my_getOOGLocation(JSContext* ctx, JS::CallArgs& args);
bool my_createCharacter(JSContext* ctx, JS::CallArgs& args);
