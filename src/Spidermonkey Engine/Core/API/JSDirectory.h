#ifndef __JSDIRECTORY_H__
#define __JSDIRECTORY_H__

// TODO: Rewrite this mess of crap too

#include "js32.h"
#include <cstdlib>
#include <cstring>

CLASS_CTOR(dir);
CLASS_FINALIZER(dir);

JSAPI_FUNC(dir_getFiles);
JSAPI_FUNC(dir_getFolders);
JSAPI_FUNC(dir_create);
JSAPI_FUNC(dir_delete);
JSAPI_FUNC(my_openDir);

JSAPI_PROP(dir_getProperty);

//////////////////////////////////////////////////////////////////
// directory stuff
//////////////////////////////////////////////////////////////////

enum { DIR_NAME };

static JSCFunctionListEntry dir_props[] = {
    JS_CGETSET_MAGIC_DEF("name", dir_getProperty, nullptr, DIR_NAME),
};

static JSCFunctionListEntry dir_methods[] = {
    JS_FS("create", dir_create, 1, FUNCTION_FLAGS),
    JS_FS("remove", dir_delete, 1, FUNCTION_FLAGS),
    JS_FS("getFiles", dir_getFiles, 1, FUNCTION_FLAGS),
    JS_FS("getFolders", dir_getFolders, 1, FUNCTION_FLAGS),
};

class DirData {
 public:
  char name[_MAX_FNAME];
  DirData(const char* newname) {
    strcpy_s(name, _MAX_FNAME, newname);
  }
};

#endif
