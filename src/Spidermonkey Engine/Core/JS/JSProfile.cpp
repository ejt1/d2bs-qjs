#include "JSProfile.h"

#include "Bindings.h"
#include "StringWrap.h"

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
      JS_PSG("type", trampoline<GetType>, JSPROP_ENUMERATE),                                      //
      JS_PSG("ip", trampoline<GetIP>, JSPROP_ENUMERATE),                                          //
      JS_PSG("username", trampoline<GetUsername>, JSPROP_ENUMERATE),                              //
      JS_PSG("gateway", trampoline<GetGateway>, JSPROP_ENUMERATE),                                //
      JS_PSG("character", trampoline<GetCharacter>, JSPROP_ENUMERATE),                            //
      JS_PSG("difficulty", trampoline<GetDifficulty>, JSPROP_ENUMERATE),                          //
      JS_PSG("maxLoginTime", trampoline<GetMaxLoginTime>, JSPROP_ENUMERATE),                      //
      JS_PSG("maxCharacterSelectTime", trampoline<GetMaxCharacterSelectTime>, JSPROP_ENUMERATE),  //
      JS_PS_END,
  };
  static JSFunctionSpec methods[] = {
      JS_FN("login", trampoline<Login>, 0, JSPROP_ENUMERATE),
      JS_FS_END,
  };

  JS::RootedObject proto(ctx, JS_InitClass(ctx, target, nullptr, &m_class, trampoline<New>, 0, props, methods, nullptr, nullptr));
  if (!proto) {
    return;
  }
}

ProfileWrap::ProfileWrap(JSContext* ctx, JS::HandleObject obj, Profile* prof) : BaseObject(ctx, obj), profile(prof) {
}

ProfileWrap::~ProfileWrap() {
  delete profile;
}

void ProfileWrap::finalize(JSFreeOp* /*fop*/, JSObject* obj) {
  BaseObject* wrap = BaseObject::FromJSObject(obj);
  if (wrap) {
    delete wrap;
  }
}

bool ProfileWrap::New(JSContext* ctx, JS::CallArgs& args) {
  Profile* prof;
  ProfileType pt;
  // unsigned int i;

  pt = PROFILETYPE_INVALID;

  try {
    // Profile()
    if (args.length() == 0) {
      if (Vars.szProfile != NULL)
        prof = new Profile();
      else
        THROW_ERROR(ctx, "No active profile!");
    }
    // Profile(name) - get the named profile
    else if (args.length() == 1 && args[0].isString()) {
      StringWrap str1(ctx, args[0]);
      prof = new Profile(str1);
    } else if (args.length() > 1 && args[0].isInt32()) {
      int32_t type = args[0].toInt32();

      // Profile(ProfileType.singlePlayer, charname, diff)
      if (args.length() == 3 && type == PROFILETYPE_SINGLEPLAYER) {
        StringWrap str1(ctx, args[1]);
        int32_t diff = args[2].toInt32();
        prof = new Profile(PROFILETYPE_SINGLEPLAYER, str1, static_cast<char>(diff));
      }
      // Profile(ProfileType.battleNet, account, pass, charname, gateway)
      else if (args.length() == 5 && type == PROFILETYPE_BATTLENET) {
        StringWrap str1(ctx, args[1]);
        StringWrap str2(ctx, args[2]);
        StringWrap str3(ctx, args[3]);
        StringWrap str4(ctx, args[4]);
        prof = new Profile(PROFILETYPE_BATTLENET, str1, str2, str3, str4);
      }
      // Profile(ProfileType.openBattleNet, account, pass, charname, gateway)
      else if (args.length() == 5 && type == PROFILETYPE_OPEN_BATTLENET) {
        StringWrap str1(ctx, args[1]);
        StringWrap str2(ctx, args[2]);
        StringWrap str3(ctx, args[3]);
        StringWrap str4(ctx, args[4]);
        prof = new Profile(PROFILETYPE_OPEN_BATTLENET, str1, str2, str3, str4);
      }
      // Profile(ProfileType.tcpIpHost, charname, diff)
      else if (args.length() == 3 && type == PROFILETYPE_TCPIP_HOST) {
        StringWrap str1(ctx, args[1]);
        int32_t diff = args[2].toInt32();
        prof = new Profile(PROFILETYPE_TCPIP_HOST, str1, static_cast<char>(diff));
      }
      // Profile(ProfileType.tcpIpJoin, charname, ip)
      else if (args.length() == 3 && type == PROFILETYPE_TCPIP_JOIN) {
        StringWrap str1(ctx, args[1]);
        StringWrap str2(ctx, args[2]);
        prof = new Profile(PROFILETYPE_TCPIP_JOIN, str1.c_str(), str2.c_str());
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
bool ProfileWrap::GetType(JSContext* ctx, JS::CallArgs& args) {
  ProfileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Profile* profile = wrap->profile;
  args.rval().setInt32(profile->type);
  return true;
}

bool ProfileWrap::GetIP(JSContext* ctx, JS::CallArgs& args) {
  ProfileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Profile* profile = wrap->profile;
  args.rval().setString(JS_NewStringCopyZ(ctx, profile->ip));
  return true;
}

bool ProfileWrap::GetUsername(JSContext* ctx, JS::CallArgs& args) {
  ProfileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Profile* profile = wrap->profile;
  args.rval().setString(JS_NewStringCopyZ(ctx, profile->username));
  return true;
}

bool ProfileWrap::GetGateway(JSContext* ctx, JS::CallArgs& args) {
  ProfileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Profile* profile = wrap->profile;
  args.rval().setString(JS_NewStringCopyZ(ctx, profile->gateway));
  return true;
}

bool ProfileWrap::GetCharacter(JSContext* ctx, JS::CallArgs& args) {
  ProfileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Profile* profile = wrap->profile;
  args.rval().setString(JS_NewStringCopyZ(ctx, profile->charname));
  return true;
}

bool ProfileWrap::GetDifficulty(JSContext* ctx, JS::CallArgs& args) {
  ProfileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Profile* profile = wrap->profile;
  args.rval().setInt32(profile->diff);
  return true;
}

bool ProfileWrap::GetMaxLoginTime(JSContext* ctx, JS::CallArgs& args) {
  ProfileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Profile* profile = wrap->profile;
  args.rval().setInt32(profile->maxLoginTime);
  return true;
}

bool ProfileWrap::GetMaxCharacterSelectTime(JSContext* ctx, JS::CallArgs& args) {
  ProfileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  Profile* profile = wrap->profile;
  args.rval().setInt32(profile->maxCharTime);
  return true;
}

// functions
bool ProfileWrap::Login(JSContext* ctx, JS::CallArgs& args) {
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
