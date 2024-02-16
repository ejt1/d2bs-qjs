#include "JSProfile.h"

#include "Bindings.h"

JSObject* ProfileWrap::Instantiate(JSContext* ctx, Profile* prof) {
  JS::RootedObject global(ctx, JS::CurrentGlobalOrNull(ctx));
  JS::RootedValue constructor_val(ctx);
  if (!JS_GetProperty(ctx, global, "Profile", &constructor_val)) {
    JS_ReportErrorASCII(ctx, "Could not find constructor object for Profile");
    return nullptr;
  }
  if (!constructor_val.isObject()) {
    JS_ReportErrorASCII(ctx, "Profile is not a constructor");
    return nullptr;
  }
  JS::RootedObject constructor(ctx, &constructor_val.toObject());

  JS::RootedObject obj(ctx, JS_New(ctx, constructor, JS::HandleValueArray::empty()));
  if (!obj) {
    JS_ReportErrorASCII(ctx, "Calling Profile constructor failed");
    return nullptr;
  }
  ProfileWrap* wrap = new ProfileWrap(ctx, obj, prof);
  if (!wrap) {
    JS_ReportOutOfMemory(ctx);
    return nullptr;
  }
  return obj;
}

void ProfileWrap::Initialize(JSContext* ctx, JS::HandleObject target) {
  static JSPropertySpec props[] = {
      JS_PSG("type", GetType, JSPROP_ENUMERATE),                                      //
      JS_PSG("ip", GetIP, JSPROP_ENUMERATE),                                          //
      JS_PSG("username", GetUsername, JSPROP_ENUMERATE),                              //
      JS_PSG("gateway", GetGateway, JSPROP_ENUMERATE),                                //
      JS_PSG("character", GetCharacter, JSPROP_ENUMERATE),                            //
      JS_PSG("difficulty", GetDifficulty, JSPROP_ENUMERATE),                          //
      JS_PSG("maxLoginTime", GetMaxLoginTime, JSPROP_ENUMERATE),                      //
      JS_PSG("maxCharacterSelectTime", GetMaxCharacterSelectTime, JSPROP_ENUMERATE),  //
      JS_PS_END,
  };
  static JSFunctionSpec methods[] = {
      JS_FN("login", Login, 0, JSPROP_ENUMERATE),
      JS_FS_END,
  };

  JS::RootedObject proto(ctx, JS_InitClass(ctx, target, nullptr, &m_class, New, 0, props, methods, nullptr, nullptr));
  if (!proto) {
    return;
  }
}

ProfileWrap::ProfileWrap(JSContext* ctx, JS::HandleObject obj, Profile* prof) : BaseObject(ctx, obj), profile(prof) {
}

ProfileWrap::~ProfileWrap() {
  delete profile;
}

void ProfileWrap::finalize(JSFreeOp* fop, JSObject* obj) {
  BaseObject* wrap = BaseObject::FromJSObject(obj);
  if (wrap) {
    delete wrap;
  }
}

bool ProfileWrap::New(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
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
    else if (argc == 1 && args[0].isString()) {
      char* str1 = JS_EncodeString(ctx, args[0].toString());
      prof = new Profile(str1);
      JS_free(ctx, str1);
    } else if (argc > 1 && args[0].isInt32()) {
      int32_t type = args[0].toInt32();

      // Profile(ProfileType.singlePlayer, charname, diff)
      if (argc == 3 && type == PROFILETYPE_SINGLEPLAYER) {
        char* str1 = JS_EncodeString(ctx, args[1].toString());
        int32_t diff = args[2].toInt32();
        prof = new Profile(PROFILETYPE_SINGLEPLAYER, str1, static_cast<char>(diff));
        JS_free(ctx, str1);
      }
      // Profile(ProfileType.battleNet, account, pass, charname, gateway)
      else if (argc == 5 && type == PROFILETYPE_BATTLENET) {
        char* str1 = JS_EncodeString(ctx, args[1].toString());
        char* str2 = JS_EncodeString(ctx, args[2].toString());
        char* str3 = JS_EncodeString(ctx, args[3].toString());
        char* str4 = JS_EncodeString(ctx, args[4].toString());
        prof = new Profile(PROFILETYPE_BATTLENET, str1, str2, str3, str4);
        JS_free(ctx, str1);
        JS_free(ctx, str2);
        JS_free(ctx, str3);
        JS_free(ctx, str4);
      }
      // Profile(ProfileType.openBattleNet, account, pass, charname, gateway)
      else if (argc == 5 && type == PROFILETYPE_OPEN_BATTLENET) {
        char* str1 = JS_EncodeString(ctx, args[1].toString());
        char* str2 = JS_EncodeString(ctx, args[2].toString());
        char* str3 = JS_EncodeString(ctx, args[3].toString());
        char* str4 = JS_EncodeString(ctx, args[4].toString());
        prof = new Profile(PROFILETYPE_OPEN_BATTLENET, str1, str2, str3, str4);
        JS_free(ctx, str1);
        JS_free(ctx, str2);
        JS_free(ctx, str3);
        JS_free(ctx, str4);
      }
      // Profile(ProfileType.tcpIpHost, charname, diff)
      else if (argc == 3 && type == PROFILETYPE_TCPIP_HOST) {
        char* str1 = JS_EncodeString(ctx, args[1].toString());
        int32_t diff = args[2].toInt32();
        prof = new Profile(PROFILETYPE_TCPIP_HOST, str1, static_cast<char>(diff));
        JS_free(ctx, str1);
      }
      // Profile(ProfileType.tcpIpJoin, charname, ip)
      else if (argc == 3 && type == PROFILETYPE_TCPIP_JOIN) {
        char* str1 = JS_EncodeString(ctx, args[1].toString());
        char* str2 = JS_EncodeString(ctx, args[2].toString());
        prof = new Profile(PROFILETYPE_TCPIP_JOIN, str1, str2);
        JS_free(ctx, str1);
        JS_free(ctx, str2);
      } else {
        THROW_ERROR(ctx, "Invalid parameters.");
      }
    } else {
      THROW_ERROR(ctx, "Invalid parameters.");
    }
  } catch (char* ex) {
    THROW_ERROR(ctx, ex);
  }

  JS::RootedObject newObject(ctx, JS_NewObjectForConstructor(ctx, &m_class, args));
  if (!newObject) {
    THROW_ERROR(ctx, "failed to instantiate profile");
  }
  ProfileWrap* wrap = new ProfileWrap(ctx, newObject, prof);
  if (!wrap) {
    JS_ReportOutOfMemory(ctx);
    return false;
  }
  args.rval().setObject(*newObject);
  return true;
}

// properties
bool ProfileWrap::GetType(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  ProfileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Profile* profile = wrap->profile;
  args.rval().setInt32(profile->type);
  return true;
}

bool ProfileWrap::GetIP(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  ProfileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Profile* profile = wrap->profile;
  args.rval().setString(JS_NewStringCopyZ(ctx, profile->ip));
  return true;
}

bool ProfileWrap::GetUsername(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  ProfileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Profile* profile = wrap->profile;
  args.rval().setString(JS_NewStringCopyZ(ctx, profile->username));
  return true;
}

bool ProfileWrap::GetGateway(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  ProfileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Profile* profile = wrap->profile;
  args.rval().setString(JS_NewStringCopyZ(ctx, profile->gateway));
  return true;
}

bool ProfileWrap::GetCharacter(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  ProfileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Profile* profile = wrap->profile;
  args.rval().setString(JS_NewStringCopyZ(ctx, profile->charname));
  return true;
}

bool ProfileWrap::GetDifficulty(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  ProfileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Profile* profile = wrap->profile;
  args.rval().setInt32(profile->diff);
  return true;
}

bool ProfileWrap::GetMaxLoginTime(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  ProfileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Profile* profile = wrap->profile;
  args.rval().setInt32(profile->maxLoginTime);
  return true;
}

bool ProfileWrap::GetMaxCharacterSelectTime(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  ProfileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Profile* profile = wrap->profile;
  args.rval().setInt32(profile->maxCharTime);
  return true;
}

// functions
bool ProfileWrap::Login(JSContext* ctx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  ProfileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  const char* error;
  Profile* profile = wrap->profile;
  if (profile->login(&error) != 0)
    THROW_ERROR(ctx, error);

  args.rval().setUndefined();
  return true;
}

D2BS_BINDING_INTERNAL(ProfileWrap, ProfileWrap::Initialize)
