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
  DirData* d = (DirData*)JS_GetPrivate(val);
  delete d;
}

JSAPI_FUNC(my_openDir) {
  if (argc != 1)
    return JS_TRUE;

  wchar_t path[_MAX_PATH];
  const char* szName = JS_ToCString(ctx, argv[0]);
  const wchar_t* name = AnsiToUnicode(szName);
  JS_FreeCString(ctx, szName);

  if (!isValidPath(name)) {
    Log(L"The following path was deemed invalid: %s. (%s, %s)", name, L"JSDirectory.cpp", L"my_openDir");
    delete[] name;
    return JS_EXCEPTION;
  }

  swprintf_s(path, _MAX_PATH, L"%s\\%s", Vars.szScriptPath, name);

  if ((_wmkdir(path) == -1) && (errno == ENOENT)) {
    char* n = UnicodeToAnsi(name);
    char* p = UnicodeToAnsi(path);
    JS_ReportError(ctx, "Couldn't get directory %s, path '%s' not found", n, p);
    delete[] name;
    delete[] n;
    delete[] p;
    return JS_EXCEPTION;
  }
  DirData* d = new DirData(name);
  delete[] name;
  return BuildObject(ctx, folder_class_id, dir_methods, _countof(dir_methods), dir_props, _countof(dir_props), d);
}

////////////////////////////////////////////////////////////////////////////////
// dir_getFiles
// searches a directory for files with a specified extension(*.* assumed)
//
// Added by lord2800
////////////////////////////////////////////////////////////////////////////////

JSAPI_FUNC(dir_getFiles) {
  DirData* d = (DirData*)JS_GetPrivate(ctx, this_val);
  wchar_t* search;

  if (argc > 1)
    return JS_EXCEPTION;
  if (argc < 1) {
    search = new wchar_t[4];
    wcscpy(search, L"*.*");
  } else {
    const char* szSearch = JS_ToCString(ctx, argv[0]);
    search = AnsiToUnicode(szSearch);
    JS_FreeCString(ctx, szSearch);
  }

  if (!search) {
    THROW_ERROR(ctx, "Failed to get search string");
  }

  long hFile;
  wchar_t path[_MAX_PATH], oldpath[_MAX_PATH];
  swprintf_s(path, _MAX_PATH, L"%s\\%s", Vars.szScriptPath, d->name);

  if (!_wgetcwd(oldpath, _MAX_PATH)) {
    Log(L"Error getting current working directory. (%s, %s)", L"JSDirectory.cpp", L"dir_getFiles");
    delete[] search;
    return JS_EXCEPTION;
  }
  if (_wchdir(path) == -1) {
    Log(L"Changing directory to %s. (%s, %s)", path, L"JSDirectory.cpp", L"dir_getFiles");
    delete[] search;
    return JS_EXCEPTION;
  }

  _wfinddata_t found;
  JSValue jsarray = JS_NewArray(ctx);
  if ((hFile = _wfindfirst(search, &found)) != -1L) {
    jsint element = 0;
    do {
      if ((found.attrib & _A_SUBDIR))
        continue;
      JS_SetPropertyUint32(ctx, jsarray, element++, JS_NewString(ctx, found.name));
    } while (_wfindnext(hFile, &found) == 0);
  }

  if (_wchdir(oldpath) == -1) {
    Log(L"Error changing directory back to %s. (%s, %s)", oldpath, L"JSDirectory.cpp", L"dir_getFiles");
    delete[] search;
    return JS_EXCEPTION;
  }

  delete[] search;
  return jsarray;
}

JSAPI_FUNC(dir_getFolders) {
  DirData* d = (DirData*)JS_GetPrivate(ctx, this_val);
  wchar_t* search;

  if (argc > 1)
    return JS_EXCEPTION;
  if (argc < 1) {
    search = new wchar_t[4];
    wcscpy(search, L"*.*");
  } else {
    const char* szSearch = JS_ToCString(ctx, argv[0]);
    search = AnsiToUnicode(szSearch);
    JS_FreeCString(ctx, szSearch);
  }

  if (!search) {
    THROW_ERROR(ctx, "Failed to get search string");
  }

  long hFile;
  wchar_t path[_MAX_PATH], oldpath[_MAX_PATH];
  swprintf_s(path, _MAX_PATH, L"%s\\%s", Vars.szScriptPath, d->name);

  if (!_wgetcwd(oldpath, _MAX_PATH)) {
    Log(L"Error getting current working directory. (%s, %s)", L"JSDirectory.cpp", L"dir_getFolders");
    delete[] search;
    return JS_EXCEPTION;
  }
  if (_wchdir(path) == -1) {
    Log(L"Changing directory to %s. (%s, %s)", path, L"JSDirectory.cpp", L"dir_getFolders");
    delete[] search;
    return JS_EXCEPTION;
  }

  _wfinddata_t found;
  JSValue jsarray = JS_NewArray(ctx);

  if ((hFile = _wfindfirst(search, &found)) != -1L) {
    jsint element = 0;
    do {
      if (!wcscmp(found.name, L"..") || !wcscmp(found.name, L".") || !(found.attrib & _A_SUBDIR))
        continue;
      JS_SetPropertyUint32(ctx, jsarray, element++, JS_NewString(ctx, found.name));
    } while (_wfindnext(hFile, &found) == 0);
  }

  if (_wchdir(oldpath) == -1) {
    Log(L"Error changing directory back to %s. (%s, %s)", oldpath, L"JSDirectory.cpp", L"dir_getFolders");
    delete[] search;
    return JS_EXCEPTION;
  }

  return jsarray;
}

JSAPI_FUNC(dir_create) {
  DirData* d = (DirData*)JS_GetPrivate(ctx, this_val);
  wchar_t path[_MAX_PATH];
  if (!JS_IsString(argv[0]))
    THROW_ERROR(ctx, "No path passed to dir.create()");
  const char* szName = JS_ToCString(ctx, argv[0]);
  const wchar_t* name = AnsiToUnicode(szName);
  JS_FreeCString(ctx, szName);

  if (!isValidPath(name)) {
    delete[] name;
    return JS_EXCEPTION;
  }

  swprintf_s(path, _MAX_PATH, L"%s\\%s\\%s", Vars.szScriptPath, d->name, name);
  if (_wmkdir(path) == -1 && (errno == ENOENT)) {
    JS_ReportError(ctx, "Couldn't create directory %s, path %s not found", name, path);
    delete[] name;
    return JS_FALSE;
  }

  DirData* _d = new DirData(name);
  delete[] name;
  return BuildObject(ctx, folder_class_id, dir_methods, _countof(dir_methods), dir_props, _countof(dir_props), _d);
}

JSAPI_FUNC(dir_delete) {
  DirData* d = (DirData*)JS_GetPrivate(ctx, this_val);

  wchar_t path[_MAX_PATH];
  swprintf_s(path, _MAX_PATH, L"%s\\%s", Vars.szScriptPath, d->name);

  if (_wrmdir(path) == -1) {
    // TODO: Make an optional param that specifies recursive delete
    if (errno == ENOTEMPTY)
      THROW_ERROR(ctx, "Tried to delete directory, but it is not empty or is the current working directory");
    if (errno == ENOENT)
      THROW_ERROR(ctx, "Path not found");
  }
  return JS_TRUE;
}

JSAPI_PROP(dir_getProperty) {
  DirData* d = (DirData*)JS_GetPrivate(ctx, this_val);

  if (!d)
    return JS_EXCEPTION;

  switch (magic) {
    case DIR_NAME:
      return JS_NewString(ctx, d->name);
      break;
  }
  return JS_TRUE;
}