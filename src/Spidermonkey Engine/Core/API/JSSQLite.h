#ifndef SQLITE_H
#define SQLITE_H

#include "js32.h"
#include "sqlite3.h"
#include "JSGlobalClasses.h"

JSAPI_FUNC(my_sqlite_version);
JSAPI_FUNC(my_sqlite_memusage);

JSAPI_FUNC(sqlite_ctor);
CLASS_FINALIZER(sqlite);
JSAPI_FUNC(sqlite_execute);
JSAPI_FUNC(sqlite_query);
JSAPI_FUNC(sqlite_close);
JSAPI_FUNC(sqlite_open);
JSAPI_PROP(sqlite_getProperty);

CLASS_CTOR(sqlite_stmt);
CLASS_FINALIZER(sqlite_stmt);

JSAPI_FUNC(sqlite_stmt_getobject);
JSAPI_FUNC(sqlite_stmt_colcount);
JSAPI_FUNC(sqlite_stmt_colval);
JSAPI_FUNC(sqlite_stmt_colname);
JSAPI_FUNC(sqlite_stmt_execute);
JSAPI_FUNC(sqlite_stmt_next);
JSAPI_FUNC(sqlite_stmt_reset);
JSAPI_FUNC(sqlite_stmt_skip);
JSAPI_FUNC(sqlite_stmt_close);
JSAPI_FUNC(sqlite_stmt_bind);
JSAPI_PROP(sqlite_stmt_getProperty);

enum { SQLITE_PATH, SQLITE_STMTS, SQLITE_OPEN, SQLITE_LASTROWID, SQLITE_CHANGES };

enum { SQLITE_STMT_SQL, SQLITE_STMT_READY };

static JSFunctionSpec sqlite_methods[] = {
    JS_FS("execute", sqlite_execute, 1, FUNCTION_FLAGS),
    JS_FS("query", sqlite_query, 1, FUNCTION_FLAGS),
    JS_FS("open", sqlite_open, 0, FUNCTION_FLAGS),
    JS_FS("close", sqlite_close, 0, FUNCTION_FLAGS),
};

static JSPropertySpec sqlite_props[] = {
    JS_CGETSET_MAGIC_DEF("path", sqlite_getProperty, nullptr, SQLITE_PATH),       JS_CGETSET_MAGIC_DEF("statements", sqlite_getProperty, nullptr, SQLITE_STMTS),
    JS_CGETSET_MAGIC_DEF("isOpen", sqlite_getProperty, nullptr, SQLITE_OPEN),     JS_CGETSET_MAGIC_DEF("lastRowId", sqlite_getProperty, nullptr, SQLITE_LASTROWID),
    JS_CGETSET_MAGIC_DEF("changes", sqlite_getProperty, nullptr, SQLITE_CHANGES),
};

static JSFunctionSpec sqlite_stmt_methods[] = {
    JS_FS("getObject", sqlite_stmt_getobject, 0, FUNCTION_FLAGS),
    JS_FS("getColumnCount", sqlite_stmt_colcount, 0, FUNCTION_FLAGS),
    JS_FS("getColumnValue", sqlite_stmt_colval, 1, FUNCTION_FLAGS),
    JS_FS("getColumnName", sqlite_stmt_colname, 1, FUNCTION_FLAGS),
    JS_FS("go", sqlite_stmt_execute, 0, FUNCTION_FLAGS),
    JS_FS("next", sqlite_stmt_next, 0, FUNCTION_FLAGS),
    JS_FS("skip", sqlite_stmt_skip, 1, FUNCTION_FLAGS),
    JS_FS("reset", sqlite_stmt_reset, 0, FUNCTION_FLAGS),
    JS_FS("close", sqlite_stmt_close, 0, FUNCTION_FLAGS),
    JS_FS("bind", sqlite_stmt_bind, 2, FUNCTION_FLAGS),
};

static JSPropertySpec sqlite_stmt_props[] = {
    JS_CGETSET_MAGIC_DEF("sql", sqlite_stmt_getProperty, nullptr, SQLITE_STMT_SQL),
    JS_CGETSET_MAGIC_DEF("ready", sqlite_stmt_getProperty, nullptr, SQLITE_STMT_READY),
};

#endif SQLITE_H
