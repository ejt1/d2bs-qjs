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
#include "D2Helpers.h" // Log
#include "Globals.h"
#include "File.h"

#include <direct.h>

JSValue DirectoryWrap::Instantiate(JSContext* ctx, JSValue new_target, const char* name) {
  JSValue proto;
  if (JS_IsUndefined(new_target)) {
    proto = JS_GetClassProto(ctx, m_class_id);
  } else {
    proto = JS_GetPropertyStr(ctx, new_target, "prototype");
    if (JS_IsException(proto)) {
      return JS_EXCEPTION;
    }
  }
  JSValue obj = JS_NewObjectProtoClass(ctx, proto, m_class_id);
  JS_FreeValue(ctx, proto);
  if (JS_IsException(obj)) {
    return obj;
  }

  DirectoryWrap* wrap = new DirectoryWrap(ctx, name);
  if (!wrap) {
    JS_FreeValue(ctx, obj);
    return JS_ThrowOutOfMemory(ctx);
  }
  JS_SetOpaque(obj, wrap);

  return obj;
}

void DirectoryWrap::Initialize(JSContext* ctx, JSValue target) {
  JSClassDef def{};
  def.class_name = "Folder";
  def.finalizer = [](JSRuntime* /*rt*/, JSValue val) {
    DirectoryWrap* wrap = static_cast<DirectoryWrap*>(JS_GetOpaque(val, m_class_id));
    if (wrap) {
      delete wrap;
    }
  };

  if (m_class_id == 0) {
    JS_NewClassID(&m_class_id);
  }
  JS_NewClass(JS_GetRuntime(ctx), m_class_id, &def);

  JSValue proto = JS_NewObject(ctx);
  JS_SetPropertyFunctionList(ctx, proto, m_proto_funcs, _countof(m_proto_funcs));

  JSValue obj = JS_NewObjectProtoClass(ctx, proto, m_class_id);
  JS_SetClassProto(ctx, m_class_id, proto);
  JS_SetPropertyStr(ctx, target, "Folder", obj);

  // globals
  JS_SetPropertyStr(ctx, target, "dopen", JS_NewCFunction(ctx, OpenDirectory, "dopen", 0));
}

DirectoryWrap::DirectoryWrap(JSContext* /*ctx*/, const char* name) {
  strcpy_s(szName, _MAX_PATH, name);
}

// properties
JSValue DirectoryWrap::GetName(JSContext* ctx, JSValue this_val) {
  DirectoryWrap* wrap = static_cast<DirectoryWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  return JS_NewString(ctx, wrap->szName);
}

// functions
JSValue DirectoryWrap::Create(JSContext* ctx, JSValue this_val, int /*argc*/, JSValue* argv) {
  DirectoryWrap* wrap = static_cast<DirectoryWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

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

  sprintf_s(path, _MAX_PATH, "%s\\%s\\%s", Vars.szScriptPath, wrap->szName, szName);
  if (_mkdir(path) == -1 && (errno == ENOENT)) {
    JS_ReportError(ctx, "Couldn't create directory %s, path %s not found", szName, path);
    JS_FreeCString(ctx, szName);
    return JS_FALSE;
  }

  JSValue rval = DirectoryWrap::Instantiate(ctx, JS_UNDEFINED, szName);
  JS_FreeCString(ctx, szName);
  return rval;
}

JSValue DirectoryWrap::Delete(JSContext* ctx, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  DirectoryWrap* wrap = static_cast<DirectoryWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  char path[_MAX_PATH];
  sprintf_s(path, _MAX_PATH, "%s\\%s", Vars.szScriptPath, wrap->szName);

  if (_rmdir(path) == -1) {
    // TODO: Make an optional param that specifies recursive delete
    if (errno == ENOTEMPTY)
      THROW_ERROR(ctx, "Tried to delete directory, but it is not empty or is the current working directory");
    if (errno == ENOENT)
      THROW_ERROR(ctx, "Path not found");
  }
  return JS_TRUE;
}

JSValue DirectoryWrap::GetFiles(JSContext* ctx, JSValue this_val, int argc, JSValue* argv) {
  DirectoryWrap* wrap = static_cast<DirectoryWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

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
  sprintf_s(path, _MAX_PATH, "%s\\%s", Vars.szScriptPath, wrap->szName);

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

JSValue DirectoryWrap::GetFolders(JSContext* ctx, JSValue this_val, int argc, JSValue* argv) {
  DirectoryWrap* wrap = static_cast<DirectoryWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

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
  sprintf_s(path, _MAX_PATH, "%s\\%s", Vars.szScriptPath, wrap->szName);

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

// globals
JSValue DirectoryWrap::OpenDirectory(JSContext* ctx, JSValue /*this_val*/, int argc, JSValue* argv) {
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
  
  JSValue rval = DirectoryWrap::Instantiate(ctx, JS_UNDEFINED, szName);
  JS_FreeCString(ctx, szName);
  return rval;
}

D2BS_BINDING_INTERNAL(DirectoryWrap, DirectoryWrap::Initialize)