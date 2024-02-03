// #include "Control.h"
#include "JSControl.h"
#include "Helpers.h"
#include "D2Helpers.h"

EMPTY_CTOR(control)

CLASS_FINALIZER(control) {
  ControlData* pData = ((ControlData*)JS_GetOpaque3(val));

  if (pData) {
    JS_SetOpaque(val, NULL);
    delete pData;
  }
}

JSAPI_PROP(control_getProperty) {
  if (ClientState() != ClientStateMenu)
    return JS_EXCEPTION;

  ControlData* pData = ((ControlData*)JS_GetOpaque3(this_val));
  if (!pData)
    return JS_EXCEPTION;

  Control* ctrl = findControl(pData->dwType, (const char*)NULL, -1, pData->dwX, pData->dwY, pData->dwSizeX, pData->dwSizeY);
  if (!ctrl)
    return JS_EXCEPTION;

  switch (magic) {
    case CONTROL_TEXT:
      if (ctrl->dwIsCloaked != 33) {
        return JS_NewString(ctx, ctrl->dwType == 6 ? ctrl->wText2 : ctrl->wText);
      }
      break;
    case CONTROL_X:
      return JS_NewFloat64(ctx, (double)ctrl->dwPosX);
      // JS_NewNumberValue(cx, ctrl->dwPosX, vp);
      break;
    case CONTROL_Y:
      return JS_NewFloat64(ctx, (double)ctrl->dwPosY);
      break;
    case CONTROL_XSIZE:
      return JS_NewFloat64(ctx, (double)ctrl->dwSizeX);
      break;
    case CONTROL_YSIZE:
      return JS_NewFloat64(ctx, (double)ctrl->dwSizeY);
      break;
    case CONTROL_STATE:
      return JS_NewFloat64(ctx, (double)(ctrl->dwDisabled - 2));
      break;
    case CONTROL_MAXLENGTH:
      // JS_NewNumberValue(cx, ctrl->dwMaxLength, vp);
      break;
    case CONTROL_TYPE:
      return JS_NewFloat64(ctx, (double)ctrl->dwType);
      break;
    case CONTROL_VISIBLE:
      // nothing to do yet because we don't know what to do
      break;
    case CONTROL_CURSORPOS:
      return JS_NewFloat64(ctx, (double)ctrl->dwCursorPos);
      break;
    case CONTROL_SELECTSTART:
      return JS_NewFloat64(ctx, (double)ctrl->dwSelectStart);
      break;
    case CONTROL_SELECTEND:
      return JS_NewFloat64(ctx, (double)ctrl->dwSelectEnd);
      break;
    case CONTROL_PASSWORD:
      return JS_NewBool(ctx, !!(ctrl->dwIsCloaked == 33));
      break;
    case CONTROL_DISABLED:
      return JS_NewFloat64(ctx, (double)ctrl->dwDisabled);
      break;
  }
  return JS_TRUE;
}

JSAPI_STRICT_PROP(control_setProperty) {
  if (ClientState() != ClientStateMenu)
    return JS_EXCEPTION;

  ControlData* pData = ((ControlData*)JS_GetOpaque3(this_val));
  if (!pData)
    return JS_EXCEPTION;

  Control* ctrl = findControl(pData->dwType, (const char*)NULL, -1, pData->dwX, pData->dwY, pData->dwSizeX, pData->dwSizeY);
  if (!ctrl)
    return JS_EXCEPTION;

  switch (magic) {
    case CONTROL_TEXT:
      if (ctrl->dwType == 1 && JS_IsString(val)) {
        const char* szText = JS_ToCString(ctx, val);
        if (!szText) {
          return JS_EXCEPTION;
        }
        setControlText(ctrl, szText);
        JS_FreeCString(ctx, szText);
      }
      break;
    case CONTROL_STATE:
      if (JS_IsNumber(val)) {
        int32_t nState;
        if (JS_ToInt32(ctx, &nState, val) || nState < 0 || nState > 3) {
          THROW_ERROR(ctx, "Invalid state value");
        }
        memset((void*)&ctrl->dwDisabled, (nState + 2), sizeof(DWORD));
      }
      break;
    case CONTROL_CURSORPOS:
      if (JS_IsNumber(val)) {
        uint32_t dwPos;
        if (JS_ToUint32(ctx, &dwPos, val)) {
          THROW_ERROR(ctx, "Invalid cursor position value");
        }
        memset((void*)&ctrl->dwCursorPos, dwPos, sizeof(DWORD));
      }
      break;
    case CONTROL_DISABLED:
      if (JS_IsNumber(val)) {
        uint32_t dwDisabled;
        if (JS_ToUint32(ctx, &dwDisabled, val)) {
          THROW_ERROR(ctx, "Invalid disabled value");
        }
        memset((void*)&ctrl->dwDisabled, dwDisabled, sizeof(DWORD));
      }
      break;
  }

  return JS_TRUE;
}

JSAPI_FUNC(control_getNext) {
  if (ClientState() != ClientStateMenu)
    return JS_UNDEFINED;

  ControlData* pData = ((ControlData*)JS_GetOpaque3(this_val));
  if (!pData)
    return JS_UNDEFINED;

  Control* pControl = findControl(pData->dwType, (const char*)NULL, -1, pData->dwX, pData->dwY, pData->dwSizeX, pData->dwSizeY);
  if (pControl && pControl->pNext)
    pControl = pControl->pNext;
  else
    pControl = NULL;

  if (pControl) {
    pData->pControl = pControl;
    pData->dwType = pData->pControl->dwType;
    pData->dwX = pData->pControl->dwPosX;
    pData->dwY = pData->pControl->dwPosY;
    pData->dwSizeX = pData->pControl->dwSizeX;
    pData->dwSizeY = pData->pControl->dwSizeY;
    JS_SetOpaque(this_val, pData);
    return JS_DupValue(ctx, this_val);
  }
  return JS_UNDEFINED;
}

JSAPI_FUNC(control_click) {
  if (ClientState() != ClientStateMenu)
    return JS_TRUE;

  ControlData* pData = ((ControlData*)JS_GetOpaque3(this_val));
  if (!pData)
    return JS_TRUE;

  Control* pControl = findControl(pData->dwType, (const char*)NULL, -1, pData->dwX, pData->dwY, pData->dwSizeX, pData->dwSizeY);
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

JSAPI_FUNC(control_setText) {
  if (ClientState() != ClientStateMenu)
    return JS_TRUE;

  ControlData* pData = ((ControlData*)JS_GetOpaque3(this_val));
  if (!pData)
    return JS_TRUE;

  Control* pControl = findControl(pData->dwType, (const char*)NULL, -1, pData->dwX, pData->dwY, pData->dwSizeX, pData->dwSizeY);
  if (!pControl) {
    return JS_NewInt32(ctx, 0);
  }

  if (argc < 0 || !JS_IsString(argv[0]))
    return JS_TRUE;

  const char* szText = JS_ToCString(ctx, argv[0]);
  if (!szText) {
    return JS_EXCEPTION;
  }
  setControlText(pControl, szText);
  JS_FreeCString(ctx, szText);
  return JS_TRUE;
}

JSAPI_FUNC(control_getText) {
  if (ClientState() != ClientStateMenu)
    return JS_UNDEFINED;

  ControlData* pData = ((ControlData*)JS_GetOpaque3(this_val));
  if (!pData)
    return JS_UNDEFINED;

  Control* pControl = findControl(pData->dwType, (const char*)NULL, -1, pData->dwX, pData->dwY, pData->dwSizeX, pData->dwSizeY);
  if (!pControl) {
    return JS_NewInt32(ctx, 0);
  }

  if (pControl->dwType != 4 || !pControl->pFirstText)
    return JS_UNDEFINED;

  JSValue pReturnArray = JS_NewArray(ctx);
  int nArrayCount = 0;
  for (ControlText* pText = pControl->pFirstText; pText; pText = pText->pNext) {
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

JSAPI_FUNC(my_getControl) {
  if (ClientState() != ClientStateMenu)
    return JS_UNDEFINED;

  int32_t nType = -1, nX = -1, nY = -1, nXSize = -1, nYSize = -1;
  int32_t* args[] = {&nType, &nX, &nY, &nXSize, &nYSize};
  for (int i = 0; i < argc; i++) {
    if (JS_IsNumber(argv[i])) {
      JS_ToInt32(ctx, args[i], argv[i]);
    }
  }

  Control* pControl = findControl(nType, (const char*)NULL, -1, nX, nY, nXSize, nYSize);
  if (!pControl)
    return JS_UNDEFINED;

  ControlData* data = new ControlData;
  data->dwType = pControl->dwType;
  data->dwX = pControl->dwPosX;
  data->dwY = pControl->dwPosY;
  data->dwSizeX = pControl->dwSizeX;
  data->dwSizeY = pControl->dwSizeY;
  data->pControl = pControl;

  JSValue control = BuildObject(ctx, control_class_id, control_funcs, _countof(control_funcs), control_props, _countof(control_props), data);
  if (!control)
    THROW_ERROR(ctx, "Failed to build control!");

  return control;
}

JSAPI_FUNC(my_getControls) {
  if (ClientState() != ClientStateMenu)
    return JS_UNDEFINED;

  DWORD dwArrayCount = NULL;

  JSValue pReturnArray = JS_NewArray(ctx);
  for (Control* pControl = *p_D2WIN_FirstControl; pControl; pControl = pControl->pNext) {
    ControlData* data = new ControlData;
    data->dwType = pControl->dwType;
    data->dwX = pControl->dwPosX;
    data->dwY = pControl->dwPosY;
    data->dwSizeX = pControl->dwSizeX;
    data->dwSizeY = pControl->dwSizeY;
    data->pControl = pControl;

    JSValue res = BuildObject(ctx, control_class_id, control_funcs, _countof(control_funcs), control_props, _countof(control_props), data);
    JS_SetPropertyUint32(ctx, pReturnArray, dwArrayCount, res);
    dwArrayCount++;
  }
  return pReturnArray;
}
