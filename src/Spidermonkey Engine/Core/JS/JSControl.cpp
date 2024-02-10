#include "JSControl.h"

#include "Bindings.h"
#include "D2Helpers.h" // ClientState

JSValue ControlWrap::Instantiate(JSContext* ctx, JSValue new_target, D2WinControlStrc* control) {
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

  ControlWrap* wrap = new ControlWrap(ctx, control);
  if (!wrap) {
    JS_FreeValue(ctx, obj);
    return JS_ThrowOutOfMemory(ctx);
  }
  JS_SetOpaque(obj, wrap);

  // NOTE(ejt): if exception with Control.hasOwnProperty enable this again
  //JS_SetPropertyFunctionList(ctx, obj, m_proto_funcs, _countof(m_proto_funcs));

  return obj;
}

void ControlWrap::Initialize(JSContext* ctx, JSValue target) {
  JSClassDef def{};
  def.class_name = "Control";
  def.finalizer = [](JSRuntime* /*rt*/, JSValue val) {
    ControlWrap* wrap = static_cast<ControlWrap*>(JS_GetOpaque(val, m_class_id));
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

  JSValue obj = JS_NewObjectProtoClass(ctx, proto, m_class_id);
  JS_SetClassProto(ctx, m_class_id, proto);
  JS_SetPropertyStr(ctx, target, "Control", obj);

  JS_SetPropertyStr(ctx, target, "getControl", JS_NewCFunction(ctx, GetControl, "getControl", 0));
  JS_SetPropertyStr(ctx, target, "getControls", JS_NewCFunction(ctx, GetControls, "getControls", 0));
}

ControlWrap::ControlWrap(JSContext* /*ctx*/, D2WinControlStrc* control)
    : dwType(control->dwType), dwX(control->dwPosX), dwY(control->dwPosY), dwSizeX(control->dwSizeX), dwSizeY(control->dwSizeY) {
}

// properties
JSValue ControlWrap::GetText(JSContext* ctx, JSValue this_val) {
  ControlWrap* wrap = static_cast<ControlWrap*>(JS_GetOpaque3(this_val));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2WinControlStrc* ctrl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (!ctrl) {
    return JS_EXCEPTION;
  }

  if (ctrl->TextBox.dwIsCloaked != 33) {
    return JS_NewString(ctx, ctrl->dwType == 6 ? ctrl->Button.wText2 : ctrl->TextBox.wText);
  }
  return JS_UNDEFINED;
}

JSValue ControlWrap::SetText(JSContext* ctx, JSValue this_val, JSValue val) {
  ControlWrap* wrap = static_cast<ControlWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2WinControlStrc* ctrl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (!ctrl) {
    return JS_EXCEPTION;
  }

  if (ctrl->dwType == 1 && JS_IsString(val)) {
    const char* szText = JS_ToCString(ctx, val);
    if (!szText) {
      return JS_EXCEPTION;
    }
    setControlText(ctrl, szText);
    JS_FreeCString(ctx, szText);
  }
  return JS_UNDEFINED;
}

JSValue ControlWrap::GetX(JSContext* ctx, JSValue this_val) {
  ControlWrap* wrap = static_cast<ControlWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2WinControlStrc* ctrl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (!ctrl) {
    return JS_EXCEPTION;
  }

  return JS_NewFloat64(ctx, (double)ctrl->dwPosX);
}

JSValue ControlWrap::GetY(JSContext* ctx, JSValue this_val) {
  ControlWrap* wrap = static_cast<ControlWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2WinControlStrc* ctrl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (!ctrl) {
    return JS_EXCEPTION;
  }

  return JS_NewFloat64(ctx, (double)ctrl->dwPosY);
}

JSValue ControlWrap::GetSizeX(JSContext* ctx, JSValue this_val) {
  ControlWrap* wrap = static_cast<ControlWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2WinControlStrc* ctrl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (!ctrl) {
    return JS_EXCEPTION;
  }

  return JS_NewFloat64(ctx, (double)ctrl->dwSizeX);
}

JSValue ControlWrap::GetSizeY(JSContext* ctx, JSValue this_val) {
  ControlWrap* wrap = static_cast<ControlWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2WinControlStrc* ctrl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (!ctrl) {
    return JS_EXCEPTION;
  }

  return JS_NewFloat64(ctx, (double)ctrl->dwSizeY);
}

JSValue ControlWrap::GetState(JSContext* ctx, JSValue this_val) {
  ControlWrap* wrap = static_cast<ControlWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2WinControlStrc* ctrl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (!ctrl) {
    return JS_EXCEPTION;
  }

  return JS_NewFloat64(ctx, (double)(ctrl->dwDisabled - 2));
}

JSValue ControlWrap::SetState(JSContext* ctx, JSValue this_val, JSValue val) {
  ControlWrap* wrap = static_cast<ControlWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2WinControlStrc* ctrl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (!ctrl) {
    return JS_EXCEPTION;
  }

  if (JS_IsNumber(val)) {
    int32_t nState;
    if (JS_ToInt32(ctx, &nState, val) || nState < 0 || nState > 3) {
      THROW_ERROR(ctx, "Invalid state value");
    }
    memset((void*)&ctrl->dwDisabled, (nState + 2), sizeof(uint32_t));
  }
  return JS_UNDEFINED;
}

JSValue ControlWrap::GetPassword(JSContext* ctx, JSValue this_val) {
  ControlWrap* wrap = static_cast<ControlWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2WinControlStrc* ctrl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (!ctrl) {
    return JS_EXCEPTION;
  }

  return JS_NewBool(ctx, !!(ctrl->TextBox.dwIsCloaked == 33));
}

JSValue ControlWrap::GetType(JSContext* ctx, JSValue this_val) {
  ControlWrap* wrap = static_cast<ControlWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2WinControlStrc* ctrl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (!ctrl) {
    return JS_EXCEPTION;
  }

  return JS_NewFloat64(ctx, (double)ctrl->dwType);
}

JSValue ControlWrap::GetCursorPos(JSContext* ctx, JSValue this_val) {
  ControlWrap* wrap = static_cast<ControlWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2WinControlStrc* ctrl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (!ctrl) {
    return JS_EXCEPTION;
  }

  return JS_NewFloat64(ctx, (double)ctrl->TextBox.dwCursorPos);
}

JSValue ControlWrap::SetCursorPos(JSContext* ctx, JSValue this_val, JSValue val) {
  ControlWrap* wrap = static_cast<ControlWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2WinControlStrc* ctrl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (!ctrl) {
    return JS_EXCEPTION;
  }

  if (JS_IsNumber(val)) {
    uint32_t dwPos;
    if (JS_ToUint32(ctx, &dwPos, val)) {
      THROW_ERROR(ctx, "Invalid cursor position value");
    }
    memset((void*)&ctrl->TextBox.dwCursorPos, dwPos, sizeof(uint32_t));
  }
  return JS_UNDEFINED;
}

JSValue ControlWrap::GetSelectStart(JSContext* ctx, JSValue this_val) {
  ControlWrap* wrap = static_cast<ControlWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2WinControlStrc* ctrl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (!ctrl) {
    return JS_EXCEPTION;
  }

  return JS_NewFloat64(ctx, (double)ctrl->dwSelectStart);
}

JSValue ControlWrap::GetSelectEnd(JSContext* ctx, JSValue this_val) {
  ControlWrap* wrap = static_cast<ControlWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2WinControlStrc* ctrl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (!ctrl) {
    return JS_EXCEPTION;
  }

  return JS_NewFloat64(ctx, (double)ctrl->dwSelectEnd);
}

JSValue ControlWrap::GetDisabled(JSContext* ctx, JSValue this_val) {
  ControlWrap* wrap = static_cast<ControlWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2WinControlStrc* ctrl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (!ctrl) {
    return JS_EXCEPTION;
  }

  return JS_NewFloat64(ctx, (double)ctrl->dwDisabled);
}

JSValue ControlWrap::SetDisabled(JSContext* ctx, JSValue this_val, JSValue val) {
  ControlWrap* wrap = static_cast<ControlWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2WinControlStrc* ctrl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (!ctrl) {
    return JS_EXCEPTION;
  }

  if (JS_IsNumber(val)) {
    uint32_t dwDisabled;
    if (JS_ToUint32(ctx, &dwDisabled, val)) {
      THROW_ERROR(ctx, "Invalid disabled value");
    }
    memset((void*)&ctrl->dwDisabled, dwDisabled, sizeof(uint32_t));
  }
  return JS_UNDEFINED;
}

// functions
JSValue ControlWrap::GetNext(JSContext* ctx, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  if (ClientState() != ClientStateMenu)
    return JS_UNDEFINED;

  ControlWrap* wrap = static_cast<ControlWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

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
    return JS_DupValue(ctx, this_val);
  }
  return JS_FALSE;
}

JSValue ControlWrap::Click(JSContext* ctx, JSValue this_val, int argc, JSValue* argv) {
  if (ClientState() != ClientStateMenu)
    return JS_UNDEFINED;

  ControlWrap* wrap = static_cast<ControlWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2WinControlStrc* pControl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (!pControl) {
    return JS_NewInt32(ctx, 0);
  }

  uint32_t x = (uint32_t)-1, y = (uint32_t)-1;

  if (argc > 1 && JS_IsNumber(argv[0]) && JS_IsNumber(argv[1])) {
    JS_ToUint32(ctx, &x, argv[0]);
    JS_ToUint32(ctx, &y, argv[1]);
  }

  clickControl(pControl, x, y);

  return JS_TRUE;
}

JSValue ControlWrap::FreeGetText(JSContext* ctx, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  if (ClientState() != ClientStateMenu)
    return JS_UNDEFINED;

  ControlWrap* wrap = static_cast<ControlWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2WinControlStrc* pControl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (!pControl) {
    return JS_NewInt32(ctx, 0);
  }

  if (pControl->dwType != 4 || !pControl->pFirstText)
    return JS_UNDEFINED;

  JSValue pReturnArray = JS_NewArray(ctx);
  int nArrayCount = 0;
  for (D2WinTextBoxLineStrc* pText = pControl->pFirstText; pText; pText = pText->pNext) {
    if (!pText->wText[0])
      continue;

    if (pText->wText[1]) {
      JSValue pSubArray = JS_NewArray(ctx);

      for (int i = 0; i < 5; i++) {
        if (pText->wText[i]) {
          JS_SetPropertyUint32(ctx, pSubArray, i, JS_NewString(ctx, pText->wText[i]));
        }
      }

      JS_SetPropertyUint32(ctx, pReturnArray, nArrayCount, pSubArray);
    } else {
      JS_SetPropertyUint32(ctx, pReturnArray, nArrayCount, JS_NewString(ctx, pText->wText[0]));
    }

    nArrayCount++;
  }

  return pReturnArray;
}

JSValue ControlWrap::FreeSetText(JSContext* ctx, JSValue this_val, int argc, JSValue* argv) {
  if (ClientState() != ClientStateMenu)
    return JS_UNDEFINED;

  ControlWrap* wrap = static_cast<ControlWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  D2WinControlStrc* pControl = findControl(wrap->dwType, (const char*)NULL, -1, wrap->dwX, wrap->dwY, wrap->dwSizeX, wrap->dwSizeY);
  if (!pControl) {
    return JS_NewInt32(ctx, 0);
  }

  if (argc < 0 || !JS_IsString(argv[0]))
    return JS_UNDEFINED;

  const char* szText = JS_ToCString(ctx, argv[0]);
  if (!szText) {
    return JS_UNDEFINED;
  }
  setControlText(pControl, szText);
  JS_FreeCString(ctx, szText);
  return JS_UNDEFINED;
}

JSValue ControlWrap::GetControl(JSContext* ctx, JSValue /*this_val*/, int argc, JSValue* argv) {
  if (ClientState() != ClientStateMenu)
    return JS_UNDEFINED;

  int32_t nType = -1, nX = -1, nY = -1, nXSize = -1, nYSize = -1;
  int32_t* args[] = {&nType, &nX, &nY, &nXSize, &nYSize};
  for (int i = 0; i < argc; i++) {
    if (JS_IsNumber(argv[i])) {
      JS_ToInt32(ctx, args[i], argv[i]);
    }
  }

  D2WinControlStrc* pControl = findControl(nType, (const char*)NULL, -1, nX, nY, nXSize, nYSize);
  if (!pControl)
    return JS_UNDEFINED;

  return ControlWrap::Instantiate(ctx, JS_UNDEFINED, pControl);
}

JSValue ControlWrap::GetControls(JSContext* ctx, JSValue /*this_val*/, int /*argc*/, JSValue* /*argv*/) {
  if (ClientState() != ClientStateMenu)
    return JS_UNDEFINED;

  uint32_t dwArrayCount = NULL;

  JSValue pReturnArray = JS_NewArray(ctx);
  for (D2WinControlStrc* pControl = *D2WIN_FirstControl; pControl; pControl = pControl->pNext) {
    JSValue obj = ControlWrap::Instantiate(ctx, JS_UNDEFINED, pControl);
    if (JS_IsException(obj)) {
      JS_FreeValue(ctx, pReturnArray);
      return JS_EXCEPTION;
    }
    JS_SetPropertyUint32(ctx, pReturnArray, dwArrayCount, obj);
    dwArrayCount++;
  }
  return pReturnArray;
}

D2BS_BINDING_INTERNAL(ControlWrap, ControlWrap::Initialize)