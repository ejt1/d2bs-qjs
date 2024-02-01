#include "JSProfile.h"
#include "Profile.h"
#include "Helpers.h"

#include "Engine.h"

// Profile() - get the active profile
// Profile(name) - get the named profile
//
// Create profiles:
//	Profile(ProfileType.singlePlayer, charname, diff)
//	Profile(ProfileType.battleNet, account, pass, charname, gateway)
//	Profile(ProfileType.openBattleNet, account, pass, charname, gateway)
//	Profile(ProfileType.tcpIpHost, charname, diff)
//	Profile(ProfileType.tcpIpJoin, charname, ip)
CLASS_CTOR(profile) {
  Profile* prof;
  ProfileType pt;
  // unsigned int i;

  pt = PROFILETYPE_INVALID;

  try {
    // Profile()
    if (argc == 0) {
      if (Vars.szProfile != NULL)
        prof = new Profile();
      else
        THROW_ERROR(ctx, "No active profile!");
    }
    // Profile(name) - get the named profile
    else if (argc == 1 && JS_IsString(argv[0])) {
      const char* str1 = JS_ToCString(ctx, argv[0]);
      wchar_t* wstr1 = AnsiToUnicode(str1);
      prof = new Profile(wstr1);
      delete[] wstr1;
      JS_FreeCString(ctx, str1);
    } else if (argc > 1 && JS_IsNumber(argv[0])) {
      int32_t type = PROFILETYPE_SINGLEPLAYER;
      JS_ToInt32(ctx, &type, argv[0]);

      // Profile(ProfileType.singlePlayer, charname, diff)
      if (argc == 3 && type == PROFILETYPE_SINGLEPLAYER) {
        const char* str1 = JS_ToCString(ctx, argv[1]);
        wchar_t* wstr1 = AnsiToUnicode(str1);
        int32_t diff;
        JS_ToInt32(ctx, &diff, argv[2]);
        prof = new Profile(PROFILETYPE_SINGLEPLAYER, wstr1, static_cast<char>(diff));
        delete[] wstr1;
        JS_FreeCString(ctx, str1);
      }
      // Profile(ProfileType.battleNet, account, pass, charname, gateway)
      else if (argc == 5 && type == PROFILETYPE_BATTLENET) {
        const char* str1 = JS_ToCString(ctx, argv[1]);
        const char* str2 = JS_ToCString(ctx, argv[2]);
        const char* str3 = JS_ToCString(ctx, argv[3]);
        const char* str4 = JS_ToCString(ctx, argv[4]);
        wchar_t* wstr1 = AnsiToUnicode(str1);
        wchar_t* wstr2 = AnsiToUnicode(str2);
        wchar_t* wstr3 = AnsiToUnicode(str3);
        wchar_t* wstr4 = AnsiToUnicode(str4);
        prof = new Profile(PROFILETYPE_BATTLENET, wstr1, wstr2, wstr3, wstr4);
        delete[] wstr4;
        delete[] wstr3;
        delete[] wstr2;
        delete[] wstr1;
        JS_FreeCString(ctx, str1);
        JS_FreeCString(ctx, str2);
        JS_FreeCString(ctx, str3);
        JS_FreeCString(ctx, str4);
      }
      // Profile(ProfileType.openBattleNet, account, pass, charname, gateway)
      else if (argc == 5 && type == PROFILETYPE_OPEN_BATTLENET) {
        const char* str1 = JS_ToCString(ctx, argv[1]);
        const char* str2 = JS_ToCString(ctx, argv[2]);
        const char* str3 = JS_ToCString(ctx, argv[3]);
        const char* str4 = JS_ToCString(ctx, argv[4]);
        wchar_t* wstr1 = AnsiToUnicode(str1);
        wchar_t* wstr2 = AnsiToUnicode(str2);
        wchar_t* wstr3 = AnsiToUnicode(str3);
        wchar_t* wstr4 = AnsiToUnicode(str4);
        prof = new Profile(PROFILETYPE_OPEN_BATTLENET, wstr1, wstr2, wstr3, wstr4);
        delete[] wstr4;
        delete[] wstr3;
        delete[] wstr2;
        delete[] wstr1;
        JS_FreeCString(ctx, str1);
        JS_FreeCString(ctx, str2);
        JS_FreeCString(ctx, str3);
        JS_FreeCString(ctx, str4);
      }
      // Profile(ProfileType.tcpIpHost, charname, diff)
      else if (argc == 3 && type == PROFILETYPE_TCPIP_HOST) {
        // JS_ConvertArguments(cx, argc, argv, "isu", &pt, &str1, &i);
        const char* str1 = JS_ToCString(ctx, argv[1]);
        wchar_t* wstr1 = AnsiToUnicode(str1);
        int32_t diff;
        JS_ToInt32(ctx, &diff, argv[2]);
        prof = new Profile(PROFILETYPE_TCPIP_HOST, wstr1, static_cast<char>(diff));
        delete[] wstr1;
        JS_FreeCString(ctx, str1);
      }
      // Profile(ProfileType.tcpIpJoin, charname, ip)
      else if (argc == 3 && type == PROFILETYPE_TCPIP_JOIN) {
        // JS_ConvertArguments(cx, argc, argv, "iss", &pt, &str1,
        // &str2);
        const char* str1 = JS_ToCString(ctx, argv[1]);
        const char* str2 = JS_ToCString(ctx, argv[2]);
        wchar_t* wstr1 = AnsiToUnicode(str1);
        wchar_t* wstr2 = AnsiToUnicode(str2);
        prof = new Profile(PROFILETYPE_TCPIP_JOIN, wstr1, wstr2);
        delete[] wstr2;
        delete[] wstr1;
        JS_FreeCString(ctx, str1);
        JS_FreeCString(ctx, str2);
      } else {
        THROW_ERROR(ctx, "Invalid parameters.");
      }
    } else {
      THROW_ERROR(ctx, "Invalid parameters.");
    }
  } catch (char* ex) {
    THROW_ERROR(ctx, ex);
  }

  return BuildObject(ctx, profile_class_id, profile_methods, _countof(profile_methods), profile_props, _countof(profile_props), prof);
}

CLASS_FINALIZER(profile) {
   Profile* prof;

   prof = (Profile*)JS_GetPrivate(val);

   if (prof != NULL)
     delete prof;

   JS_SetPrivate(val, NULL);
}

JSAPI_FUNC(profile_login) {
   const char* error;
   Profile* prof;

   prof = (Profile*)JS_GetPrivate(ctx, this_val);

   if (prof->login(&error) != 0)
     THROW_ERROR(ctx, error);

  return JS_UNDEFINED;
}

JSAPI_PROP(profile_getProperty) {
   Profile* prof;

   prof = (Profile*)JS_GetPrivate(ctx, this_val);

   switch (magic) {
     case PROFILE_TYPE:
       return JS_NewInt32(ctx, prof->type);
     case PROFILE_IP:
       return JS_NewString(ctx, prof->ip);
       break;
     case PROFILE_USERNAME:
       return JS_NewString(ctx, prof->username);
       break;
     case PROFILE_GATEWAY:
       return JS_NewString(ctx, prof->gateway);
       break;
     case PROFILE_CHARACTER:
       return JS_NewString(ctx, prof->charname);
       break;
     case PROFILE_DIFFICULTY:
       return JS_NewInt32(ctx, prof->diff);
       break;
     case PROFILE_MAXLOGINTIME:
       return JS_NewUint32(ctx, prof->maxLoginTime);
       break;
     case PROFILE_MAXCHARSELTIME:
       return JS_NewUint32(ctx, prof->maxCharTime);
       break;
   }

  return JS_TRUE;
}

EMPTY_CTOR(profileType);

JSAPI_PROP(profileType_getProperty) {
  return JS_TRUE;
}
