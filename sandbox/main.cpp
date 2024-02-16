#pragma warning(push, 0)
#include <jsapi.h>
#include <jsfriendapi.h>
#include <js/Initialization.h>
#pragma warning(pop)

class A {
 public:
  A(JS::HandleObject obj) {
    JS_SetReservedSlot(obj, 0, JS::PrivateValue(this));
    printf("A\n");
  }

  virtual ~A() {
    printf("~A\n");
  }

  static JSObject* Instantiate(JSContext* cx) {
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS::RootedValue constructor_val(cx);
    if (!JS_GetProperty(cx, global, "A", &constructor_val)) {
      JS_ReportErrorASCII(cx, "Could not find constructor object for A");
      return nullptr;
    }
    if (!constructor_val.isObject()) {
      JS_ReportErrorASCII(cx, "A is not a constructor");
      return nullptr;
    }
    JS::RootedObject constructor(cx, &constructor_val.toObject());

    JS::RootedObject obj(cx, JS_New(cx, constructor, JS::HandleValueArray::empty()));
    if (!obj) {
      JS_ReportErrorASCII(cx, "Calling A constructor failed");
      return nullptr;
    }
    return obj;
  }

  static bool New(JSContext* cx, unsigned argc, JS::Value* vp) {
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject newObject(cx, JS_NewObjectForConstructor(cx, &Aclass, args));
    A* a = new A(newObject);
    args.rval().setObject(*newObject);
    return true;
  }

  static inline void finalize(JSFreeOp* fop, JSObject* obj) {
    printf("finalize\n");
    JS::Value val = JS_GetReservedSlot(obj, 0);
    if (val.isDouble()) {
      A* a = static_cast<A*>(val.toPrivate());
      delete a;
    }
  }

  static inline JSClassOps Aops = {
      .addProperty = nullptr,
      .delProperty = nullptr,
      .enumerate = nullptr,
      .newEnumerate = nullptr,
      .resolve = nullptr,
      .mayResolve = nullptr,
      .finalize = finalize,
      .call = nullptr,
      .hasInstance = nullptr,
      .construct = nullptr,
      .trace = nullptr,
  };
  static inline JSClass Aclass = {
      "A",
      JSCLASS_HAS_RESERVED_SLOTS(1) | JSCLASS_FOREGROUND_FINALIZE,
      &Aops,
  };
};

const JSClassOps DefaultGlobalClassOps = {
    nullptr,                         // addProperty
    nullptr,                         // deleteProperty
    nullptr,                         // enumerate
    JS_NewEnumerateStandardClasses,  // newEnumerate
    JS_ResolveStandardClass,         // resolve
    JS_MayResolveStandardClass,      // mayResolve
    nullptr,                         // finalize
    nullptr,                         // call
    nullptr,                         // hasInstance
    nullptr,                         // construct
    JS_GlobalObjectTraceHook         // trace
};

int main() {
  JS_Init();
  JSContext* cx = JS_NewContext(JS::DefaultHeapMaxBytes);

  js::UseInternalJobQueues(cx);
  JS::InitSelfHostedCode(cx);
  JS::PersistentRootedObject persistent;

  {
    JSAutoRequest request(cx);
    JS::CompartmentOptions global_options;
    static JSClass kGlobalClass{"D2BSGlobal", JSCLASS_GLOBAL_FLAGS, &DefaultGlobalClassOps};
    JS::RootedObject global(cx, JS_NewGlobalObject(cx, &kGlobalClass, nullptr, JS::FireOnNewGlobalHook, global_options));
    JSAutoCompartment ac(cx, global);

    JS_InitClass(cx, global, nullptr, &A::Aclass, A::New, 0, nullptr, nullptr, nullptr, nullptr);

    persistent.init(cx, A::Instantiate(cx));

    JS::CompileOptions options(cx);
    options.setFileAndLine("<eval>", 1);

    std::string source(R"js(
        const a = new A;
      )js");

    JS::RootedValue rval(cx);
    JS::Evaluate(cx, options, source.c_str(), source.length(), &rval);
  }

  JS_DestroyContext(cx);

  JS_ShutDown();
  return 0;
}
