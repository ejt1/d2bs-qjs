#ifndef __JSSCREENHOOK_H__
#define __JSSCREENHOOK_H__

#include "js32.h"

JSAPI_FUNC(hook_remove);
CLASS_FINALIZER(hook);

/*********************************************************
                                        Frame Header
**********************************************************/
JSAPI_FUNC(frame_ctor);
JSAPI_PROP(frame_getProperty);
JSAPI_STRICT_PROP(frame_setProperty);

enum frame_tinyid { FRAME_X, FRAME_Y, FRAME_XSIZE, FRAME_YSIZE, FRAME_VISIBLE, FRAME_ALIGN, FRAME_ONCLICK, FRAME_ONHOVER, FRAME_ZORDER };

static JSCFunctionListEntry frame_props[] = {
    JS_CGETSET_MAGIC_DEF("x", frame_getProperty, frame_setProperty, FRAME_X),
    JS_CGETSET_MAGIC_DEF("y", frame_getProperty, frame_setProperty, FRAME_Y),
    JS_CGETSET_MAGIC_DEF("xsize", frame_getProperty, frame_setProperty, FRAME_XSIZE),
    JS_CGETSET_MAGIC_DEF("ysize", frame_getProperty, frame_setProperty, FRAME_YSIZE),
    JS_CGETSET_MAGIC_DEF("visible", frame_getProperty, frame_setProperty, FRAME_VISIBLE),
    JS_CGETSET_MAGIC_DEF("align", frame_getProperty, frame_setProperty, FRAME_ALIGN),
    JS_CGETSET_MAGIC_DEF("zorder", frame_getProperty, frame_setProperty, FRAME_ZORDER),
    JS_CGETSET_MAGIC_DEF("click", frame_getProperty, frame_setProperty, FRAME_ONCLICK),
    JS_CGETSET_MAGIC_DEF("hover", frame_getProperty, frame_setProperty, FRAME_ONHOVER),
};

static JSCFunctionListEntry frame_methods[] = {
    JS_FS("remove", hook_remove, 0, FUNCTION_FLAGS),
};

/*********************************************************
                                        box Header
**********************************************************/
JSAPI_FUNC(box_ctor);
JSAPI_PROP(box_getProperty);
JSAPI_STRICT_PROP(box_setProperty);

enum box_tinyid { BOX_X, BOX_Y, BOX_XSIZE, BOX_YSIZE, BOX_COLOR, BOX_OPACITY, BOX_VISIBLE, BOX_ALIGN, BOX_ONCLICK, BOX_ONHOVER, BOX_ZORDER };

static JSCFunctionListEntry box_props[] = {
    JS_CGETSET_MAGIC_DEF("x", box_getProperty, box_setProperty, BOX_X),
    JS_CGETSET_MAGIC_DEF("y", box_getProperty, box_setProperty, BOX_Y),
    JS_CGETSET_MAGIC_DEF("xsize", box_getProperty, box_setProperty, BOX_XSIZE),
    JS_CGETSET_MAGIC_DEF("ysize", box_getProperty, box_setProperty, BOX_YSIZE),
    JS_CGETSET_MAGIC_DEF("visible", box_getProperty, box_setProperty, BOX_VISIBLE),
    JS_CGETSET_MAGIC_DEF("color", box_getProperty, box_setProperty, BOX_COLOR),
    JS_CGETSET_MAGIC_DEF("opacity", box_getProperty, box_setProperty, BOX_OPACITY),
    JS_CGETSET_MAGIC_DEF("align", box_getProperty, box_setProperty, BOX_ALIGN),
    JS_CGETSET_MAGIC_DEF("zorder", box_getProperty, box_setProperty, BOX_ZORDER),
    JS_CGETSET_MAGIC_DEF("click", box_getProperty, box_setProperty, BOX_ONCLICK),
    JS_CGETSET_MAGIC_DEF("hover", box_getProperty, box_setProperty, BOX_ONHOVER),
};

static JSCFunctionListEntry box_methods[] = {
    JS_FS("remove", hook_remove, 0, FUNCTION_FLAGS),
};

/*********************************************************
                                        Line Header
**********************************************************/
JSAPI_FUNC(line_ctor);
JSAPI_PROP(line_getProperty);
JSAPI_STRICT_PROP(line_setProperty);

enum line_tinyid { LINE_X, LINE_Y, LINE_XSIZE, LINE_YSIZE, LINE_COLOR, LINE_VISIBLE, LINE_ONCLICK, LINE_ONHOVER, LINE_ZORDER };

static JSCFunctionListEntry line_props[] = {
    JS_CGETSET_MAGIC_DEF("x", line_getProperty, line_setProperty, LINE_X),
    JS_CGETSET_MAGIC_DEF("y", line_getProperty, line_setProperty, LINE_Y),
    JS_CGETSET_MAGIC_DEF("x2", line_getProperty, line_setProperty, LINE_XSIZE),
    JS_CGETSET_MAGIC_DEF("y2", line_getProperty, line_setProperty, LINE_YSIZE),
    JS_CGETSET_MAGIC_DEF("visible", line_getProperty, line_setProperty, LINE_VISIBLE),
    JS_CGETSET_MAGIC_DEF("color", line_getProperty, line_setProperty, LINE_COLOR),
    JS_CGETSET_MAGIC_DEF("zorder", line_getProperty, line_setProperty, LINE_ZORDER),
    JS_CGETSET_MAGIC_DEF("click", line_getProperty, line_setProperty, LINE_ONCLICK),
    JS_CGETSET_MAGIC_DEF("hover", line_getProperty, line_setProperty, LINE_ONHOVER),
};

static JSCFunctionListEntry line_methods[] = {
    JS_FS("remove", hook_remove, 0, FUNCTION_FLAGS),
};

/*********************************************************
                                        Text Header
**********************************************************/
JSAPI_FUNC(text_ctor);
JSAPI_PROP(text_getProperty);
JSAPI_STRICT_PROP(text_setProperty);

enum text_tinyid { TEXT_X, TEXT_Y, TEXT_COLOR, TEXT_FONT, TEXT_TEXT, TEXT_ALIGN, TEXT_VISIBLE, TEXT_ONCLICK, TEXT_ONHOVER, TEXT_ZORDER };

static JSCFunctionListEntry text_props[] = {
    JS_CGETSET_MAGIC_DEF("x", text_getProperty, text_setProperty, TEXT_X),
    JS_CGETSET_MAGIC_DEF("y", text_getProperty, text_setProperty, TEXT_Y),
    JS_CGETSET_MAGIC_DEF("color", text_getProperty, text_setProperty, TEXT_COLOR),
    JS_CGETSET_MAGIC_DEF("font", text_getProperty, text_setProperty, TEXT_FONT),
    JS_CGETSET_MAGIC_DEF("visible", text_getProperty, text_setProperty, TEXT_VISIBLE),
    JS_CGETSET_MAGIC_DEF("text", text_getProperty, text_setProperty, TEXT_TEXT),
    JS_CGETSET_MAGIC_DEF("align", text_getProperty, text_setProperty, TEXT_ALIGN),
    JS_CGETSET_MAGIC_DEF("zorder", text_getProperty, text_setProperty, TEXT_ZORDER),
    JS_CGETSET_MAGIC_DEF("click", text_getProperty, text_setProperty, TEXT_ONCLICK),
    JS_CGETSET_MAGIC_DEF("hover", text_getProperty, text_setProperty, TEXT_ONHOVER),
};

static JSCFunctionListEntry text_methods[] = {
    JS_FS("remove", hook_remove, 0, FUNCTION_FLAGS),
};

/*********************************************************
                                        Image Header
**********************************************************/
JSAPI_FUNC(image_ctor);
JSAPI_PROP(image_getProperty);
JSAPI_STRICT_PROP(image_setProperty);

enum image_tinyid { IMAGE_X, IMAGE_Y, IMAGE_LOCATION, IMAGE_ALIGN, IMAGE_VISIBLE, IMAGE_ONCLICK, IMAGE_ONHOVER, IMAGE_ZORDER };

static JSCFunctionListEntry image_props[] = {
    JS_CGETSET_MAGIC_DEF("x", image_getProperty, image_setProperty, IMAGE_X),
    JS_CGETSET_MAGIC_DEF("y", image_getProperty, image_setProperty, IMAGE_Y),
    JS_CGETSET_MAGIC_DEF("visible", image_getProperty, image_setProperty, IMAGE_VISIBLE),
    JS_CGETSET_MAGIC_DEF("location", image_getProperty, image_setProperty, IMAGE_LOCATION),
    JS_CGETSET_MAGIC_DEF("align", image_getProperty, image_setProperty, IMAGE_ALIGN),
    JS_CGETSET_MAGIC_DEF("zorder", image_getProperty, image_setProperty, IMAGE_ZORDER),
    JS_CGETSET_MAGIC_DEF("click", image_getProperty, image_setProperty, IMAGE_ONCLICK),
    JS_CGETSET_MAGIC_DEF("hover", image_getProperty, image_setProperty, IMAGE_ONHOVER),
};

static JSCFunctionListEntry image_methods[] = {
    JS_FS("remove", hook_remove, 0, FUNCTION_FLAGS),
};

JSAPI_FUNC(screenToAutomap);
JSAPI_FUNC(automapToScreen);

#endif
