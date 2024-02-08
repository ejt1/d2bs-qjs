#include "js32.h"
#include "JSFile.h"
#include "JSSocket.h"
#include "JSUnit.h"
#include "JSScreenHook.h"
#include "JSPresetUnit.h"
#include "JSDirectory.h"
#include "JSFileTools.h"
#include "JSArea.h"
#include "JSControl.h"
#include "JSParty.h"
#include "JSExits.h"
#include "JSRoom.h"
#include "JSScript.h"
#include "JSProfile.h"

JSClassID script_class_id;
JSClassID frame_class_id;
JSClassID box_class_id;
JSClassID line_class_id;
JSClassID text_class_id;
JSClassID image_class_id;
JSClassID room_class_id;
JSClassID presetunit_class_id;
JSClassID party_class_id;
JSClassID filetools_class_id;
JSClassID file_class_id;
JSClassID socket_class_id;
JSClassID exit_class_id;
JSClassID folder_class_id;
JSClassID control_class_id;
JSClassID area_class_id;
JSClassID unit_class_id;
JSClassID profile_class_id;
JSClassID profileType_class_id;
JSClassID dialogLine_class_id;

JSClassSpec global_classes[] = {
    {
        "Unit",
        &unit_class_id,
        unit_ctor,
        unit_finalizer,
        0,
        FUNCLIST(unit_proto_funcs),
        EMPTY_FUNCLIST,
    },
    {
        "Room",
        &room_class_id,
        room_ctor,
        nullptr,
        0,
        FUNCLIST(room_proto_funcs),
        EMPTY_FUNCLIST,
    },
    {
        "Socket",
        &socket_class_id,
        socket_ctor,
        socket_finalizer,
        0,
        FUNCLIST(socket_proto_funcs),
        FUNCLIST(socket_static_funcs),
    },

    {
        "Frame",
        &frame_class_id,
        frame_ctor,
        hook_finalizer,
        0,
        FUNCLIST(frame_proto_funcs),
        EMPTY_FUNCLIST,
    },
    {
        "Box",
        &box_class_id,
        box_ctor,
        hook_finalizer,
        0,
        FUNCLIST(box_proto_funcs),
        EMPTY_FUNCLIST,
    },
    {
        "Line",
        &line_class_id,
        line_ctor,
        hook_finalizer,
        0,
        FUNCLIST(line_proto_funcs),
        EMPTY_FUNCLIST,
    },
    {
        "Text",
        &text_class_id,
        text_ctor,
        hook_finalizer,
        0,
        FUNCLIST(text_proto_funcs),
        EMPTY_FUNCLIST,
    },
    {
        "Image",
        &image_class_id,
        image_ctor,
        hook_finalizer,
        0,
        FUNCLIST(image_proto_funcs),
        EMPTY_FUNCLIST,
    },

    {0},
};
