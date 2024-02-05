/*
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

// TODO: Rewrite this crap :(

#define _USE_32BIT_TIME_T

#include <direct.h>
#include <io.h>
#include <cerrno>

#include "JSDirectory.h"
#include "Engine.h"
#include "File.h"
#include "Helpers.h"
// #include "js32.h"

////////////////////////////////////////////////////////////////////////////////
// Directory stuff
////////////////////////////////////////////////////////////////////////////////

EMPTY_CTOR(dir)

CLASS_FINALIZER(dir) {
  JSDirectory* d = (JSDirectory*)JS_GetOpaque3(val);
  delete d;
}

JSAPI_FUNC(my_openDir) {
  if (argc != 1)
    return JS_UNDEFINED;

  char path[_MAX_PATH];
  const char* szName = JS_ToCString(ctx, argv[0]);
  if (!szName) {
    return JS_EXCEPTION;
  }

  if (!isValidPath(szName)) {
    Log("The following path was deemed invalid: %s. (%s, %s)", szName, "JSDirectory.cpp", "my_openDir");
    JS_FreeCString(ctx, szName);
    return JS_EXCEPTION;
  }

  sprintf_s(path, _MAX_PATH, "%s\\%s", Vars.szScriptPath, szName);

  if ((_mkdir(path) == -1) && (errno == ENOENT)) {
    JS_ReportError(ctx, "Couldn't get directory %s, path '%s' not found", szName, path);
    JS_FreeCString(ctx, szName);
    return JS_EXCEPTION;
  }
  JSDirectory* d = new JSDirectory(szName);
  JS_FreeCString(ctx, szName);
  return BuildObject(ctx, folder_class_id, FUNCLIST(dir_proto_funcs), d);
}

////////////////////////////////////////////////////////////////////////////////
// dir_getFiles
// searches a directory for files with a specified extension(*.* assumed)
//
// Added by lord2800
////////////////////////////////////////////////////////////////////////////////

JSAPI_FUNC(dir_getFiles) {
  JSDirectory* d = (JSDirectory*)JS_GetOpaque3(this_val);
  char search[_MAX_PATH] = "*.*";

  if (argc > 1)
    return JS_EXCEPTION;
  if (argc == 1) {
    const char* szSearch = JS_ToCString(ctx, argv[0]);
    if (!szSearch) {
      return JS_EXCEPTION;
    }
    strcpy_s(search, szSearch);
    JS_FreeCString(ctx, szSearch);
  }

  long hFile;
  char path[_MAX_PATH], oldpath[_MAX_PATH];
  sprintf_s(path, _MAX_PATH, "%s\\%s", Vars.szScriptPath, d->name);

  if (!_getcwd(oldpath, _MAX_PATH)) {
    Log("Error getting current working directory. (%s, %s)", "JSDirectory.cpp", "dir_getFiles");
    return JS_EXCEPTION;
  }
  if (_chdir(path) == -1) {
    Log("Changing directory to %s. (%s, %s)", path, "JSDirectory.cpp", "dir_getFiles");
    return JS_EXCEPTION;
  }

  _finddata_t found;
  JSValue jsarray = JS_NewArray(ctx);
  if ((hFile = _findfirst(search, &found)) != -1L) {
    int32_t element = 0;
    do {
      if ((found.attrib & _A_SUBDIR))
        continue;
      JS_SetPropertyUint32(ctx, jsarray, element++, JS_NewString(ctx, found.name));
    } while (_findnext(hFile, &found) == 0);
  }

  if (_chdir(oldpath) == -1) {
    Log("Error changing directory back to %s. (%s, %s)", oldpath, "JSDirectory.cpp", "dir_getFiles");
    return JS_EXCEPTION;
  }

  return jsarray;
}

JSAPI_FUNC(dir_getFolders) {
  JSDirectory* d = (JSDirectory*)JS_GetOpaque3(this_val);
  char search[_MAX_PATH] = "*.*";

  if (argc > 1)
    return JS_EXCEPTION;
  if (argc == 1) {
    const char* szSearch = JS_ToCString(ctx, argv[0]);
    if (!szSearch) {
      return JS_EXCEPTION;
    }
    strcpy_s(search, szSearch);
    JS_FreeCString(ctx, szSearch);
  }

  long hFile;
  char path[_MAX_PATH], oldpath[_MAX_PATH];
  sprintf_s(path, _MAX_PATH, "%s\\%s", Vars.szScriptPath, d->name);

  if (!_getcwd(oldpath, _MAX_PATH)) {
    Log("Error getting current working directory. (%s, %s)", "JSDirectory.cpp", "dir_getFolders");
    return JS_EXCEPTION;
  }
  if (_chdir(path) == -1) {
    Log("Changing directory to %s. (%s, %s)", path, "JSDirectory.cpp", "dir_getFolders");
    return JS_EXCEPTION;
  }

  _finddata_t found;
  JSValue jsarray = JS_NewArray(ctx);

  if ((hFile = _findfirst(search, &found)) != -1L) {
    int32_t element = 0;
    do {
      if (!strcmp(found.name, "..") || !strcmp(found.name, ".") || !(found.attrib & _A_SUBDIR))
        continue;
      JS_SetPropertyUint32(ctx, jsarray, element++, JS_NewString(ctx, found.name));
    } while (_findnext(hFile, &found) == 0);
  }

  if (_chdir(oldpath) == -1) {
    Log("Error changing directory back to %s. (%s, %s)", oldpath, "JSDirectory.cpp", "dir_getFolders");
    return JS_EXCEPTION;
  }

  return jsarray;
}

JSAPI_FUNC(dir_create) {
  JSDirectory* d = (JSDirectory*)JS_GetOpaque3(this_val);
  char path[_MAX_PATH];
  if (!JS_IsString(argv[0]))
    THROW_ERROR(ctx, "No path passed to dir.create()");

  const char* szName = JS_ToCString(ctx, argv[0]);
  if (!szName) {
    return JS_EXCEPTION;
  }

  if (!isValidPath(szName)) {
    JS_FreeCString(ctx, szName);
    return JS_EXCEPTION;
  }

  sprintf_s(path, _MAX_PATH, "%s\\%s\\%s", Vars.szScriptPath, d->name, szName);
  if (_mkdir(path) == -1 && (errno == ENOENT)) {
    JS_ReportError(ctx, "Couldn't create directory %s, path %s not found", szName, path);
    JS_FreeCString(ctx, szName);
    return JS_FALSE;
  }

  JSDirectory* _d = new JSDirectory(szName);
  JS_FreeCString(ctx, szName);
  return BuildObject(ctx, folder_class_id, FUNCLIST(dir_proto_funcs), _d);
}

JSAPI_FUNC(dir_delete) {
  JSDirectory* d = (JSDirectory*)JS_GetOpaque3(this_val);

  char path[_MAX_PATH];
  sprintf_s(path, _MAX_PATH, "%s\\%s", Vars.szScriptPath, d->name);

  if (_rmdir(path) == -1) {
    // TODO: Make an optional param that specifies recursive delete
    if (errno == ENOTEMPTY)
      THROW_ERROR(ctx, "Tried to delete directory, but it is not empty or is the current working directory");
    if (errno == ENOENT)
      THROW_ERROR(ctx, "Path not found");
  }
  return JS_TRUE;
}

JSAPI_PROP(dir_getProperty) {
  JSDirectory* d = (JSDirectory*)JS_GetOpaque3(this_val);

  if (!d)
    return JS_EXCEPTION;

  switch (magic) {
    case DIR_NAME:
      return JS_NewString(ctx, d->name);
      break;
  }
  return JS_UNDEFINED;
}