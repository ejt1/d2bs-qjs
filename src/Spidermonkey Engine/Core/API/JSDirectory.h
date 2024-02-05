#ifndef __JSDIRECTORY_H__
#define __JSDIRECTORY_H__

// TODO: Rewrite this mess of crap too

#include "js32.h"
#include <cstdlib>
#include <cstring>

CLASS_CTOR(dir);
CLASS_FINALIZER(dir);

JSAPI_PROP(dir_getProperty);

JSAPI_FUNC(dir_getFiles);
JSAPI_FUNC(dir_getFolders);
JSAPI_FUNC(dir_create);
JSAPI_FUNC(dir_delete);

JSAPI_FUNC(my_openDir);

class JSDirectory {
 public:
  char name[_MAX_FNAME];
  JSDirectory(const char* newname) {
    strcpy_s(name, _MAX_FNAME, newname);
  }
};

enum { DIR_NAME };

static JSCFunctionListEntry dir_proto_funcs[] = {
    JS_CGETSET_MAGIC_DEF("name", dir_getProperty, nullptr, DIR_NAME),

    JS_FS("create", dir_create, 1, FUNCTION_FLAGS),
    JS_FS("remove", dir_delete, 1, FUNCTION_FLAGS),
    JS_FS("getFiles", dir_getFiles, 1, FUNCTION_FLAGS),
    JS_FS("getFolders", dir_getFolders, 1, FUNCTION_FLAGS),
};

#endif
