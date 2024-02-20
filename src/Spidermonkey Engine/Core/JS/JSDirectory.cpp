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

#include "JSDirectory.h"

#include "Bindings.h"
#include "D2Helpers.h"  // Log
#include "Globals.h"
#include "File.h"
#include "StringWrap.h"

#include <direct.h>

JSObject* DirectoryWrap::Instantiate(JSContext* ctx, const char* name) {
  JS::RootedObject global(ctx, JS::CurrentGlobalOrNull(ctx));
  JS::RootedValue constructor_val(ctx);
  if (!JS_GetProperty(ctx, global, "Folder", &constructor_val)) {
    JS_ReportErrorASCII(ctx, "Could not find constructor object for Folder");
    return nullptr;
  }
  if (!constructor_val.isObject()) {
    JS_ReportErrorASCII(ctx, "Folder is not a constructor");
    return nullptr;
  }
  JS::RootedObject constructor(ctx, &constructor_val.toObject());

  JS::RootedObject obj(ctx, JS_New(ctx, constructor, JS::HandleValueArray::empty()));
  if (!obj) {
    JS_ReportErrorASCII(ctx, "Calling Folder constructor failed");
    return nullptr;
  }
  DirectoryWrap* wrap = new DirectoryWrap(ctx, obj, name);
  if (!wrap) {
    JS_ReportOutOfMemory(ctx);
    return nullptr;
  }
  return obj;
}

void DirectoryWrap::Initialize(JSContext* ctx, JS::HandleObject target) {
  static JSPropertySpec props[] = {
      JS_PSG("name", trampoline<GetName>, JSPROP_ENUMERATE),
      JS_PS_END,
  };
  static JSFunctionSpec methods[] = {
      JS_FN("create", trampoline<Create>, 1, JSPROP_ENUMERATE),          //
      JS_FN("remove", trampoline<Delete>, 1, JSPROP_ENUMERATE),          //
      JS_FN("getFiles", trampoline<GetFiles>, 1, JSPROP_ENUMERATE),      //
      JS_FN("getFolders", trampoline<GetFolders>, 1, JSPROP_ENUMERATE),  //
      JS_FS_END,
  };

  JS::RootedObject proto(ctx, JS_InitClass(ctx, target, nullptr, &m_class, trampoline<New>, 0, props, methods, nullptr, nullptr));
  if (!proto) {
    return;
  }

  // globals
  JS_DefineFunction(ctx, target, "dopen", trampoline<OpenDirectory>, 0, JSPROP_ENUMERATE);
}

DirectoryWrap::DirectoryWrap(JSContext* ctx, JS::HandleObject obj, const char* name) : BaseObject(ctx, obj) {
  strcpy_s(szName, _MAX_PATH, name);
}

void DirectoryWrap::finalize(JSFreeOp* /*fop*/, JSObject* obj) {
  BaseObject* wrap = BaseObject::FromJSObject(obj);
  if (wrap) {
    delete wrap;
  }
}

bool DirectoryWrap::New(JSContext* ctx, JS::CallArgs& args) {
  JS::RootedObject newObject(ctx, JS_NewObjectForConstructor(ctx, &m_class, args));
  if (!newObject) {
    THROW_ERROR(ctx, "failed to instantiate folder");
  }
  args.rval().setObject(*newObject);
  return true;
}

// properties
bool DirectoryWrap::GetName(JSContext* ctx, JS::CallArgs& args) {
  DirectoryWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  args.rval().setString(JS_NewStringCopyZ(ctx, wrap->szName));
  return true;
}

// functions
bool DirectoryWrap::Create(JSContext* ctx, JS::CallArgs& args) {
  DirectoryWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  char path[_MAX_PATH];
  if (!args[0].isString())
    THROW_ERROR(ctx, "No path passed to dir.create()");

  StringWrap szName(ctx, args[0]);
  if (!szName) {
    THROW_ERROR(ctx, "failed to encode string");
  }

  if (!isValidPath(szName)) {
    THROW_ERROR(ctx, "invalid path");
  }

  sprintf_s(path, _MAX_PATH, "%s\\%s\\%s", Vars.szScriptPath, wrap->szName, szName.c_str());
  if (_mkdir(path) == -1 && (errno == ENOENT)) {
    JS_ReportErrorASCII(ctx, "Couldn't create directory %s, path %s not found", szName.c_str(), path);
    return false;
  }
  args.rval().setObjectOrNull(DirectoryWrap::Instantiate(ctx, szName));
  return true;
}

bool DirectoryWrap::Delete(JSContext* ctx, JS::CallArgs& args) {
  DirectoryWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  char path[_MAX_PATH];
  sprintf_s(path, _MAX_PATH, "%s\\%s", Vars.szScriptPath, wrap->szName);

  if (_rmdir(path) == -1) {
    // TODO: Make an optional param that specifies recursive delete
    if (errno == ENOTEMPTY)
      THROW_ERROR(ctx, "Tried to delete directory, but it is not empty or is the current working directory");
    if (errno == ENOENT)
      THROW_ERROR(ctx, "Path not found");
  }
  args.rval().setBoolean(true);
  return true;
}

bool DirectoryWrap::GetFiles(JSContext* ctx, JS::CallArgs& args) {
  DirectoryWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  char search[_MAX_PATH] = "*.*";

  if (args.length() > 1)
    THROW_ERROR(ctx, "not enough arguments");
  if (args.length() == 1 && args[0].isString()) {
    StringWrap szSearch(ctx, args[0]);
    if (!szSearch) {
      THROW_ERROR(ctx, "failed to encode string");
    }
    strcpy_s(search, szSearch.c_str());
  }

  long hFile;
  char path[_MAX_PATH], oldpath[_MAX_PATH];
  sprintf_s(path, _MAX_PATH, "%s\\%s", Vars.szScriptPath, wrap->szName);

  if (!_getcwd(oldpath, _MAX_PATH)) {
    Log("Error getting current working directory. (%s, %s)", "JSDirectory.cpp", "dir_getFiles");
    THROW_ERROR(ctx, "failed to get working directory");
  }
  if (_chdir(path) == -1) {
    Log("Changing directory to %s. (%s, %s)", path, "JSDirectory.cpp", "dir_getFiles");
    THROW_ERROR(ctx, "failed to change directory");
  }

  _finddata_t found;
  JS::RootedObject jsarray(ctx, JS::NewArrayObject(ctx, 0));
  if ((hFile = _findfirst(search, &found)) != -1L) {
    int32_t element = 0;
    do {
      if ((found.attrib & _A_SUBDIR))
        continue;
      JS::RootedString name_val(ctx, JS_NewStringCopyZ(ctx, found.name));
      JS_SetElement(ctx, jsarray, element++, name_val);
    } while (_findnext(hFile, &found) == 0);
  }

  if (_chdir(oldpath) == -1) {
    Log("Error changing directory back to %s. (%s, %s)", oldpath, "JSDirectory.cpp", "dir_getFiles");
    THROW_ERROR(ctx, "failed to change directory");
  }

  args.rval().setObject(*jsarray);
  return true;
}

bool DirectoryWrap::GetFolders(JSContext* ctx, JS::CallArgs& args) {
  DirectoryWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  char search[_MAX_PATH] = "*.*";

  if (args.length() > 1)
    THROW_ERROR(ctx, "not enough arguments");
  if (args.length() == 1 && args[0].isString()) {
    StringWrap szSearch(ctx, args[0]);
    if (!szSearch) {
      THROW_ERROR(ctx, "failed to encode string");
    }
    strcpy_s(search, szSearch.c_str());
  }

  long hFile;
  char path[_MAX_PATH], oldpath[_MAX_PATH];
  sprintf_s(path, _MAX_PATH, "%s\\%s", Vars.szScriptPath, wrap->szName);

  if (!_getcwd(oldpath, _MAX_PATH)) {
    Log("Error getting current working directory. (%s, %s)", "JSDirectory.cpp", "dir_getFolders");
    THROW_ERROR(ctx, "failed to get working directory");
  }
  if (_chdir(path) == -1) {
    Log("Changing directory to %s. (%s, %s)", path, "JSDirectory.cpp", "dir_getFolders");
    THROW_ERROR(ctx, "failed to change directory");
  }

  _finddata_t found;

  JS::RootedObject jsarray(ctx, JS::NewArrayObject(ctx, 0));
  if ((hFile = _findfirst(search, &found)) != -1L) {
    int32_t element = 0;
    do {
      if (!strcmp(found.name, "..") || !strcmp(found.name, ".") || !(found.attrib & _A_SUBDIR))
        continue;
      JS::RootedString name_val(ctx, JS_NewStringCopyZ(ctx, found.name));
      JS_SetElement(ctx, jsarray, element++, name_val);
    } while (_findnext(hFile, &found) == 0);
  }

  if (_chdir(oldpath) == -1) {
    Log("Error changing directory back to %s. (%s, %s)", oldpath, "JSDirectory.cpp", "dir_getFolders");
    THROW_ERROR(ctx, "failed to change directory");
  }

  args.rval().setObject(*jsarray);
  return true;
}

// globals
bool DirectoryWrap::OpenDirectory(JSContext* ctx, JS::CallArgs& args) {
  if (args.length() != 1) {
    args.rval().setUndefined();
    return true;
  }

  char path[_MAX_PATH];
  StringWrap szName(ctx, args[0]);
  if (!szName) {
    THROW_ERROR(ctx, "failed to encode string");
  }

  if (!isValidPath(szName)) {
    Log("The following path was deemed invalid: %s. (%s, %s)", szName.c_str(), "JSDirectory.cpp", "my_openDir");
    THROW_ERROR(ctx, "invalid path");
  }

  sprintf_s(path, _MAX_PATH, "%s\\%s", Vars.szScriptPath, szName.c_str());

  if ((_mkdir(path) == -1) && (errno == ENOENT)) {
    JS_ReportErrorASCII(ctx, "Couldn't get directory %s, path '%s' not found", szName.c_str(), path);
    return false;
  }
  JS::RootedObject obj(ctx, DirectoryWrap::Instantiate(ctx, szName));
  if (!obj) {
    THROW_ERROR(ctx, "failed to instantiate folder");
  }
  args.rval().setObject(*obj);
  return true;
}

D2BS_BINDING_INTERNAL(DirectoryWrap, DirectoryWrap::Initialize)
