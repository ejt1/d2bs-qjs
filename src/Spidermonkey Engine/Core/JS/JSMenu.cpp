#include "JSMenu.h"

#include "Control.h"
#include "JSControl.h"
#include "Helpers.h"
#include "Engine.h"
#include "Profile.h"
#include "StringWrap.h"

bool my_login(JSContext* ctx, JS::CallArgs& args) {
  if (ClientState() != ClientStateMenu) {
    args.rval().setUndefined();
    return true;
  }

  std::string profile;
  const char* error;

  if (!args[0].isString()) {
    if (Vars.szProfile != NULL) {
      profile = Vars.szProfile;
    } else
      THROW_ERROR(ctx, "Invalid profile specified!");
  } else {
    StringWrap szProfile(ctx, args[0]);
    if (!szProfile) {
      THROW_ERROR(ctx, "failed to encode string");
    }
    strcpy_s(Vars.szProfile, 256, szProfile);
    profile = szProfile.c_str();
  }

  if (profile.empty()) {
    THROW_ERROR(ctx, "Could not get profile!");
  }
  if (!Profile::ProfileExists(profile.c_str())) {
    THROW_ERROR(ctx, "Profile does not exist!");
  }

  Profile* prof = new Profile(profile.c_str());
  if (prof->login(&error) != 0) {
    delete prof;
    THROW_ERROR(ctx, error);
  }

  delete prof;
  args.rval().setUndefined();
  return true;
}

bool my_selectChar(JSContext* ctx, JS::CallArgs& args) {
  if (args.length() != 1 || !args[0].isString())
    THROW_ERROR(ctx, "Invalid parameters specified to selectCharacter");

  StringWrap szProfile(ctx, args[0]);
  if (!szProfile) {
    THROW_ERROR(ctx, "failed to encode string");
  }

  if (!Profile::ProfileExists(szProfile)) {
    THROW_ERROR(ctx, "Invalid profile specified");
  }
  char charname[24], file[_MAX_FNAME + MAX_PATH];
  sprintf_s(file, _countof(file), "%sd2bs.ini", Vars.szPath);
  GetPrivateProfileStringA(szProfile, "character", "ERROR", charname, _countof(charname), file);

  args.rval().setBoolean(OOG_SelectCharacter(charname));
  return true;
}

bool my_createGame(JSContext* ctx, JS::CallArgs& args) {
  if (ClientState() != ClientStateMenu) {
    args.rval().setNull();
    return true;
  }

  std::string name, pass;
  if (args[0].isString()) {
    StringWrap jsname(ctx, args[0]);
    if (!jsname) {
      THROW_ERROR(ctx, "failed to encode string");
    }
    name = jsname.c_str();
  }
  if (args[1].isString()) {
    StringWrap jspass(ctx, args[1]);
    if (!jspass) {
      THROW_ERROR(ctx, "failed to encode string");
    }
    pass = jspass.c_str();
  }
  int32_t diff = 3;
  if (args.length() > 2 && args[2].isInt32()) {
    diff = args[2].toInt32();
  }

  if (name.length() > 15 || pass.length() > 15)
    THROW_ERROR(ctx, "Invalid game name or password length");

  if (!OOG_CreateGame(name.c_str(), pass.c_str(), diff))
    THROW_ERROR(ctx, "createGame failed");

  args.rval().setNull();
  return true;
}

bool my_joinGame(JSContext* ctx, JS::CallArgs& args) {
  if (ClientState() != ClientStateMenu) {
    args.rval().setNull();
    return true;
  }

  std::string name, pass;
  if (args[0].isString()) {
    StringWrap jsname(ctx, args[0]);
    if (!jsname) {
      THROW_ERROR(ctx, "failed to encode string");
    }
    name = jsname.c_str();
  }
  if (args[1].toString()) {
    StringWrap jspass(ctx, args[1]);
    if (!jspass) {
      THROW_ERROR(ctx, "failed to encode string");
    }
    pass = jspass.c_str();
  }

  if (name.length() > 15 || pass.length() > 15)
    THROW_ERROR(ctx, "Invalid game name or password length");

  if (!OOG_JoinGame(name.c_str(), pass.c_str()))
    THROW_ERROR(ctx, "joinGame failed");

  args.rval().setNull();
  return true;
}

bool my_addProfile(JSContext* ctx, JS::CallArgs& args) {
  // validate the args...
  StringWrap profile, mode, gateway, username, password, charname;
  int spdifficulty = 3;
  if (args.length() < 6 || args.length() > 7)
    THROW_ERROR(ctx, "Invalid arguments passed to addProfile");

  StringWrap* vals[] = {&profile, &mode, &gateway, &username, &password, &charname};
  for (uint32_t i = 0; i < 6; i++) {
    *vals[i] = std::move(StringWrap{ctx, JS::ToString(ctx, args[i])});
  }

  if (args.length() == 7) {
    spdifficulty = args[6].toInt32();
  }

  if (spdifficulty > 3 || spdifficulty < 0)
    THROW_ERROR(ctx, "Invalid argument passed to addProfile");

  char file[_MAX_FNAME + _MAX_PATH];

  sprintf_s(file, _countof(file), "%sd2bs.ini", Vars.szPath);
  if (!Profile::ProfileExists(vals[0]->c_str())) {
    char settings[600] = "";
    sprintf_s(settings, _countof(settings), "mode=%s\tgateway=%s\tusername=%s\tpassword=%s\tcharacter=%s\tspdifficulty=%d\t", mode.c_str(), gateway.c_str(),
              username.c_str(), password.c_str(), charname.c_str(), spdifficulty);

    StringReplace(settings, '\t', '\0', 600);
    WritePrivateProfileSectionA(vals[0]->c_str(), settings, file);
  }

  args.rval().setNull();
  return true;
}

bool my_getOOGLocation(JSContext* /*ctx*/, JS::CallArgs& args) {
  if (ClientState() != ClientStateMenu) {
    args.rval().setNull();
    return true;
  }

  args.rval().setInt32(OOG_GetLocation());
  return true;
}

bool my_createCharacter(JSContext* ctx, JS::CallArgs& args) {
  if (ClientState() != ClientStateMenu) {
    args.rval().setUndefined();
    return true;
  }

  int32_t type = -1;
  bool hc = false, ladder = false;
  if (!args[0].isString() || !args[1].isInt32()) {
    THROW_ERROR(ctx, "invalid arguments");
  }
  StringWrap name(ctx, args[0]);
  if (!name) {
    THROW_ERROR(ctx, "failed to encode string");
  }
  type = args[1].toInt32();
  if (args[2].isBoolean()) {
    hc = args[2].toBoolean();
  }
  if (args[3].isBoolean()) {
    ladder = args[3].toBoolean();
  }

  args.rval().setBoolean(OOG_CreateCharacter(name.c_str(), type, hc, ladder));
  return true;
}
