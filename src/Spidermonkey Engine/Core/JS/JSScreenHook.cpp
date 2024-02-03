#include "JSScreenHook.h"
#include "Engine.h"
#include "ScreenHook.h"
#include "Script.h"
#include "File.h"
#include <Helpers.h>

CLASS_FINALIZER(hook) {
  Genhook* hook = (Genhook*)JS_GetPrivate(val);
  Genhook::EnterGlobalSection();
  if (hook) {
    JS_SetPrivate(val, NULL);
    delete hook;
  }
  Genhook::LeaveGlobalSection();
}

JSAPI_FUNC(hook_remove) {
  (argc);

  Genhook::EnterGlobalSection();
  Genhook* hook = (Genhook*)JS_GetPrivate(ctx, this_val);
  if (hook) {
    delete hook;
  }

  JS_SetPrivate(ctx, this_val, NULL);
  // JS_ValueToObject(cx, JSVAL_VOID, &obj);
  Genhook::LeaveGlobalSection();

  return JS_TRUE;
}

// Function to create a frame which gets called on a "new Frame ()"
// Parameters: x, y, xsize, ysize, alignment, automap, onClick, onHover
JSAPI_FUNC(frame_ctor) {
  Script* script = (Script*)JS_GetContextPrivate(ctx);

  uint32_t x = 0, y = 0, x2 = 0, y2 = 0;
  Align align = Left;
  bool automap = false;
  JSValue click = JS_UNDEFINED, hover = JS_UNDEFINED;

  if (argc > 0 && JS_IsNumber(argv[0]))
    JS_ToUint32(ctx, &x, argv[0]);
  if (argc > 1 && JS_IsNumber(argv[1]))
    JS_ToUint32(ctx, &y, argv[1]);
  if (argc > 2 && JS_IsNumber(argv[2]))
    JS_ToUint32(ctx, &x2, argv[2]);
  if (argc > 3 && JS_IsNumber(argv[3]))
    JS_ToUint32(ctx, &y2, argv[3]);
  if (argc > 4 && JS_IsNumber(argv[4]))
    JS_ToUint32(ctx, (uint32_t*)(&align), argv[4]);
  if (argc > 5 && JS_IsBool(argv[5]))
    automap = !!JS_ToBool(ctx, argv[5]);
  if (argc > 6 && JS_IsFunction(ctx, argv[6]))
    click = argv[6];
  if (argc > 7 && JS_IsFunction(ctx, argv[7]))
    hover = argv[7];

  JSValue hook = BuildObject(ctx, frame_class_id, frame_methods, _countof(frame_methods), frame_props, _countof(frame_props));
  if (JS_IsException(hook))
    THROW_ERROR(ctx, "Failed to create frame object");

  // framehooks don't work out of game -- they just crash
  FrameHook* pFrameHook = new FrameHook(script, hook, x, y, x2, y2, automap, align, IG);
  if (!pFrameHook)
    THROW_ERROR(ctx, "Failed to create framehook");

  JS_SetPrivate(ctx, hook, pFrameHook);
  pFrameHook->SetClickHandler(click);
  pFrameHook->SetHoverHandler(hover);

  return hook;
}

JSAPI_PROP(frame_getProperty) {
  FrameHook* pFramehook = (FrameHook*)JS_GetPrivate(ctx, this_val);
  if (!pFramehook)
    return JS_TRUE;

  switch (magic) {
    case FRAME_X:
      return JS_NewUint32(ctx, pFramehook->GetX());
      break;
    case FRAME_Y:
      return JS_NewUint32(ctx, pFramehook->GetY());
      break;
    case FRAME_XSIZE:
      return JS_NewUint32(ctx, pFramehook->GetXSize());
      break;
    case FRAME_YSIZE:
      return JS_NewUint32(ctx, pFramehook->GetYSize());
      break;
    case FRAME_ALIGN:
      return JS_NewUint32(ctx, pFramehook->GetAlign());
      break;
    case FRAME_VISIBLE:
      return JS_NewBool(ctx, pFramehook->GetIsVisible());
      break;
    case FRAME_ZORDER:
      return JS_NewUint32(ctx, pFramehook->GetZOrder());
      break;
    case FRAME_ONCLICK:
      return JS_DupValue(ctx, pFramehook->GetClickHandler());
      break;
    case FRAME_ONHOVER:
      return JS_DupValue(ctx, pFramehook->GetHoverHandler());
      break;
  }
  return JS_TRUE;
}

JSAPI_STRICT_PROP(frame_setProperty) {
  FrameHook* pFramehook = (FrameHook*)JS_GetPrivate(ctx, this_val);
  if (!pFramehook)
    return JS_UNDEFINED;

  uint32_t ival = 0;
  bool bval = 0;
  if (JS_IsBool(val)) {
    bval = JS_ToBool(ctx, val);
  }
  if (JS_IsNumber(val) && JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }

  switch (magic) {
    case FRAME_X:
      pFramehook->SetX(ival);
      break;
    case FRAME_Y:
      pFramehook->SetY(ival);
      break;
    case FRAME_XSIZE:
      pFramehook->SetXSize(ival);
      break;
    case FRAME_YSIZE:
      pFramehook->SetYSize(ival);
      break;
    case FRAME_ALIGN:
      pFramehook->SetAlign((Align)ival);
      break;
    case FRAME_VISIBLE:
      pFramehook->SetIsVisible(bval);
      break;
    case FRAME_ZORDER:
      pFramehook->SetZOrder((ushort)ival);
      break;
    case FRAME_ONCLICK:
      pFramehook->SetClickHandler(val);
      break;
    case FRAME_ONHOVER:
      pFramehook->SetHoverHandler(val);
      break;
  }
  return JS_TRUE;
}

// Box functions

// Parameters: x, y, xsize, ysize, color, opacity, alignment, automap, onClick, onHover
JSAPI_FUNC(box_ctor) {
  Script* script = (Script*)JS_GetContextPrivate(ctx);

  ScreenhookState state = (script->GetMode() == kScriptModeMenu) ? OOG : IG;
  uint32_t x = 0, y = 0, x2 = 0, y2 = 0, color = 0, opacity = 0, align = Left;
  bool automap = false;
  JSValue click = JS_UNDEFINED, hover = JS_UNDEFINED;

  if (argc > 0 && JS_IsNumber(argv[0]))
    JS_ToUint32(ctx, &x, argv[0]);
  if (argc > 1 && JS_IsNumber(argv[1]))
    JS_ToUint32(ctx, &y, argv[1]);
  if (argc > 2 && JS_IsNumber(argv[2]))
    JS_ToUint32(ctx, &x2, argv[2]);
  if (argc > 3 && JS_IsNumber(argv[3]))
    JS_ToUint32(ctx, &y2, argv[3]);
  if (argc > 4 && JS_IsNumber(argv[4]))
    JS_ToUint32(ctx, &color, argv[4]);
  if (argc > 5 && JS_IsNumber(argv[5]))
    JS_ToUint32(ctx, &opacity, argv[5]);
  if (argc > 6 && JS_IsNumber(argv[6]))
    JS_ToUint32(ctx, &align, argv[6]);
  if (argc > 7 && JS_IsBool(argv[7]))
    automap = !!JS_ToBool(ctx, argv[7]);
  if (argc > 8 && JS_IsFunction(ctx, argv[8]))
    click = argv[8];
  if (argc > 9 && JS_IsFunction(ctx, argv[9]))
    hover = argv[9];

  JSValue hook = BuildObject(ctx, box_class_id, box_methods, _countof(box_methods), box_props, _countof(box_props));
  if (!hook) {
    THROW_ERROR(ctx, "Failed to create box object");
  }

  BoxHook* pBoxHook = new BoxHook(script, hook, x, y, x2, y2, (ushort)color, (ushort)opacity, automap, (Align)align, state);

  if (!pBoxHook)
    THROW_ERROR(ctx, "Unable to initalize a box class.");

  JS_SetPrivate(ctx, hook, pBoxHook);
  pBoxHook->SetClickHandler(click);
  pBoxHook->SetHoverHandler(hover);

  return hook;
}
JSAPI_PROP(box_getProperty) {
  BoxHook* pBoxHook = (BoxHook*)JS_GetPrivate(ctx, this_val);
  if (!pBoxHook)
    return JS_TRUE;

  switch (magic) {
    case BOX_X:
      return JS_NewUint32(ctx, pBoxHook->GetX());
      break;
    case BOX_Y:
      return JS_NewUint32(ctx, pBoxHook->GetY());
      break;
    case BOX_XSIZE:
      return JS_NewUint32(ctx, pBoxHook->GetXSize());
      break;
    case BOX_YSIZE:
      return JS_NewUint32(ctx, pBoxHook->GetYSize());
      break;
    case BOX_ALIGN:
      return JS_NewUint32(ctx, pBoxHook->GetAlign());
      break;
    case BOX_COLOR:
      return JS_NewUint32(ctx, pBoxHook->GetColor());
      break;
    case BOX_OPACITY:
      return JS_NewUint32(ctx, pBoxHook->GetOpacity());
      break;
    case BOX_VISIBLE:
      return JS_NewBool(ctx, pBoxHook->GetIsVisible());
      break;
    case BOX_ZORDER:
      return JS_NewUint32(ctx, pBoxHook->GetZOrder());
      break;
    case BOX_ONCLICK:
      return JS_DupValue(ctx, pBoxHook->GetClickHandler());
      break;
    case BOX_ONHOVER:
      return JS_DupValue(ctx, pBoxHook->GetHoverHandler());
      break;
  }
  return JS_TRUE;
}

JSAPI_STRICT_PROP(box_setProperty) {
  BoxHook* pBoxHook = (BoxHook*)JS_GetPrivate(ctx, this_val);
  if (!pBoxHook)
    return JS_TRUE;

  uint32_t ival = 0;
  bool bval = 0;
  if (JS_IsBool(val)) {
    bval = JS_ToBool(ctx, val);
  }
  if (JS_IsNumber(val) && JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }

  switch (magic) {
    case BOX_X:
      pBoxHook->SetX(ival);
      break;
    case BOX_Y:
      pBoxHook->SetY(ival);
      break;
    case BOX_XSIZE:
      pBoxHook->SetXSize(ival);
      break;
    case BOX_YSIZE:
      pBoxHook->SetYSize(ival);
      break;
    case BOX_OPACITY:
      pBoxHook->SetOpacity((ushort)ival);
      break;
    case BOX_COLOR:
      pBoxHook->SetColor((ushort)ival);
      break;
    case BOX_ALIGN:
      pBoxHook->SetAlign((Align)ival);
      break;
    case BOX_VISIBLE:
      pBoxHook->SetIsVisible(!!bval);
      break;
    case BOX_ZORDER:
      pBoxHook->SetZOrder((ushort)ival);
      break;
    case BOX_ONCLICK:
      pBoxHook->SetClickHandler(val);
      break;
    case BOX_ONHOVER:
      pBoxHook->SetHoverHandler(val);
      break;
  }
  return JS_TRUE;
}

// Line functions

// Parameters: x, y, x2, y2, color, automap, click, hover
JSAPI_FUNC(line_ctor) {
  Script* script = (Script*)JS_GetContextPrivate(ctx);

  ScreenhookState state = (script->GetMode() == kScriptModeMenu) ? OOG : IG;
  uint32_t x = 0, y = 0, x2 = 0, y2 = 0, color = 0;
  bool automap = false;
  JSValue click = JS_UNDEFINED, hover = JS_UNDEFINED;

  if (argc > 0 && JS_IsNumber(argv[0]))
    JS_ToUint32(ctx, &x, argv[0]);
  if (argc > 1 && JS_IsNumber(argv[1]))
    JS_ToUint32(ctx, &y, argv[1]);
  if (argc > 2 && JS_IsNumber(argv[2]))
    JS_ToUint32(ctx, &x2, argv[2]);
  if (argc > 3 && JS_IsNumber(argv[3]))
    JS_ToUint32(ctx, &y2, argv[3]);
  if (argc > 4 && JS_IsNumber(argv[4]))
    JS_ToUint32(ctx, &color, argv[4]);
  if (argc > 5 && JS_IsBool(argv[5]))
    automap = !!JS_ToBool(ctx, argv[5]);
  if (argc > 6 && JS_IsFunction(ctx, argv[6]))
    click = argv[6];
  if (argc > 7 && JS_IsFunction(ctx, argv[7]))
    hover = argv[7];

  JSValue hook = BuildObject(ctx, line_class_id, line_methods, _countof(line_methods), line_props, _countof(line_props));
  if (!hook) {
    THROW_ERROR(ctx, "Failed to create line object");
  }

  LineHook* pLineHook = new LineHook(script, hook, x, y, x2, y2, static_cast<ushort>(color), automap, Left, state);

  if (!pLineHook) {
    THROW_ERROR(ctx, "Unable to initalize a line class.");
  }

  JS_SetPrivate(ctx, hook, pLineHook);
  pLineHook->SetClickHandler(click);
  pLineHook->SetHoverHandler(hover);

  return hook;
}

JSAPI_PROP(line_getProperty) {
  LineHook* pLineHook = (LineHook*)JS_GetPrivate(ctx, this_val);
  if (!pLineHook)
    return JS_TRUE;

  switch (magic) {
    case LINE_X:
      return JS_NewUint32(ctx, pLineHook->GetX());
      break;
    case LINE_Y:
      return JS_NewUint32(ctx, pLineHook->GetY());
      break;
    case LINE_XSIZE:
      return JS_NewUint32(ctx, pLineHook->GetX2());
      break;
    case LINE_YSIZE:
      return JS_NewUint32(ctx, pLineHook->GetY2());
      break;
    case LINE_COLOR:
      return JS_NewUint32(ctx, pLineHook->GetColor());
      break;
    case LINE_VISIBLE:
      return JS_NewBool(ctx, pLineHook->GetIsVisible());
      break;
    case LINE_ZORDER:
      return JS_NewUint32(ctx, pLineHook->GetZOrder());
      break;
    case LINE_ONCLICK:
      return JS_DupValue(ctx, pLineHook->GetClickHandler());
      break;
    case LINE_ONHOVER:
      return JS_DupValue(ctx, pLineHook->GetHoverHandler());
      break;
  }
  return JS_TRUE;
}

JSAPI_STRICT_PROP(line_setProperty) {
  LineHook* pLineHook = (LineHook*)JS_GetPrivate(ctx, this_val);
  if (!pLineHook)
    return JS_TRUE;

  uint32_t ival = 0;
  bool bval = 0;
  if (JS_IsBool(val)) {
    bval = JS_ToBool(ctx, val);
  }
  if (JS_IsNumber(val) && JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }

  switch (magic) {
    case LINE_X:
      pLineHook->SetX(ival);
      break;
    case LINE_Y:
      pLineHook->SetY(ival);
      break;
    case LINE_XSIZE:
      pLineHook->SetX2(ival);
      break;
    case LINE_YSIZE:
      pLineHook->SetY2(ival);
      break;
    case LINE_COLOR:
      pLineHook->SetColor((ushort)ival);
      break;
    case LINE_VISIBLE:
      pLineHook->SetIsVisible(!!bval);
      break;
    case LINE_ZORDER:
      pLineHook->SetZOrder((ushort)ival);
      break;
    case LINE_ONCLICK:
      pLineHook->SetClickHandler(val);
      break;
    case LINE_ONHOVER:
      pLineHook->SetHoverHandler(val);
      break;
  }
  return JS_TRUE;
}

// Function to create a text which gets called on a "new text ()"

// Parameters: text, x, y, color, font, align, automap, onHover, onText
JSAPI_FUNC(text_ctor) {
  Script* script = (Script*)JS_GetContextPrivate(ctx);

  ScreenhookState state = (script->GetMode() == kScriptModeMenu) ? OOG : IG;
  uint32_t x = 0, y = 0, color = 0, font = 0, align = Left;
  bool automap = false;
  JSValue click = JS_UNDEFINED, hover = JS_UNDEFINED;
  std::wstring szText;

  if (argc > 0 && JS_IsString(argv[0])) {
    const char* str = JS_ToCString(ctx, argv[0]);
    if (!str) {
      return JS_EXCEPTION;
    }
    szText = AnsiToWide(str);
    JS_FreeCString(ctx, str);
  }
  if (argc > 1 && JS_IsNumber(argv[1]))
    JS_ToUint32(ctx, &x, argv[1]);
  if (argc > 2 && JS_IsNumber(argv[2]))
    JS_ToUint32(ctx, &y, argv[2]);
  if (argc > 3 && JS_IsNumber(argv[3]))
    JS_ToUint32(ctx, &color, argv[3]);
  if (argc > 4 && JS_IsNumber(argv[4]))
    JS_ToUint32(ctx, &font, argv[4]);
  if (argc > 5 && JS_IsNumber(argv[5]))
    JS_ToUint32(ctx, &align, argv[5]);
  if (argc > 6 && JS_IsBool(argv[6]))
    automap = !!JS_ToBool(ctx, argv[6]);
  if (argc > 7 && JS_IsFunction(ctx, argv[7]))
    click = argv[7];
  if (argc > 8 && JS_IsFunction(ctx, argv[8]))
    hover = argv[8];

  JSValue hook = BuildObject(ctx, text_class_id, text_methods, _countof(text_methods), text_props, _countof(text_props));
  if (!hook) {
    THROW_ERROR(ctx, "Failed to create text object");
  }

  TextHook* pTextHook = new TextHook(script, hook, szText.c_str(), x, y, static_cast<ushort>(font), static_cast<ushort>(color), automap, (Align)align, state);

  if (!pTextHook) {
    THROW_ERROR(ctx, "Failed to create texthook");
  }

  JS_SetPrivate(ctx, hook, pTextHook);
  pTextHook->SetClickHandler(click);
  pTextHook->SetHoverHandler(hover);
  return hook;
}

JSAPI_PROP(text_getProperty) {
  TextHook* pTextHook = (TextHook*)JS_GetPrivate(ctx, this_val);
  if (!pTextHook)
    return JS_TRUE;

  switch (magic) {
    case TEXT_X:
      return JS_NewUint32(ctx, pTextHook->GetX());
      break;
    case TEXT_Y:
      return JS_NewUint32(ctx, pTextHook->GetY());
      break;
    case TEXT_COLOR:
      return JS_NewUint32(ctx, pTextHook->GetColor());
      break;
    case TEXT_FONT:
      return JS_NewUint32(ctx, pTextHook->GetFont());
      break;
    case TEXT_TEXT:
      return JS_NewString(ctx, pTextHook->GetText());
      break;
    case TEXT_ALIGN:
      return JS_NewUint32(ctx, pTextHook->GetAlign());
      break;
    case TEXT_VISIBLE:
      return JS_NewBool(ctx, pTextHook->GetIsVisible());
      break;
    case TEXT_ZORDER:
      return JS_NewUint32(ctx, pTextHook->GetZOrder());
      break;
    case TEXT_ONCLICK:
      return JS_DupValue(ctx, pTextHook->GetClickHandler());
      break;
    case TEXT_ONHOVER:
      return JS_DupValue(ctx, pTextHook->GetHoverHandler());
      break;
  }
  return JS_TRUE;
}

JSAPI_STRICT_PROP(text_setProperty) {
  TextHook* pTextHook = (TextHook*)JS_GetPrivate(ctx, this_val);
  if (!pTextHook)
    return JS_TRUE;

  uint32_t ival = 0;
  bool bval = 0;
  if (JS_IsBool(val)) {
    bval = JS_ToBool(ctx, val);
  }
  if (JS_IsNumber(val) && JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }

  switch (magic) {
    case TEXT_X:
      pTextHook->SetX(ival);
      break;
    case TEXT_Y:
      pTextHook->SetY(ival);
      break;
    case TEXT_COLOR:
      pTextHook->SetColor((ushort)ival);
      break;
    case TEXT_FONT:
      pTextHook->SetFont((ushort)ival);
      break;
    case TEXT_TEXT:
      if (JS_IsString(val)) {
        const char* szText = JS_ToCString(ctx, val);
        if (!szText) {
          return JS_EXCEPTION;
        }
        std::wstring pText = AnsiToWide(szText);
        pTextHook->SetText(pText.c_str());
        JS_FreeCString(ctx, szText);
      }
      break;
    case TEXT_ALIGN:
      pTextHook->SetAlign((Align)ival);
      break;
    case TEXT_VISIBLE:
      pTextHook->SetIsVisible(!!bval);
      break;
    case TEXT_ZORDER:
      pTextHook->SetZOrder((ushort)ival);
      break;
    case TEXT_ONCLICK:
      pTextHook->SetClickHandler(val);
      break;
    case TEXT_ONHOVER:
      pTextHook->SetHoverHandler(val);
      break;
  }
  return JS_TRUE;
}

// Function to create a image which gets called on a "new Image ()"

// Parameters: image, x, y, color, align, automap, onHover, onimage
JSAPI_FUNC(image_ctor) {
  Script* script = (Script*)JS_GetContextPrivate(ctx);

  ScreenhookState state = (script->GetMode() == kScriptModeMenu) ? OOG : IG;
  uint32_t x = 0, y = 0, color = 0, align = Left;
  bool automap = false;
  JSValue click = JS_UNDEFINED, hover = JS_UNDEFINED;
  const char* szText = nullptr;
  wchar_t path[_MAX_FNAME + _MAX_PATH];

  if (argc > 0 && JS_IsString(argv[0])) {
    szText = JS_ToCString(ctx, argv[0]);
    if (!szText) {
      return JS_EXCEPTION;
    }
  }
  if (argc > 1 && JS_IsNumber(argv[1]))
    JS_ToUint32(ctx, &x, argv[1]);
  if (argc > 2 && JS_IsNumber(argv[2]))
    JS_ToUint32(ctx, &y, argv[2]);
  if (argc > 3 && JS_IsNumber(argv[3]))
    JS_ToUint32(ctx, &color, argv[3]);
  if (argc > 4 && JS_IsNumber(argv[4]))
    JS_ToUint32(ctx, &align, argv[4]);
  if (argc > 5 && JS_IsBool(argv[5]))
    automap = !!JS_ToBool(ctx, argv[5]);
  if (argc > 6 && JS_IsFunction(ctx, argv[6]))
    click = argv[6];
  if (argc > 7 && JS_IsFunction(ctx, argv[7]))
    hover = argv[7];

  if (isValidPath(szText)) {
    swprintf_s(path, _countof(path), L"%S", szText);
    JS_FreeCString(ctx, szText);
  } else {
    JS_FreeCString(ctx, szText);
    THROW_ERROR(ctx, "Invalid image file path");
  }

  JSValue hook = BuildObject(ctx, image_class_id, image_methods, _countof(image_methods), image_props, _countof(image_props));
  if (!hook)
    THROW_ERROR(ctx, "Failed to create image object");

  ImageHook* pImageHook = new ImageHook(script, hook, path, x, y, static_cast<ushort>(color), automap, (Align)align, state, true);

  if (!pImageHook)
    THROW_ERROR(ctx, "Failed to create ImageHook");

  JS_SetPrivate(ctx, hook, pImageHook);
  pImageHook->SetClickHandler(click);
  pImageHook->SetHoverHandler(hover);

  return hook;
}

JSAPI_PROP(image_getProperty) {
  ImageHook* pImageHook = (ImageHook*)JS_GetPrivate(ctx, this_val);
  if (!pImageHook)
    return JS_TRUE;

  switch (magic) {
    case IMAGE_X:
      return JS_NewUint32(ctx, pImageHook->GetX());
      break;
    case IMAGE_Y:
      return JS_NewUint32(ctx, pImageHook->GetY());
      break;
    case IMAGE_LOCATION:
      return JS_NewString(ctx, pImageHook->GetImage());
      break;
    case IMAGE_ALIGN:
      return JS_NewUint32(ctx, pImageHook->GetAlign());
      break;
    case IMAGE_VISIBLE:
      return JS_NewBool(ctx, pImageHook->GetIsVisible());
      break;
    case IMAGE_ZORDER:
      return JS_NewUint32(ctx, pImageHook->GetZOrder());
      break;
    case IMAGE_ONCLICK:
      return JS_DupValue(ctx, pImageHook->GetClickHandler());
      break;
    case IMAGE_ONHOVER:
      return JS_DupValue(ctx, pImageHook->GetHoverHandler());
      break;
  }
  return JS_TRUE;
}

JSAPI_STRICT_PROP(image_setProperty) {
  ImageHook* pImageHook = (ImageHook*)JS_GetPrivate(ctx, this_val);
  if (!pImageHook)
    return JS_TRUE;

  uint32_t ival = 0;
  bool bval = 0;
  if (JS_IsBool(val)) {
    bval = JS_ToBool(ctx, val);
  }
  if (JS_IsNumber(val) && JS_ToUint32(ctx, &ival, val)) {
    return JS_EXCEPTION;
  }

  switch (magic) {
    case IMAGE_X:
      pImageHook->SetX(ival);
      break;
    case IMAGE_Y:
      pImageHook->SetY(ival);
      break;
    case IMAGE_LOCATION:
      if (JS_IsString(val)) {
        const char* szText = JS_ToCString(ctx, val);
        if (!szText) {
          return JS_EXCEPTION;
        }
        std::wstring pText = AnsiToWide(szText);
        pImageHook->SetImage(pText.c_str());
        JS_FreeCString(ctx, szText);
      }
      break;
    case IMAGE_ALIGN:
      pImageHook->SetAlign((Align)ival);
      break;
    case IMAGE_VISIBLE:
      pImageHook->SetIsVisible(!!bval);
      break;
    case IMAGE_ZORDER:
      pImageHook->SetZOrder((ushort)ival);
      break;
    case IMAGE_ONCLICK:
      pImageHook->SetClickHandler(val);
      break;
    case IMAGE_ONHOVER:
      pImageHook->SetHoverHandler(val);
      break;
  }
  return JS_TRUE;
}

JSAPI_FUNC(screenToAutomap) {
  if (argc == 1) {
    // the arg must be an object with an x and a y that we can convert
    if (JS_IsObject(argv[0])) {
      JSValue x = JS_GetPropertyStr(ctx, argv[0], "x");
      JSValue y = JS_GetPropertyStr(ctx, argv[0], "y");
      if (!JS_IsNumber(x) || !JS_IsNumber(y)) {
        THROW_ERROR(ctx, "Input has an x or y, but they aren't the correct type!");
      }

      int32_t ix, iy;
      if (JS_ToInt32(ctx, &ix, x) || JS_ToInt32(ctx, &iy, y)) {
        THROW_ERROR(ctx, "Failed to convert x and/or y values");
      }

      // convert the values
      POINT result = ScreenToAutomap(ix, iy);
      JSValue rval = JS_NewObject(ctx);
      JS_SetPropertyStr(ctx, argv[0], "x", JS_NewInt32(ctx, result.x));
      JS_SetPropertyStr(ctx, argv[0], "y", JS_NewInt32(ctx, result.y));
      return rval;
    } else
      THROW_ERROR(ctx, "Invalid object specified to screenToAutomap");
  } else if (argc == 2) {
    // the args must be ints
    if (JS_IsNumber(argv[0]) && JS_IsNumber(argv[1])) {
      int32_t ix, iy;
      if (JS_ToInt32(ctx, &ix, argv[0]) || JS_ToInt32(ctx, &iy, argv[1])) {
        THROW_ERROR(ctx, "Failed to convert x and/or y values");
      }

      // convert the values
      POINT result = ScreenToAutomap(ix, iy);
      JSValue rval = JS_NewObject(ctx);
      JS_SetPropertyStr(ctx, argv[0], "x", JS_NewInt32(ctx, result.x));
      JS_SetPropertyStr(ctx, argv[0], "y", JS_NewInt32(ctx, result.y));
      return rval;
    } else
      THROW_ERROR(ctx, "screenToAutomap expects two arguments to be two integers");
  }
  THROW_ERROR(ctx, "Invalid arguments specified for screenToAutomap");
}

// POINT result = {ix, iy};
// AutomapToScreen(&result);
JSAPI_FUNC(automapToScreen) {
  if (argc == 1) {
    // the arg must be an object with an x and a y that we can convert
    if (JS_IsObject(argv[0])) {
      JSValue x = JS_GetPropertyStr(ctx, argv[0], "x");
      JSValue y = JS_GetPropertyStr(ctx, argv[0], "y");
      if (!JS_IsNumber(x) || !JS_IsNumber(y)) {
        THROW_ERROR(ctx, "Input has an x or y, but they aren't the correct type!");
      }

      int32_t ix, iy;
      if (JS_ToInt32(ctx, &ix, x) || JS_ToInt32(ctx, &iy, y)) {
        THROW_ERROR(ctx, "Failed to convert x and/or y values");
      }

      // convert the values
      POINT result = {ix, iy};
      AutomapToScreen(&result);
      JSValue rval = JS_NewObject(ctx);
      JS_SetPropertyStr(ctx, argv[0], "x", JS_NewInt32(ctx, result.x));
      JS_SetPropertyStr(ctx, argv[0], "y", JS_NewInt32(ctx, result.y));
      return rval;
    } else
      THROW_ERROR(ctx, "Invalid object specified to screenToAutomap");
  } else if (argc == 2) {
    // the args must be ints
    if (JS_IsNumber(argv[0]) && JS_IsNumber(argv[1])) {
      int32_t ix, iy;
      if (JS_ToInt32(ctx, &ix, argv[0]) || JS_ToInt32(ctx, &iy, argv[1])) {
        THROW_ERROR(ctx, "Failed to convert x and/or y values");
      }

      // convert the values
      POINT result = {ix, iy};
      AutomapToScreen(&result);
      JSValue rval = JS_NewObject(ctx);
      JS_SetPropertyStr(ctx, argv[0], "x", JS_NewInt32(ctx, result.x));
      JS_SetPropertyStr(ctx, argv[0], "y", JS_NewInt32(ctx, result.y));
      return rval;
    } else {
      THROW_ERROR(ctx, "automapToScreen expects two arguments to be two integers");
    }
  }
  THROW_ERROR(ctx, "Invalid arguments specified for automapToScreen");
}
