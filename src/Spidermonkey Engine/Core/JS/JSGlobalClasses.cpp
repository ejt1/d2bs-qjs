#include "js32.h"
#include "JSFile.h"
#include "JSSocket.h"
#include "JSSQLite.h"
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

JSClassID sqlite_db_class_id;
JSClassID sqlite_stmt_class_id;
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

JSClassDef sqlite_db = {
    .class_name = "SQLite",
    .finalizer = sqlite_finalizer,
};

JSClassDef sqlite_stmt = {
    .class_name = "DBStatement",
    .finalizer = sqlite_stmt_finalizer,
};

JSClassDef script_class = {
    .class_name = "D2BSScript",
};

JSClassDef frame_class = {
    .class_name = "Frame",
    .finalizer = hook_finalizer,
};

JSClassDef box_class = {
    .class_name = "Box",
    .finalizer = hook_finalizer,
};

JSClassDef line_class = {
    .class_name = "Line",
    .finalizer = hook_finalizer,
};

JSClassDef text_class = {
    .class_name = "Text",
    .finalizer = hook_finalizer,
};

JSClassDef image_class = {
    .class_name = "Image",
    .finalizer = hook_finalizer,
};

JSClassDef room_class = {
    .class_name = "Room",
};

JSClassDef presetunit_class = {
    .class_name = "PresetUnit",
    .finalizer = presetunit_finalizer,
};

JSClassDef party_class = {
    .class_name = "Party",
};

JSClassDef filetools_class = {
    .class_name = "FileTools",
};

JSClassDef file_class = {
    .class_name = "File",
    .finalizer = file_finalizer,
};

JSClassDef socket_class = {
    .class_name = "Socket",
    .finalizer = socket_finalizer,
};

JSClassDef exit_class = {.class_name = "Exit", .finalizer = exit_finalizer};

JSClassDef folder_class = {
    .class_name = "Folder",
    .finalizer = dir_finalizer,
};

JSClassDef control_class = {
    .class_name = "Control",
    .finalizer = control_finalizer,
};

JSClassDef area_class = {
    .class_name = "Area",
    .finalizer = area_finalizer,
};

JSClassDef unit_class = {
    .class_name = "Unit",
    .finalizer = unit_finalizer,
};

JSClassDef profile_class = {
    .class_name = "Profile",
    .finalizer = profile_finalizer,
};

JSClassDef profileType_class = {
    .class_name = "ProfileType",
};

JSClassDef dialogLine_class = {
    .class_name = "DailogLine",
};

JSClassSpec global_classes[] = {
    {&unit_class_id, &unit_class, 0, unit_ctor, 0, unit_methods, _countof(unit_methods), unit_props, _countof(unit_props), NULL, 0, NULL, 0},
    {&presetunit_class_id, &presetunit_class, 0, presetunit_ctor, 0, NULL, 0, presetunit_props, _countof(presetunit_props), NULL, 0, NULL, 0},
    {&area_class_id, &area_class, 0, area_ctor, 0, NULL, 0, area_props, _countof(area_props), NULL, 0, NULL, 0},
    {&control_class_id, &control_class, 0, control_ctor, 0, control_funcs, _countof(control_funcs), control_props, _countof(control_props), NULL, 0, NULL, 0},
    {&folder_class_id, &folder_class, 0, dir_ctor, 0, dir_methods, _countof(dir_methods), dir_props, _countof(dir_props), NULL, 0, NULL, 0},
    {&exit_class_id, &exit_class, 0, exit_ctor, 0, NULL, 0, exit_props, _countof(exit_props), NULL, 0, NULL, 0},
    {&party_class_id, &party_class, 0, party_ctor, 0, party_methods, _countof(party_methods), party_props, _countof(party_props), NULL, 0, NULL, 0},
    {&room_class_id, &room_class, 0, room_ctor, 0, room_methods, _countof(room_methods), room_props, _countof(room_props), NULL, 0, NULL, 0},

    {&file_class_id, &file_class, 0, file_ctor, 0, file_methods, _countof(file_methods), file_props, _countof(file_props), file_s_methods, _countof(file_s_methods), NULL,
     0},
    {&socket_class_id, &socket_class, 0, socket_ctor, 0, socket_methods, _countof(socket_methods), socket_props, _countof(socket_props), socket_s_methods,
     _countof(socket_s_methods), NULL, 0},
    {&filetools_class_id, &filetools_class, 0, filetools_ctor, 0, NULL, 0, NULL, 0, filetools_s_methods, _countof(filetools_s_methods), NULL, 0},
    {&sqlite_db_class_id, &sqlite_db, 0, sqlite_ctor, 0, sqlite_methods, _countof(sqlite_methods), sqlite_props, _countof(sqlite_props), NULL, 0, NULL, 0},
    {&sqlite_stmt_class_id, &sqlite_stmt, 0, sqlite_stmt_ctor, 0, sqlite_stmt_methods, _countof(sqlite_stmt_methods), sqlite_stmt_props, _countof(sqlite_stmt_props),
     NULL, 0, NULL, 0},
    {&script_class_id, &script_class, 0, script_ctor, 0, script_methods, _countof(script_methods), script_props, _countof(script_props), NULL, 0, NULL, 0},

    {&frame_class_id, &frame_class, 0, frame_ctor, 0, frame_methods, _countof(frame_methods), frame_props, _countof(frame_props), NULL, 0, NULL, 0},
    {&box_class_id, &box_class, 0, box_ctor, 0, box_methods, _countof(box_methods), box_props, _countof(box_props), NULL, 0, NULL, 0},
    {&line_class_id, &line_class, 0, line_ctor, 0, line_methods, _countof(line_methods), line_props, _countof(line_props), NULL, 0, NULL, 0},
    {&text_class_id, &text_class, 0, text_ctor, 0, text_methods, _countof(text_methods), text_props, _countof(text_props), NULL, 0, NULL, 0},
    {&image_class_id, &image_class, 0, image_ctor, 0, image_methods, _countof(image_methods), image_props, _countof(image_props), NULL, 0, NULL, 0},
    {&profile_class_id, &profile_class, 0, profile_ctor, 0, profile_methods, _countof(profile_methods), profile_props, _countof(profile_props), NULL, 0, NULL, 0},
    {0},

};
