#pragma once

#include "js32.h"

bool my_login(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_selectChar(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_createGame(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_joinGame(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_addProfile(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_getOOGLocation(JSContext* ctx, unsigned argc, JS::Value* vp);
bool my_createCharacter(JSContext* ctx, unsigned argc, JS::Value* vp);
