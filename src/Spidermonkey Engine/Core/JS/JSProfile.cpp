#include "JSProfile.h"

#include "Bindings.h"

JSValue ProfileWrap::Instantiate(JSContext* ctx, JSValue new_target, Profile* prof) {
  JSValue proto;
  if (JS_IsUndefined(new_target)) {
    proto = JS_GetClassProto(ctx, m_class_id);
  } else {
    proto = JS_GetPropertyStr(ctx, new_target, "prototype");
    if (JS_IsException(proto)) {
      return JS_EXCEPTION;
    }
  }
  JSValue obj = JS_NewObjectProtoClass(ctx, proto, m_class_id);
  JS_FreeValue(ctx, proto);
  if (JS_IsException(obj)) {
    return obj;
  }

  ProfileWrap* wrap = new ProfileWrap(ctx, prof);
  if (!wrap) {
    JS_FreeValue(ctx, obj);
    return JS_ThrowOutOfMemory(ctx);
  }
  JS_SetOpaque(obj, wrap);

  return obj;
}

void ProfileWrap::Initialize(JSContext* ctx, JSValue target) {
  JSClassDef def{};
  def.class_name = "Profile";
  def.finalizer = [](JSRuntime* /*rt*/, JSValue val) {
    ProfileWrap* wrap = static_cast<ProfileWrap*>(JS_GetOpaque(val, m_class_id));
    if (wrap) {
      delete wrap;
    }
  };

  if (m_class_id == 0) {
    JS_NewClassID(&m_class_id);
  }
  JS_NewClass(JS_GetRuntime(ctx), m_class_id, &def);

  JSValue proto = JS_NewObject(ctx);
  JS_SetPropertyFunctionList(ctx, proto, m_proto_funcs, _countof(m_proto_funcs));

  JSValue obj = JS_NewCFunction2(ctx, New, "Profile", 0, JS_CFUNC_constructor, 0);
  JS_SetConstructor(ctx, obj, proto);

  JS_SetClassProto(ctx, m_class_id, proto);
  JS_SetPropertyStr(ctx, target, "Profile", obj);
}

ProfileWrap::ProfileWrap(JSContext* /*ctx*/, Profile* prof) : profile(prof) {
}

ProfileWrap::~ProfileWrap() {
  delete profile;
}

JSValue ProfileWrap::New(JSContext* ctx, JSValue new_target, int argc, JSValue* argv) {
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
      prof = new Profile(str1);
      JS_FreeCString(ctx, str1);
    } else if (argc > 1 && JS_IsNumber(argv[0])) {
      int32_t type = PROFILETYPE_SINGLEPLAYER;
      JS_ToInt32(ctx, &type, argv[0]);

      // Profile(ProfileType.singlePlayer, charname, diff)
      if (argc == 3 && type == PROFILETYPE_SINGLEPLAYER) {
        const char* str1 = JS_ToCString(ctx, argv[1]);
        int32_t diff;
        JS_ToInt32(ctx, &diff, argv[2]);
        prof = new Profile(PROFILETYPE_SINGLEPLAYER, str1, static_cast<char>(diff));
        JS_FreeCString(ctx, str1);
      }
      // Profile(ProfileType.battleNet, account, pass, charname, gateway)
      else if (argc == 5 && type == PROFILETYPE_BATTLENET) {
        const char* str1 = JS_ToCString(ctx, argv[1]);
        const char* str2 = JS_ToCString(ctx, argv[2]);
        const char* str3 = JS_ToCString(ctx, argv[3]);
        const char* str4 = JS_ToCString(ctx, argv[4]);
        prof = new Profile(PROFILETYPE_BATTLENET, str1, str2, str3, str4);
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
        prof = new Profile(PROFILETYPE_OPEN_BATTLENET, str1, str2, str3, str4);
        JS_FreeCString(ctx, str1);
        JS_FreeCString(ctx, str2);
        JS_FreeCString(ctx, str3);
        JS_FreeCString(ctx, str4);
      }
      // Profile(ProfileType.tcpIpHost, charname, diff)
      else if (argc == 3 && type == PROFILETYPE_TCPIP_HOST) {
        // JS_ConvertArguments(cx, argc, argv, "isu", &pt, &str1, &i);
        const char* str1 = JS_ToCString(ctx, argv[1]);
        int32_t diff;
        JS_ToInt32(ctx, &diff, argv[2]);
        prof = new Profile(PROFILETYPE_TCPIP_HOST, str1, static_cast<char>(diff));
        JS_FreeCString(ctx, str1);
      }
      // Profile(ProfileType.tcpIpJoin, charname, ip)
      else if (argc == 3 && type == PROFILETYPE_TCPIP_JOIN) {
        // JS_ConvertArguments(cx, argc, argv, "iss", &pt, &str1,
        // &str2);
        const char* str1 = JS_ToCString(ctx, argv[1]);
        const char* str2 = JS_ToCString(ctx, argv[2]);
        prof = new Profile(PROFILETYPE_TCPIP_JOIN, str1, str2);
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

  return ProfileWrap::Instantiate(ctx, new_target, prof);
}

// properties
JSValue ProfileWrap::GetType(JSContext* ctx, JSValue this_val) {
  ProfileWrap* wrap = static_cast<ProfileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  Profile* profile = wrap->profile;
  return JS_NewInt32(ctx, profile->type);
}

JSValue ProfileWrap::GetIP(JSContext* ctx, JSValue this_val) {
  ProfileWrap* wrap = static_cast<ProfileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  Profile* profile = wrap->profile;
  return JS_NewString(ctx, profile->ip);
}

JSValue ProfileWrap::GetUsername(JSContext* ctx, JSValue this_val) {
  ProfileWrap* wrap = static_cast<ProfileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  Profile* profile = wrap->profile;
  return JS_NewString(ctx, profile->username);
}

JSValue ProfileWrap::GetGateway(JSContext* ctx, JSValue this_val) {
  ProfileWrap* wrap = static_cast<ProfileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  Profile* profile = wrap->profile;
  return JS_NewString(ctx, profile->gateway);
}

JSValue ProfileWrap::GetCharacter(JSContext* ctx, JSValue this_val) {
  ProfileWrap* wrap = static_cast<ProfileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  Profile* profile = wrap->profile;
  return JS_NewString(ctx, profile->charname);
}

JSValue ProfileWrap::GetDifficulty(JSContext* ctx, JSValue this_val) {
  ProfileWrap* wrap = static_cast<ProfileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  Profile* profile = wrap->profile;
  return JS_NewInt32(ctx, profile->diff);
}

JSValue ProfileWrap::GetMaxLoginTime(JSContext* ctx, JSValue this_val) {
  ProfileWrap* wrap = static_cast<ProfileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  Profile* profile = wrap->profile;
  return JS_NewUint32(ctx, profile->maxLoginTime);
}

JSValue ProfileWrap::GetMaxCharacterSelectTime(JSContext* ctx, JSValue this_val) {
  ProfileWrap* wrap = static_cast<ProfileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  Profile* profile = wrap->profile;
  return JS_NewUint32(ctx, profile->maxCharTime);
}

// functions
JSValue ProfileWrap::Login(JSContext* ctx, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  const char* error;

  ProfileWrap* wrap = static_cast<ProfileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  Profile* profile = wrap->profile;
  if (profile->login(&error) != 0)
    THROW_ERROR(ctx, error);

  return JS_UNDEFINED;
}

D2BS_BINDING_INTERNAL(ProfileWrap, ProfileWrap::Initialize)