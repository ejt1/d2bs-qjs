#include "JSControl.h"

#include "Bindings.h"
#include "D2Helpers.h"  // ClientState
#include "StringWrap.h"

JSObject* ControlWrap::Instantiate(JSContext* ctx, D2WinControlStrc* control) {
  JS::RootedObject global(ctx, JS::CurrentGlobalOrNull(ctx));
  JS::RootedValue constructor_val(ctx);
  if (!JS_GetProperty(ctx, global, "Control", &constructor_val)) {
    JS_ReportErrorASCII(ctx, "Could not find constructor object for Control");
    return nullptr;
  }
  if (!constructor_val.isObject()) {
    JS_ReportErrorASCII(ctx, "Control is not a constructor");
    return nullptr;
  }
  JS::RootedObject constructor(ctx, &constructor_val.toObject());

  JS::RootedObject obj(ctx, JS_New(ctx, constructor, JS::HandleValueArray::empty()));
  if (!obj) {
    JS_ReportErrorASCII(ctx, "Calling Control constructor failed");
    return nullptr;
  }
  ControlWrap* wrap = new ControlWrap(ctx, obj, control);
  if (!wrap) {
    JS_ReportOutOfMemory(ctx);
    return nullptr;
  }
  return obj;
}

void ControlWrap::Initialize(JSContext* ctx, JS::HandleObject target) {
  static JSPropertySpec props[] = {
      JS_PSGS("text", trampoline<GetText>, trampoline<SetText>, JSPROP_ENUMERATE),
      JS_PSG("x", trampoline<GetX>, JSPROP_ENUMERATE),
      JS_PSG("y", trampoline<GetY>, JSPROP_ENUMERATE),
      JS_PSG("xsize", trampoline<GetSizeX>, JSPROP_ENUMERATE),
      JS_PSG("ysize", trampoline<GetSizeY>, JSPROP_ENUMERATE),
      JS_PSGS("state", trampoline<GetState>, trampoline<SetState>, JSPROP_ENUMERATE),
      JS_PSG("password", trampoline<GetPassword>, JSPROP_ENUMERATE),
      JS_PSG("type", trampoline<GetType>, JSPROP_ENUMERATE),
      JS_PSGS("cursorpos", trampoline<GetCursorPos>, trampoline<SetCursorPos>, JSPROP_ENUMERATE),
      JS_PSG("selectstart", trampoline<GetSelectStart>, JSPROP_ENUMERATE),
      JS_PSG("selectend", trampoline<GetSelectEnd>, JSPROP_ENUMERATE),
      JS_PSGS("disabled", trampoline<GetDisabled>, trampoline<SetDisabled>, JSPROP_ENUMERATE),
      JS_PS_END,
  };
  static JSFunctionSpec methods[] = {
      JS_FN("getNext", trampoline<GetNext>, 0, JSPROP_ENUMERATE),
      JS_FN("click", trampoline<Click>, 0, JSPROP_ENUMERATE),
      JS_FN("getText", trampoline<FreeGetText>, 0, JSPROP_ENUMERATE),
      JS_FN("setText", trampoline<FreeSetText>, 1, JSPROP_ENUMERATE),
      JS_FS_END,
  };
  JS::RootedObject proto(ctx, JS_InitClass(ctx, target, nullptr, &m_class, trampoline<New>, 0, props, methods, nullptr, nullptr));
  if (!proto) {
    return;
  }

  // globals
  JS_DefineFunction(ctx, target, "getControl", trampoline<GetControl>, 0, JSPROP_ENUMERATE);
  JS_DefineFunction(ctx, target, "getControls", trampoline<GetControls>, 0, JSPROP_ENUMERATE);
}

ControlWrap::ControlWrap(JSContext* ctx, JS::HandleObject obj, D2WinControlStrc* control)
    : BaseObject(ctx, obj), dwType(control->dwType), dwX(control->dwPosX), dwY(control->dwPosY), dwSizeX(control->dwSizeX), dwSizeY(control->dwSizeY) {
}

void ControlWrap::finalize(JSFreeOp* /*fop*/, JSObject* obj) {
  BaseObject* wrap = BaseObject::FromJSObject(obj);
  if (wrap) {
    delete wrap;
  }
}

bool ControlWrap::New(JSContext* ctx, JS::CallArgs& args) {
  JS::RootedObject newObject(ctx, JS_NewObjectForConstructor(ctx, &m_class, args));
  if (!newObject) {
    THROW_ERROR(ctx, "failed to instantiate control");
  }
  args.rval().setObject(*newObject);
  return true;
}

// properties
bool ControlWrap::GetText(JSContext* ctx, JS::CallArgs& args) {
  D2WinControlStrc* ctrl;
  UNWRAP_CONTROL_OR_ERROR(ctx, &ctrl, args.thisv());

  if (ctrl->TextBox.dwIsCloaked != 33) {
    args.rval().setString(JS_NewUCStringCopyZ(ctx, reinterpret_cast<const char16_t*>(ctrl->dwType == 6 ? ctrl->Button.wText2 : ctrl->TextBox.wText)));
    return true;
  }
  args.rval().setUndefined();
  return true;
}

bool ControlWrap::SetText(JSContext* ctx, JS::CallArgs& args) {
  D2WinControlStrc* ctrl;
  UNWRAP_CONTROL_OR_ERROR(ctx, &ctrl, args.thisv());

  if (ctrl->dwType == 1 && args[0].isString()) {
    StringWrap szText(ctx, args[0]);
    if (!szText) {
      THROW_ERROR(ctx, "failed to encode string");
    }
    setControlText(ctrl, szText);
  }
  args.rval().setUndefined();
  return true;
}

bool ControlWrap::GetX(JSContext* ctx, JS::CallArgs& args) {
  D2WinControlStrc* ctrl;
  UNWRAP_CONTROL_OR_ERROR(ctx, &ctrl, args.thisv());
  args.rval().setNumber(ctrl->dwPosX);
  return true;
}

bool ControlWrap::GetY(JSContext* ctx, JS::CallArgs& args) {
  D2WinControlStrc* ctrl;
  UNWRAP_CONTROL_OR_ERROR(ctx, &ctrl, args.thisv());
  args.rval().setNumber(ctrl->dwPosY);
  return true;
}

bool ControlWrap::GetSizeX(JSContext* ctx, JS::CallArgs& args) {
  D2WinControlStrc* ctrl;
  UNWRAP_CONTROL_OR_ERROR(ctx, &ctrl, args.thisv());
  args.rval().setNumber(ctrl->dwSizeX);
  return true;
}

bool ControlWrap::GetSizeY(JSContext* ctx, JS::CallArgs& args) {
  D2WinControlStrc* ctrl;
  UNWRAP_CONTROL_OR_ERROR(ctx, &ctrl, args.thisv());
  args.rval().setNumber(ctrl->dwSizeY);
  return true;
}

bool ControlWrap::GetState(JSContext* ctx, JS::CallArgs& args) {
  D2WinControlStrc* ctrl;
  UNWRAP_CONTROL_OR_ERROR(ctx, &ctrl, args.thisv());
  args.rval().setNumber(ctrl->dwDisabled - 2);
  return true;
}

bool ControlWrap::SetState(JSContext* ctx, JS::CallArgs& args) {
  D2WinControlStrc* ctrl;
  UNWRAP_CONTROL_OR_ERROR(ctx, &ctrl, args.thisv());

  if (args[0].isInt32()) {
    int32_t nState = args[0].toInt32();
    if (nState < 0 || nState > 3) {
      THROW_ERROR(ctx, "Invalid state value");
    }
    memset((void*)&ctrl->dwDisabled, (nState + 2), sizeof(uint32_t));
  }
  args.rval().setUndefined();
  return true;
}

bool ControlWrap::GetPassword(JSContext* ctx, JS::CallArgs& args) {
  D2WinControlStrc* ctrl;
  UNWRAP_CONTROL_OR_ERROR(ctx, &ctrl, args.thisv());
  args.rval().setBoolean(ctrl->TextBox.dwIsCloaked == 33);
  return true;
}

bool ControlWrap::GetType(JSContext* ctx, JS::CallArgs& args) {
  D2WinControlStrc* ctrl;
  UNWRAP_CONTROL_OR_ERROR(ctx, &ctrl, args.thisv());
  args.rval().setNumber(ctrl->dwType);
  return true;
}

bool ControlWrap::GetCursorPos(JSContext* ctx, JS::CallArgs& args) {
  D2WinControlStrc* ctrl;
  UNWRAP_CONTROL_OR_ERROR(ctx, &ctrl, args.thisv());
  args.rval().setNumber(ctrl->TextBox.dwCursorPos);
  return true;
}

bool ControlWrap::SetCursorPos(JSContext* ctx, JS::CallArgs& args) {
  D2WinControlStrc* ctrl;
  UNWRAP_CONTROL_OR_ERROR(ctx, &ctrl, args.thisv());

  if (args[0].isNumber()) {
    uint32_t dwPos;
    if (!JS::ToUint32(ctx, args[0], &dwPos)) {
      THROW_ERROR(ctx, "Invalid cursor position value");
    }
    memset((void*)&ctrl->TextBox.dwCursorPos, dwPos, sizeof(uint32_t));
  }
  args.rval().setUndefined();
  return true;
}

bool ControlWrap::GetSelectStart(JSContext* ctx, JS::CallArgs& args) {
  D2WinControlStrc* ctrl;
  UNWRAP_CONTROL_OR_ERROR(ctx, &ctrl, args.thisv());
  args.rval().setNumber(ctrl->dwSelectStart);
  return true;
}

bool ControlWrap::GetSelectEnd(JSContext* ctx, JS::CallArgs& args) {
  D2WinControlStrc* ctrl;
  UNWRAP_CONTROL_OR_ERROR(ctx, &ctrl, args.thisv());
  args.rval().setNumber(ctrl->dwSelectEnd);
  return true;
}

bool ControlWrap::GetDisabled(JSContext* ctx, JS::CallArgs& args) {
  D2WinControlStrc* ctrl;
  UNWRAP_CONTROL_OR_ERROR(ctx, &ctrl, args.thisv());
  args.rval().setNumber(ctrl->dwDisabled);
  return true;
}

bool ControlWrap::SetDisabled(JSContext* ctx, JS::CallArgs& args) {
  D2WinControlStrc* ctrl;
  UNWRAP_CONTROL_OR_ERROR(ctx, &ctrl, args.thisv());

  if (args[0].isNumber()) {
    uint32_t dwDisabled;
    if (!JS::ToUint32(ctx, args[0], &dwDisabled)) {
      THROW_ERROR(ctx, "Invalid disabled value");
    }
    memset((void*)&ctrl->dwDisabled, dwDisabled, sizeof(uint32_t));
  }
  args.rval().setUndefined();
  return true;
}

// functions
bool ControlWrap::GetNext(JSContext* ctx, JS::CallArgs& args) {
  if (ClientState() != ClientStateMenu) {
    args.rval().setUndefined();
    return true;
  }

  ControlWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  D2WinControlStrc* pControl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (pControl && pControl->pNext)
    pControl = pControl->pNext;
  else
    pControl = NULL;

  if (pControl) {
    // TODO(ejt): want to use this way of returning a new object for the next control
    // however kolbot reuses this_val which is because legacy did what we're about to do
    // replace opaque and return this_val...
    // return ControlWrap::Instantiate(ctx, JS_UNDEFINED, pControl);
    wrap->dwType = pControl->dwType;
    wrap->dwX = pControl->dwPosX;
    wrap->dwY = pControl->dwPosY;
    wrap->dwSizeX = pControl->dwSizeX;
    wrap->dwSizeY = pControl->dwSizeY;
    args.rval().setObject(args.thisv().toObject());
    return true;
  }
  args.rval().setBoolean(false);
  return true;
}

bool ControlWrap::Click(JSContext* ctx, JS::CallArgs& args) {
  if (ClientState() != ClientStateMenu) {
    args.rval().setUndefined();
    return true;
  }

  ControlWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  D2WinControlStrc* pControl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (!pControl) {
    args.rval().setInt32(0);
    return true;
  }

  uint32_t x = (uint32_t)-1, y = (uint32_t)-1;

  if (args.length() > 1 && args[0].isNumber() && args[1].isNumber()) {
    JS::ToUint32(ctx, args[0], &x);
    JS::ToUint32(ctx, args[1], &y);
  }

  clickControl(pControl, x, y);

  args.rval().setBoolean(true);
  return true;
}

bool ControlWrap::FreeGetText(JSContext* ctx, JS::CallArgs& args) {
  if (ClientState() != ClientStateMenu) {
    args.rval().setUndefined();
    return true;
  }

  ControlWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  D2WinControlStrc* pControl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (!pControl) {
    args.rval().setInt32(0);
    return true;
  }

  if (pControl->dwType != 4 || !pControl->pFirstText) {
    args.rval().setUndefined();
    return true;
  }

  JS::RootedObject pReturnArray(ctx, JS::NewArrayObject(ctx, 0));
  int nArrayCount = 0;
  for (D2WinTextBoxLineStrc* pText = pControl->pFirstText; pText; pText = pText->pNext) {
    if (!pText->wText[0])
      continue;

    if (pText->wText[1]) {
      JS::RootedObject pSubArray(ctx, JS::NewArrayObject(ctx, 0));

      for (int i = 0; i < 5; i++) {
        if (pText->wText[i]) {
          JS::RootedString str_val(ctx, JS_NewUCStringCopyZ(ctx, reinterpret_cast<const char16_t*>(pText->wText[i])));
          JS_SetElement(ctx, pSubArray, i, str_val);
        }
      }

      JS_SetElement(ctx, pReturnArray, nArrayCount, pSubArray);
    } else {
      JS::RootedString str_val(ctx, JS_NewUCStringCopyZ(ctx, reinterpret_cast<const char16_t*>(pText->wText[0])));
      JS_SetElement(ctx, pReturnArray, nArrayCount, str_val);
    }

    nArrayCount++;
  }
  args.rval().setObject(*pReturnArray);
  return true;
}

bool ControlWrap::FreeSetText(JSContext* ctx, JS::CallArgs& args) {
  if (ClientState() != ClientStateMenu) {
    args.rval().setUndefined();
    return true;
  }

  ControlWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  D2WinControlStrc* pControl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (!pControl) {
    args.rval().setInt32(0);
    return true;
  }

  if (args.length() < 0 || !args[0].isString()) {
    args.rval().setUndefined();
    return true;
  }

  StringWrap szText(ctx, args[0]);
  if (!szText) {
    args.rval().setUndefined();
    return true;
  }
  setControlText(pControl, szText);
  args.rval().setUndefined();
  return true;
}

bool ControlWrap::GetControl(JSContext* ctx, JS::CallArgs& args) {
  if (ClientState() != ClientStateMenu) {
    args.rval().setUndefined();
    return true;
  }

  int32_t nType = -1, nX = -1, nY = -1, nXSize = -1, nYSize = -1;
  int32_t* vals[] = {&nType, &nX, &nY, &nXSize, &nYSize};
  for (uint32_t i = 0; i < args.length(); i++) {
    if (args[i].isInt32()) {
      *vals[i] = args[i].toInt32();
    }
  }

  D2WinControlStrc* pControl = findControl(nType, (const char*)NULL, -1, nX, nY, nXSize, nYSize);
  if (!pControl) {
    args.rval().setUndefined();
    return true;
  }

  JS::RootedObject obj(ctx, ControlWrap::Instantiate(ctx, pControl));
  if (!obj) {
    THROW_ERROR(ctx, "failed to instantiate control");
  }
  args.rval().setObject(*obj);
  return true;
}

bool ControlWrap::GetControls(JSContext* ctx, JS::CallArgs& args) {
  if (ClientState() != ClientStateMenu) {
    args.rval().setUndefined();
    return true;
  }

  uint32_t dwArrayCount = NULL;

  JS::RootedObject pReturnArray(ctx, JS::NewArrayObject(ctx, 0));
  for (D2WinControlStrc* pControl = *D2WIN_FirstControl; pControl; pControl = pControl->pNext) {
    JS::RootedObject obj(ctx, ControlWrap::Instantiate(ctx, pControl));
    if (!obj) {
      THROW_ERROR(ctx, "failed to instantiate control");
    }
    JS_SetElement(ctx, pReturnArray, dwArrayCount, obj);
    dwArrayCount++;
  }
  args.rval().setObject(*pReturnArray);
  return true;
}

D2BS_BINDING_INTERNAL(ControlWrap, ControlWrap::Initialize)
