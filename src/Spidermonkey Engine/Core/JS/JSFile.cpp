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

#include "JSFile.h"

#include "Bindings.h"
#include "D2Helpers.h"
#include "File.h"
#include "StringWrap.h"

JSObject* FileWrap::Instantiate(JSContext* ctx, const FileData& data) {
  JS::RootedObject global(ctx, JS::CurrentGlobalOrNull(ctx));
  JS::RootedValue constructor_val(ctx);
  if (!JS_GetProperty(ctx, global, "File", &constructor_val)) {
    JS_ReportErrorASCII(ctx, "Could not find constructor object for File");
    return nullptr;
  }
  if (!constructor_val.isObject()) {
    JS_ReportErrorASCII(ctx, "File is not a constructor");
    return nullptr;
  }
  JS::RootedObject constructor(ctx, &constructor_val.toObject());

  JS::RootedObject obj(ctx, JS_New(ctx, constructor, JS::HandleValueArray::empty()));
  if (!obj) {
    JS_ReportErrorASCII(ctx, "Calling File constructor failed");
    return nullptr;
  }

  FileWrap* wrap = new FileWrap(ctx, obj, data);
  if (!wrap) {
    JS_ReportOutOfMemory(ctx);
    return nullptr;
  }

  return obj;
}

void FileWrap::Initialize(JSContext* ctx, JS::HandleObject target) {
  JS::RootedObject proto(ctx, JS_InitClass(ctx, target, nullptr, &m_class, trampoline<New>, 0, m_props, m_methods, nullptr, m_smethods));
  if (!proto) {
    Log("failed to initialize class Unit");
    return;
  }
}

FileWrap::FileWrap(JSContext* ctx, JS::HandleObject obj, const FileData& data) : BaseObject(ctx, obj), fdata(data) {
}

FileWrap::~FileWrap() {
  free(fdata.path);
  if (fdata.fptr) {
    if (fdata.locked) {
      _unlock_file(fdata.fptr);
#if DEBUG
      fdata.lockLocation = __FILE__;
      fdata.line = __LINE__;
#endif
      fclose(fdata.fptr);
    } else
      _fclose_nolock(fdata.fptr);
  }
}

void FileWrap::finalize(JSFreeOp* /*fop*/, JSObject* obj) {
  BaseObject* wrap = BaseObject::FromJSObject(obj);
  if (wrap) {
    delete wrap;
  }
}

bool FileWrap::New(JSContext* ctx, JS::CallArgs& args) {
  JS::RootedObject newObject(ctx, JS_NewObjectForConstructor(ctx, &m_class, args));
  if (!newObject) {
    THROW_ERROR(ctx, "failed to construct File");
  }
  // BUG(ejt): ask ejt
  JS_DefineProperties(ctx, newObject, m_props);

  args.rval().setObject(*newObject);
  return true;
}

// properties
bool FileWrap::GetReadable(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FileData* fdata = &wrap->fdata;
  args.rval().setBoolean(fdata->fptr && !feof(fdata->fptr) && !ferror(fdata->fptr));
  return true;
}

bool FileWrap::GetWriteable(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FileData* fdata = &wrap->fdata;
  args.rval().setBoolean(fdata->fptr && !ferror(fdata->fptr) && (fdata->mode % 3) > FILE_READ);
  return true;
}

bool FileWrap::GetSeekable(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FileData* fdata = &wrap->fdata;
  args.rval().setBoolean(fdata->fptr && !ferror(fdata->fptr));
  return true;
}

bool FileWrap::GetMode(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FileData* fdata = &wrap->fdata;
  args.rval().setInt32(fdata->mode % 3);
  return true;
}

bool FileWrap::GetBinaryMode(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FileData* fdata = &wrap->fdata;
  args.rval().setBoolean(fdata->mode > 2);
  return true;
}

bool FileWrap::GetLength(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FileData* fdata = &wrap->fdata;
  if (fdata->fptr) {
    args.rval().setInt32(_filelength(_fileno(fdata->fptr)));
  } else {
    args.rval().setInt32(0);  //= JSVAL_ZERO;
  }
  return true;
}

bool FileWrap::GetPath(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FileData* fdata = &wrap->fdata;
  args.rval().setString(JS_NewStringCopyZ(ctx, fdata->path + 1));
  return true;
}

bool FileWrap::GetPosition(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FileData* fdata = &wrap->fdata;
  if (fdata->fptr) {
    if (fdata->locked) {
      args.rval().setInt32(ftell(fdata->fptr));
    } else {
      args.rval().setInt32(_ftell_nolock(fdata->fptr));
    }
    return true;
  } else {
    args.rval().setInt32(0);  //= JSVAL_ZERO;
  }
  return true;
}

bool FileWrap::GetEOF(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FileData* fdata = &wrap->fdata;
  if (fdata->fptr)
    args.rval().setBoolean(!!feof(fdata->fptr));
  else
    args.rval().setBoolean(true);
  return true;
}

bool FileWrap::GetAccessed(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  struct _stat filestat = {0};
  FileData* fdata = &wrap->fdata;
  if (fdata->fptr) {
    _fstat(_fileno(fdata->fptr), &filestat);
    args.rval().setDouble((double)filestat.st_atime);
  } else {
    args.rval().setInt32(0);  //= JSVAL_ZERO;
  }
  return true;
}

bool FileWrap::GetCreated(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  struct _stat filestat = {0};
  FileData* fdata = &wrap->fdata;
  if (fdata->fptr) {
    _fstat(_fileno(fdata->fptr), &filestat);
    args.rval().setDouble((double)filestat.st_ctime);
  } else {
    args.rval().setInt32(0);
  }
  return true;
}

bool FileWrap::GetModified(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  struct _stat filestat = {0};
  FileData* fdata = &wrap->fdata;
  if (fdata->fptr) {
    _fstat(_fileno(fdata->fptr), &filestat);
    args.rval().setDouble((double)filestat.st_mtime);
  } else {
    args.rval().setInt32(0);
  }
  return true;
}

bool FileWrap::GetAutoFlush(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FileData* fdata = &wrap->fdata;
  args.rval().setBoolean(fdata->autoflush);
  return true;
}

bool FileWrap::SetAutoFlush(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());
  FileData* fdata = &wrap->fdata;
  if (args.length() > 0 && args[0].isBoolean())
    fdata->autoflush = args[0].toBoolean();
  args.rval().setUndefined();
  return true;
}

// functions
bool FileWrap::Close(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  FileData* fdata = &wrap->fdata;
  if (fdata) {
    if (fdata->fptr) {
      if (fdata->locked) {
        _unlock_file(fdata->fptr);
#if DEBUG
        fdata->lockLocation = __FILE__;
        fdata->line = __LINE__;
#endif
        if (!!fclose(fdata->fptr)) {
          THROW_ERROR(ctx, "Close failed");
        }
      } else if (!!_fclose_nolock(fdata->fptr)) {
        THROW_ERROR(ctx, "Close failed");
      }
      fdata->fptr = NULL;
    } else
      THROW_ERROR(ctx, "File is not open");
  }
  args.rval().set(args.thisv());
  return true;
}

bool FileWrap::Reopen(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  FileData* fdata = &wrap->fdata;
  if (fdata)
    if (!fdata->fptr) {
      static const char* modes[] = {"rt", "w+t", "a+t", "rb", "w+b", "a+b"};
      fdata->fptr = fileOpenRelScript(fdata->path, modes[fdata->mode], ctx);

      // If fileOpenRelScript failed, it already reported the error
      if (fdata->fptr == NULL)
        return false;

      if (fdata->locked) {
        _lock_file(fdata->fptr);
#if DEBUG
        fdata->lockLocation = __FILE__;
        fdata->line = __LINE__;
#endif
      }
    } else
      THROW_ERROR(ctx, "File is not closed");
  args.rval().set(args.thisv());
  return true;
}

bool FileWrap::Read(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  FileData* fdata = &wrap->fdata;
  if (fdata && fdata->fptr) {
    clearerr(fdata->fptr);
    int32_t count = 1;
    if (!(args.length() > 0 && !JS::ToInt32(ctx, args[0], &count)))
      THROW_ERROR(ctx, "Invalid arguments");

    if (fdata->mode > 2) {
      // binary mode
      int* result = new int[count + 1];
      memset(result, 0, count + 1);
      uint32_t size = 0;
      if (fdata->locked)
        size = fread(result, sizeof(int), count, fdata->fptr);
      else
        size = _fread_nolock(result, sizeof(int), count, fdata->fptr);

      if (size != (uint32_t)count && ferror(fdata->fptr)) {
        delete[] result;
        THROW_ERROR(ctx, "Read failed");
      }
      if (count == 1) {
        args.rval().setInt32(result[0]);
        return true;
      }
      else {
        JS::RootedObject arr(ctx, JS::NewArrayObject(ctx, 0));
        for (int i = 0; i < count; i++) {
          JS_SetElement(ctx, arr, i, result[i]);
        }
        args.rval().setObject(*arr);
        return true;
      }
    } else {
      uint32_t size = 0;
      int offset = 0;
      bool begin = false;

      if (fdata->locked)
        size = ftell(fdata->fptr);
      else
        size = _ftell_nolock(fdata->fptr);

      if (size == 0)
        begin = true;
      // text mode
      if (fdata->locked)
        fflush(fdata->fptr);
      else
        _fflush_nolock(fdata->fptr);

      char* result = new char[count + 1];
      memset(result, 0, count + 1);
      size = 0;
      if (fdata->locked)
        size = fread(result, sizeof(char), count, fdata->fptr);
      else
        size = _fread_nolock(result, sizeof(char), count, fdata->fptr);

      if (size != (uint32_t)count && ferror(fdata->fptr)) {
        delete[] result;
        THROW_ERROR(ctx, "Read failed");
      }

      if (begin && size > 2 && result[0] == '\xEF' && result[1] == '\xBB' && result[2] == '\xBF') {  // skip BOM
        offset = 3;
      }
      args.rval().setString(JS_NewStringCopyZ(ctx, result + offset));
      delete[] result;
      return true;
    }
  }
  args.rval().setUndefined();
  return true;
}

bool FileWrap::ReadLine(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  FileData* fdata = &wrap->fdata;
  if (fdata && fdata->fptr) {
    uint32_t size = 0;
    int offset = 0;
    bool begin = false;

    if (fdata->locked)
      size = ftell(fdata->fptr);
    else
      size = _ftell_nolock(fdata->fptr);

    if (size == 0)
      begin = true;

    char* line = readLine(fdata->fptr, fdata->locked);
    if (!line)
      THROW_ERROR(ctx, "Read failed");

    if (begin && strlen(line) > 2 && line[0] == '\xEF' && line[1] == '\xBB' && line[2] == '\xBF') {  // skip BOM
      offset = 3;
    }

    args.rval().setString(JS_NewStringCopyZ(ctx, line + offset));
    free(line);
    return true;
  }
  args.rval().setUndefined();
  return true;
}

bool FileWrap::ReadAllLines(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  FileData* fdata = &wrap->fdata;
  if (fdata && fdata->fptr) {
    JS::RootedObject arr(ctx, JS::NewArrayObject(ctx, 0));
    int i = 0;
    while (!feof(fdata->fptr)) {
      uint32_t size = 0;
      int offset = 0;
      bool begin = false;

      if (fdata->locked)
        size = ftell(fdata->fptr);
      else
        size = _ftell_nolock(fdata->fptr);

      if (size == 0)
        begin = true;

      char* line = readLine(fdata->fptr, fdata->locked);
      if (!line)
        THROW_ERROR(ctx, "Read failed");

      if (begin && strlen(line) > 2 && line[0] == '\xEF' && line[1] == '\xBB' && line[2] == '\xBF') {  // skip BOM
        offset = 3;
      }

      JS::RootedString val(ctx, JS_NewStringCopyZ(ctx, line + offset));
      JS_SetElement(ctx, arr, i++, val);
      free(line);
    }
    args.rval().setObject(*arr);
    return true;
  }
  args.rval().setUndefined();
  return true;
}

bool FileWrap::ReadAll(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  FileData* fdata = &wrap->fdata;
  if (fdata && fdata->fptr) {
    uint32_t size = 0;
    int offset = 0;
    bool begin = false;

    if (fdata->locked)
      size = ftell(fdata->fptr);
    else
      size = _ftell_nolock(fdata->fptr);

    if (size == 0) {
      begin = true;
    }

    if (fdata->locked)
      fseek(fdata->fptr, 0, SEEK_END);
    else
      _fseek_nolock(fdata->fptr, 0, SEEK_END);

    size = 0;
    if (fdata->locked)
      size = ftell(fdata->fptr);
    else
      size = _ftell_nolock(fdata->fptr);

    if (fdata->locked)
      fseek(fdata->fptr, 0, SEEK_SET);
    else
      _fseek_nolock(fdata->fptr, 0, SEEK_SET);

    char* contents = new char[size + 1];
    memset(contents, 0, size + 1);
    uint32_t count = 0;
    if (fdata->locked)
      count = fread(contents, sizeof(char), size, fdata->fptr);
    else
      count = _fread_nolock(contents, sizeof(char), size, fdata->fptr);

    if (count != size && ferror(fdata->fptr)) {
      delete[] contents;
      THROW_ERROR(ctx, "Read failed");
    }
    if (begin && count > 2 && contents[0] == '\xEF' && contents[1] == '\xBB' && contents[2] == '\xBF') {  // skip BOM
      offset = 3;
    }
    args.rval().setString(JS_NewStringCopyZ(ctx, contents));
    delete[] contents;
    return true;
  }
  args.rval().setUndefined();
  return true;
}

bool FileWrap::Write(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  FileData* fdata = &wrap->fdata;
  if (fdata && fdata->fptr) {
    for (uint32_t i = 0; i < args.length(); i++) {
      writeValue(fdata->fptr, ctx, args[i], !!(fdata->mode > 2), fdata->locked);
    }

    if (fdata->autoflush) {
      if (fdata->locked)
        fflush(fdata->fptr);
      else
        _fflush_nolock(fdata->fptr);
    }
  }
  args.rval().set(args.thisv());
  return true;
}

bool FileWrap::Seek(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  FileData* fdata = &wrap->fdata;
  if (fdata && fdata->fptr) {
    if (args.length() > 0) {
      int32_t bytes;
      bool isLines = false, fromStart = false;
      if (!JS::ToInt32(ctx, args[0], &bytes)) {
        THROW_ERROR(ctx, "Could not convert parameter 1");
      }
      if (args.length() > 1 && args[1].isBoolean())
        isLines = args[1].toBoolean();
      if (args.length() > 2 && args[2].isBoolean())
        fromStart = args[2].toBoolean();

      if (fromStart)
        rewind(fdata->fptr);

      if (!isLines) {
        if (fdata->locked && fseek(fdata->fptr, bytes, SEEK_CUR)) {
          THROW_ERROR(ctx, "Seek failed");
        } else if (_fseek_nolock(fdata->fptr, bytes, SEEK_CUR))
          THROW_ERROR(ctx, "Seek failed");
      } else {
        // semi-ugly hack to seek to the specified line
        // if I were unlazy I wouldn't be allocating/deallocating all this memory, but for now it's ok
        while (bytes--) free(readLine(fdata->fptr, fdata->locked));
      }
    } else
      THROW_ERROR(ctx, "Not enough parameters");
  }
  args.rval().set(args.thisv());
  return true;
}

bool FileWrap::Flush(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  FileData* fdata = &wrap->fdata;
  if (fdata && fdata->fptr)
    if (fdata->locked)
      fflush(fdata->fptr);
    else
      _fflush_nolock(fdata->fptr);

  args.rval().set(args.thisv());
  return true;
}

bool FileWrap::Reset(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  FileData* fdata = &wrap->fdata;
  if (fdata && fdata->fptr) {
    if (fdata->locked && fseek(fdata->fptr, 0L, SEEK_SET)) {
      THROW_ERROR(ctx, "Seek failed");
    } else if (_fseek_nolock(fdata->fptr, 0L, SEEK_SET))
      THROW_ERROR(ctx, "Seek failed");
  }
  args.rval().set(args.thisv());
  return true;
}

bool FileWrap::End(JSContext* ctx, JS::CallArgs& args) {
  FileWrap* wrap;
  UNWRAP_OR_RETURN(ctx, &wrap, args.thisv());

  FileData* fdata = &wrap->fdata;
  if (fdata && fdata->fptr) {
    if (fdata->locked && fseek(fdata->fptr, 0L, SEEK_END)) {
      THROW_ERROR(ctx, "Seek failed");
    } else if (_fseek_nolock(fdata->fptr, 0L, SEEK_END))
      THROW_ERROR(ctx, "Seek failed");
  }

  args.rval().set(args.thisv());
  return true;
}

// static functions
bool FileWrap::Open(JSContext* ctx, JS::CallArgs& args) {
  if (args.length() < 2)
    THROW_ERROR(ctx, "Not enough parameters, 2 or more expected");
  if (!args[0].isString())
    THROW_ERROR(ctx, "Parameter 1 not a string");
  if (!args[1].isInt32())
    THROW_ERROR(ctx, "Parameter 2 not a mode");

  // Convert from JS params to C values
  StringWrap szFile(ctx, args[0]);
  if (!szFile) {
    return false;
  }

  int32_t mode;
  if (!JS::ToInt32(ctx, args[1], &mode)) {
    return false;
  }
  bool binary = false;
  bool autoflush = false;
  bool lockFile = false;
  if (args.length() > 2 && args[2].isBoolean())
    binary = args[2].toBoolean();
  if (args.length() > 3 && args[3].isBoolean())
    autoflush = args[3].toBoolean();
  if (args.length() > 4 && args[4].isBoolean())
    lockFile = args[4].toBoolean();

  // Check that the path looks basically ok, validation is handled later
  if (szFile[0] == '\0') {
    THROW_ERROR(ctx, "Invalid file name");
  }

  // this could be simplified to: mode > FILE_APPEND || mode < FILE_READ
  // but then it takes a hit for readability
  switch (mode) {
    // Good modes
    case FILE_READ:
    case FILE_WRITE:
    case FILE_APPEND:
      break;
    // Bad modes
    default:
      THROW_ERROR(ctx, "Invalid file mode");
  }

  if (binary)
    mode += 3;

  static const char* modes[] = {"rt", "w+t", "a+t", "rb", "w+b", "a+b"};
  FILE* fptr = fileOpenRelScript(szFile, modes[mode], ctx);

  // If fileOpenRelScript failed, it already reported the error
  if (fptr == NULL) {
    return false;
  }

  FileData data{};
  data.mode = mode;
  data.path = _strdup(szFile);
  data.autoflush = autoflush;
  data.locked = lockFile;
  data.fptr = fptr;
  if (lockFile) {
    _lock_file(fptr);
#if DEBUG
    data.lockLocation = __FILE__;
    data.line = __LINE__;
#endif
  }

  JS::RootedObject rval(ctx, FileWrap::Instantiate(ctx, data));
  if (!rval) {
    if (lockFile)
      fclose(fptr);
    else
      _fclose_nolock(fptr);
    free(data.path);
    return false;
  }
  args.rval().setObject(*rval);
  return true;
}

D2BS_BINDING_INTERNAL(FileWrap, FileWrap::Initialize)
