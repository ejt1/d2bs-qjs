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

  std::wstring profile;
  const char* error;

  if (!JS_IsString(argv[0])) {
    if (Vars.szProfile != NULL) {
      profile = Vars.szProfile;
    } else
      THROW_ERROR(ctx, "Invalid profile specified!");
  } else {
    const char* szProfile = JS_ToCString(ctx, argv[0]);
    wchar_t* wszProfile = AnsiToUnicode(szProfile);
    JS_FreeCString(ctx, szProfile);
    wcscpy_s(Vars.szProfile, 256, wszProfile);
    profile = wszProfile;
    delete[] wszProfile;
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
  const wchar_t* profile = AnsiToUnicode(szProfile);
  JS_FreeCString(ctx, szProfile);

  if (!Profile::ProfileExists(profile)) {
    delete[] profile;
    THROW_ERROR(ctx, "Invalid profile specified");
  }
  wchar_t charname[24], file[_MAX_FNAME + MAX_PATH];
  swprintf_s(file, _countof(file), L"%sd2bs.ini", Vars.szPath);
  GetPrivateProfileStringW(profile, L"character", L"ERROR", charname, _countof(charname), file);

  delete[] profile;
  return JS_NewBool(ctx, OOG_SelectCharacter(charname));
}

JSAPI_FUNC(my_createGame) {
  if (ClientState() != ClientStateMenu)
    return JS_NULL;

  std::wstring name, pass;
  if (JS_IsString(argv[0])) {
    auto jsname = JS_ToWString(ctx, argv[0]);
    if (!jsname) {
      return JS_EXCEPTION;
    }
  }
  if (JS_IsString(argv[0])) {
    auto jspass= JS_ToWString(ctx, argv[0]);
    if (!jspass) {
      return JS_EXCEPTION;
    }
  }
  int32_t diff = 3;
  if (argc > 2 && JS_IsNumber(argv[2])) {
    if (JS_ToInt32(ctx, &diff, argv[2])) {
      return JS_EXCEPTION;
    }
  }

  if (name.length() > 15 || pass.length()  > 15)
    THROW_ERROR(ctx, "Invalid game name or password length");

  if (!OOG_CreateGame(name.c_str(), pass.c_str(), diff))
    THROW_ERROR(ctx, "createGame failed");

  return JS_NULL;
}

JSAPI_FUNC(my_joinGame) {
  if (ClientState() != ClientStateMenu)
    return JS_NULL;

  std::wstring name, pass;
  if (JS_IsString(argv[0])) {
    auto jsname = JS_ToWString(ctx, argv[0]);
    if (!jsname) {
      return JS_EXCEPTION;
    }
  }
  if (JS_IsString(argv[0])) {
    auto jspass = JS_ToWString(ctx, argv[0]);
    if (!jspass) {
      return JS_EXCEPTION;
    }
  }

  if (name.length() > 15 || pass.length() > 15)
    THROW_ERROR(ctx, "Invalid game name or password length");

  if (!OOG_JoinGame(name.c_str(), pass.c_str()))
    THROW_ERROR(ctx, "joinGame failed");

  return JS_NULL;
}

JSAPI_FUNC(my_addProfile) {
  // validate the args...
  std::wstring profile, mode, gateway, username, password, charname;
  int spdifficulty = 3;
  if (argc < 6 || argc > 7)
    THROW_ERROR(ctx, "Invalid arguments passed to addProfile");

  std::wstring* args[] = {&profile, &mode, &gateway, &username, &password, &charname};
  for (uint i = 0; i < 6; i++) {
    auto tmp = JS_ToWString(ctx, argv[i]);
    if (!tmp) {
      THROW_ERROR(ctx, "Invalid argument passed to addProfile");
    }
    *args[i] = *tmp;
  }

  if (argc == 7) {
    JS_ToInt32(ctx, &spdifficulty, argv[6]);
  }

  if (spdifficulty > 3 || spdifficulty < 0)
    THROW_ERROR(ctx, "Invalid argument passed to addProfile");

  wchar_t file[_MAX_FNAME + _MAX_PATH];

  swprintf_s(file, _countof(file), L"%sd2bs.ini", Vars.szPath);
  if (!Profile::ProfileExists(args[0]->c_str())) {
    wchar_t settings[600] = L"";
    swprintf_s(settings, _countof(settings), L"mode=%s\tgateway=%s\tusername=%s\tpassword=%s\tcharacter=%s\tspdifficulty=%d\t", mode.c_str(), gateway.c_str(),
               username.c_str(), password.c_str(), charname.c_str(), spdifficulty);

    StringReplace(settings, '\t', '\0', 600);
    WritePrivateProfileSectionW(args[0]->c_str(), settings, file);
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
    return JS_TRUE;

  std::wstring name;
  int32 type = -1;
  bool hc = false, ladder = false;
  if (!JS_IsString(argv[0]) || !JS_IsNumber(argv[1])) {
    return JS_EXCEPTION;
  }
  auto tmp = JS_ToWString(ctx, argv[0]);
  if (!tmp || JS_ToInt32(ctx, &type, argv[1])) {
    return JS_EXCEPTION;
  }
  if (JS_IsBool(argv[2])) {
    hc = JS_ToBool(ctx, argv[2]);
  }
  if (JS_IsBool(argv[3])) {
    ladder = JS_ToBool(ctx, argv[3]);
  }

  return JS_NewBool(ctx, OOG_CreateCharacter(name.c_str(), type, hc, ladder));
}
