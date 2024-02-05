#include "JSMenu.h"
// #include "Control.h"
#include "JSControl.h"
#include "Constants.h"
#include "Helpers.h"
#include "Engine.h"

#include "Profile.h"

JSAPI_FUNC(my_login) {
  if (ClientState() != ClientStateMenu)
    return JS_UNDEFINED;

  std::string profile;
  const char* error;

  if (!JS_IsString(argv[0])) {
    if (Vars.szProfile != NULL) {
      profile = Vars.szProfile;
    } else
      THROW_ERROR(ctx, "Invalid profile specified!");
  } else {
    const char* szProfile = JS_ToCString(ctx, argv[0]);
    if (!szProfile) {
      return JS_EXCEPTION;
    }
    strcpy_s(Vars.szProfile, 256, szProfile);
    profile = szProfile;
    JS_FreeCString(ctx, szProfile);
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
  return JS_UNDEFINED;
}

JSAPI_FUNC(my_selectChar) {
  if (argc != 1 || !JS_IsString(argv[0]))
    THROW_ERROR(ctx, "Invalid parameters specified to selectCharacter");

  const char* szProfile = JS_ToCString(ctx, argv[0]);
  if (!szProfile) {
    return JS_EXCEPTION;
  }

  if (!Profile::ProfileExists(szProfile)) {
    JS_FreeCString(ctx, szProfile);
    THROW_ERROR(ctx, "Invalid profile specified");
  }
  char charname[24], file[_MAX_FNAME + MAX_PATH];
  sprintf_s(file, _countof(file), "%sd2bs.ini", Vars.szPath);
  GetPrivateProfileStringA(szProfile, "character", "ERROR", charname, _countof(charname), file);

  JS_FreeCString(ctx, szProfile);
  return JS_NewBool(ctx, OOG_SelectCharacter(charname));
}

JSAPI_FUNC(my_createGame) {
  if (ClientState() != ClientStateMenu)
    return JS_NULL;

  std::string name, pass;
  if (JS_IsString(argv[0])) {
    const char* jsname = JS_ToCString(ctx, argv[0]);
    if (!jsname) {
      return JS_EXCEPTION;
    }
    name = jsname;
    JS_FreeCString(ctx, jsname);
  }
  if (JS_IsString(argv[1])) {
    const char* jspass = JS_ToCString(ctx, argv[1]);
    if (!jspass) {
      return JS_EXCEPTION;
    }
    pass = jspass;
    JS_FreeCString(ctx, jspass);
  }
  int32_t diff = 3;
  if (argc > 2 && JS_IsNumber(argv[2])) {
    if (JS_ToInt32(ctx, &diff, argv[2])) {
      return JS_EXCEPTION;
    }
  }

  if (name.length() > 15 || pass.length() > 15)
    THROW_ERROR(ctx, "Invalid game name or password length");

  if (!OOG_CreateGame(name.c_str(), pass.c_str(), diff))
    THROW_ERROR(ctx, "createGame failed");

  return JS_NULL;
}

JSAPI_FUNC(my_joinGame) {
  if (ClientState() != ClientStateMenu)
    return JS_NULL;

  std::string name, pass;
  if (JS_IsString(argv[0])) {
    const char* jsname = JS_ToCString(ctx, argv[0]);
    if (!jsname) {
      return JS_EXCEPTION;
    }
    name = jsname;
    JS_FreeCString(ctx, jsname);
  }
  if (JS_IsString(argv[1])) {
    const char* jspass = JS_ToCString(ctx, argv[1]);
    if (!jspass) {
      return JS_EXCEPTION;
    }
    pass = jspass;
    JS_FreeCString(ctx, jspass);
  }

  if (name.length() > 15 || pass.length() > 15)
    THROW_ERROR(ctx, "Invalid game name or password length");

  if (!OOG_JoinGame(name.c_str(), pass.c_str()))
    THROW_ERROR(ctx, "joinGame failed");

  return JS_NULL;
}

JSAPI_FUNC(my_addProfile) {
  // validate the args...
  std::string profile, mode, gateway, username, password, charname;
  int spdifficulty = 3;
  if (argc < 6 || argc > 7)
    THROW_ERROR(ctx, "Invalid arguments passed to addProfile");

  std::string* args[] = {&profile, &mode, &gateway, &username, &password, &charname};
  for (uint32_t i = 0; i < 6; i++) {
    const char* tmp = JS_ToCString(ctx, argv[i]);
    if (!tmp) {
      return JS_EXCEPTION;
    }
    *args[i] = tmp;
    JS_FreeCString(ctx, tmp);
  }

  if (argc == 7) {
    JS_ToInt32(ctx, &spdifficulty, argv[6]);
  }

  if (spdifficulty > 3 || spdifficulty < 0)
    THROW_ERROR(ctx, "Invalid argument passed to addProfile");

  char file[_MAX_FNAME + _MAX_PATH];

  sprintf_s(file, _countof(file), "%sd2bs.ini", Vars.szPath);
  if (!Profile::ProfileExists(args[0]->c_str())) {
    char settings[600] = "";
    sprintf_s(settings, _countof(settings), "mode=%s\tgateway=%s\tusername=%s\tpassword=%s\tcharacter=%s\tspdifficulty=%d\t", mode.c_str(), gateway.c_str(),
               username.c_str(), password.c_str(), charname.c_str(), spdifficulty);

    StringReplace(settings, '\t', '\0', 600);
    WritePrivateProfileSectionA(args[0]->c_str(), settings, file);
  }

  return JS_NULL;
}

JSAPI_FUNC(my_getOOGLocation) {
  if (ClientState() != ClientStateMenu)
    return JS_NULL;

  return JS_NewInt32(ctx, OOG_GetLocation());
}

JSAPI_FUNC(my_createCharacter) {
  if (ClientState() != ClientStateMenu)
    return JS_UNDEFINED;

  std::string name;
  int32_t type = -1;
  bool hc = false, ladder = false;
  if (!JS_IsString(argv[0]) || !JS_IsNumber(argv[1])) {
    return JS_EXCEPTION;
  }
  const char* str = JS_ToCString(ctx, argv[0]);
  if (!str || JS_ToInt32(ctx, &type, argv[1])) {
    return JS_EXCEPTION;
  }
  name = str;
  JS_FreeCString(ctx, str);
  if (JS_IsBool(argv[2])) {
    hc = JS_ToBool(ctx, argv[2]);
  }
  if (JS_IsBool(argv[3])) {
    ladder = JS_ToBool(ctx, argv[3]);
  }

  return JS_NewBool(ctx, OOG_CreateCharacter(name.c_str(), type, hc, ladder));
}
