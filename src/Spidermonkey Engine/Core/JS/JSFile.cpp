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
#include "File.h"

JSValue FileWrap::Instantiate(JSContext* ctx, JSValue new_target, const FileData& data) {
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

  FileWrap* wrap = new FileWrap(ctx, data);
  if (!wrap) {
    JS_FreeValue(ctx, obj);
    return JS_ThrowOutOfMemory(ctx);
  }
  JS_SetOpaque(obj, wrap);

  return obj;
}

void FileWrap::Initialize(JSContext* ctx, JSValue target) {
  JSClassDef def{};
  def.class_name = "File";
  def.finalizer = [](JSRuntime* /*rt*/, JSValue val) {
    FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(val, m_class_id));
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
  JS_SetPropertyFunctionList(ctx, obj, m_static_funcs, _countof(m_static_funcs));
  JS_SetClassProto(ctx, m_class_id, proto);
  JS_SetPropertyStr(ctx, target, "File", obj);
}

FileWrap::FileWrap(JSContext* /*ctx*/, const FileData& data) : fdata(data) {
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

// properties
JSValue FileWrap::GetReadable(JSContext* ctx, JSValue this_val) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FileData* fdata = &wrap->fdata;
  return JS_NewBool(ctx, (fdata->fptr && !feof(fdata->fptr) && !ferror(fdata->fptr)));
}

JSValue FileWrap::GetWriteable(JSContext* ctx, JSValue this_val) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FileData* fdata = &wrap->fdata;
  return JS_NewBool(ctx, (fdata->fptr && !ferror(fdata->fptr) && (fdata->mode % 3) > FILE_READ));
}

JSValue FileWrap::GetSeekable(JSContext* ctx, JSValue this_val) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FileData* fdata = &wrap->fdata;
  return JS_NewBool(ctx, (fdata->fptr && !ferror(fdata->fptr)));
}

JSValue FileWrap::GetMode(JSContext* ctx, JSValue this_val) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FileData* fdata = &wrap->fdata;
  return JS_NewInt32(ctx, (fdata->mode % 3));
}

JSValue FileWrap::GetBinaryMode(JSContext* ctx, JSValue this_val) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FileData* fdata = &wrap->fdata;
  return JS_NewBool(ctx, (fdata->mode > 2));
}

JSValue FileWrap::GetLength(JSContext* ctx, JSValue this_val) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FileData* fdata = &wrap->fdata;
  if (fdata->fptr)
    return JS_NewInt32(ctx, _filelength(_fileno(fdata->fptr)));

  return JS_NewInt32(ctx, 0);  //= JSVAL_ZERO;
}

JSValue FileWrap::GetPath(JSContext* ctx, JSValue this_val) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FileData* fdata = &wrap->fdata;
  return JS_NewString(ctx, fdata->path + 1);
}

JSValue FileWrap::GetPosition(JSContext* ctx, JSValue this_val) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FileData* fdata = &wrap->fdata;
  if (fdata->fptr) {
    if (fdata->locked)
      return JS_NewInt32(ctx, ftell(fdata->fptr));

    return JS_NewInt32(ctx, _ftell_nolock(fdata->fptr));
  }

  return JS_NewInt32(ctx, 0);  //= JSVAL_ZERO;
}

JSValue FileWrap::GetEOF(JSContext* ctx, JSValue this_val) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FileData* fdata = &wrap->fdata;
  if (fdata->fptr)
    return JS_NewBool(ctx, !!feof(fdata->fptr));
  else
    return JS_TRUE;
}

JSValue FileWrap::GetAccessed(JSContext* ctx, JSValue this_val) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  struct _stat filestat = {0};
  FileData* fdata = &wrap->fdata;
  if (fdata->fptr) {
    _fstat(_fileno(fdata->fptr), &filestat);
    return JS_NewFloat64(ctx, (double)filestat.st_atime);
  }

  return JS_NewInt32(ctx, 0);  //= JSVAL_ZERO;
}

JSValue FileWrap::GetCreated(JSContext* ctx, JSValue this_val) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  struct _stat filestat = {0};
  FileData* fdata = &wrap->fdata;
  if (fdata->fptr) {
    _fstat(_fileno(fdata->fptr), &filestat);
    return JS_NewFloat64(ctx, (double)filestat.st_ctime);
  }

  return JS_NewInt32(ctx, 0);
}

JSValue FileWrap::GetModified(JSContext* ctx, JSValue this_val) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  struct _stat filestat = {0};
  FileData* fdata = &wrap->fdata;
  if (fdata->fptr) {
    _fstat(_fileno(fdata->fptr), &filestat);
    return JS_NewFloat64(ctx, (double)filestat.st_mtime);
  }

  return JS_NewInt32(ctx, 0);
}

JSValue FileWrap::GetAutoFlush(JSContext* ctx, JSValue this_val) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FileData* fdata = &wrap->fdata;
  return JS_NewBool(ctx, fdata->autoflush);
}

JSValue FileWrap::SetAutoFlush(JSContext* ctx, JSValue this_val, JSValue val) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FileData* fdata = &wrap->fdata;
  if (JS_IsBool(val))
    fdata->autoflush = !!JS_ToBool(ctx, val);
  return JS_UNDEFINED;
}

// functions
JSValue FileWrap::Close(JSContext* ctx, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

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
  // BUG(ejt): possible value leak
  return JS_DupValue(ctx, this_val);
}

JSValue FileWrap::Reopen(JSContext* ctx, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FileData* fdata = &wrap->fdata;
  if (fdata)
    if (!fdata->fptr) {
      static const char* modes[] = {"rt", "w+t", "a+t", "rb", "w+b", "a+b"};
      fdata->fptr = fileOpenRelScript(fdata->path, modes[fdata->mode], ctx);

      // If fileOpenRelScript failed, it already reported the error
      if (fdata->fptr == NULL)
        return JS_EXCEPTION;

      if (fdata->locked) {
        _lock_file(fdata->fptr);
#if DEBUG
        fdata->lockLocation = __FILE__;
        fdata->line = __LINE__;
#endif
      }
    } else
      THROW_ERROR(ctx, "File is not closed");
  // BUG(ejt): possible value leak
  return JS_DupValue(ctx, this_val);
}

JSValue FileWrap::Read(JSContext* ctx, JSValue this_val, int argc, JSValue* argv) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FileData* fdata = &wrap->fdata;
  if (fdata && fdata->fptr) {
    clearerr(fdata->fptr);
    int32_t count = 1;
    if (!(argc > 0 && JS_ToInt32(ctx, &count, argv[0])))
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
      if (count == 1)
        return JS_NewInt32(ctx, result[0]);
      else {
        JSValue arr = JS_NewArray(ctx);
        for (int i = 0; i < count; i++) {
          JS_SetPropertyUint32(ctx, arr, i, JS_NewInt32(ctx, result[i]));
        }
        return arr;
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
      JSValue rval = JS_NewString(ctx, result + offset);
      delete[] result;
      return rval;
    }
  }
  return JS_UNDEFINED;
}

JSValue FileWrap::ReadLine(JSContext* ctx, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

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

    JSValue rval = JS_NewString(ctx, line + offset);
    free(line);
    return rval;
  }
  return JS_UNDEFINED;
}

JSValue FileWrap::ReadAllLines(JSContext* ctx, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FileData* fdata = &wrap->fdata;
  if (fdata && fdata->fptr) {
    JSValue arr = JS_NewArray(ctx);
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

      JS_SetPropertyUint32(ctx, arr, i++, JS_NewString(ctx, line + offset));
      free(line);
    }
    return arr;
  }
  return JS_UNDEFINED;
}

JSValue FileWrap::ReadAll(JSContext* ctx, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

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
    JSValue rval = JS_NewString(ctx, contents);
    delete[] contents;
    return rval;
  }
  return JS_UNDEFINED;
}

JSValue FileWrap::Write(JSContext* ctx, JSValue this_val, int argc, JSValue* argv) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FileData* fdata = &wrap->fdata;
  if (fdata && fdata->fptr) {
    for (int i = 0; i < argc; i++) writeValue(fdata->fptr, ctx, argv[i], !!(fdata->mode > 2), fdata->locked);

    if (fdata->autoflush) {
      if (fdata->locked)
        fflush(fdata->fptr);
      else
        _fflush_nolock(fdata->fptr);
    }
  }
  // BUG(ejt): possible value leak
  return JS_DupValue(ctx, this_val);
}

JSValue FileWrap::Seek(JSContext* ctx, JSValue this_val, int argc, JSValue* argv) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FileData* fdata = &wrap->fdata;
  if (fdata && fdata->fptr) {
    if (argc > 0) {
      int32_t bytes;
      bool isLines = false, fromStart = false;
      if (JS_ToInt32(ctx, &bytes, argv[0])) {
        THROW_ERROR(ctx, "Could not convert parameter 1");
      }
      if (argc > 1 && JS_IsBool(argv[1]))
        isLines = !!JS_ToBool(ctx, argv[1]);
      if (argc > 2 && JS_IsBool(argv[2]))
        fromStart = !!JS_ToBool(ctx, argv[2]);

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
  // BUG(ejt): possible value leak
  return JS_DupValue(ctx, this_val);
}

JSValue FileWrap::Flush(JSContext* ctx, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FileData* fdata = &wrap->fdata;
  if (fdata && fdata->fptr)
    if (fdata->locked)
      fflush(fdata->fptr);
    else
      _fflush_nolock(fdata->fptr);

  // BUG(ejt): possible value leak
  return JS_DupValue(ctx, this_val);
}

JSValue FileWrap::Reset(JSContext* ctx, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FileData* fdata = &wrap->fdata;
  if (fdata && fdata->fptr) {
    if (fdata->locked && fseek(fdata->fptr, 0L, SEEK_SET)) {
      THROW_ERROR(ctx, "Seek failed");
    } else if (_fseek_nolock(fdata->fptr, 0L, SEEK_SET))
      THROW_ERROR(ctx, "Seek failed");
  }
  // BUG(ejt): possible value leak
  return JS_DupValue(ctx, this_val);
}

JSValue FileWrap::End(JSContext* ctx, JSValue this_val, int /*argc*/, JSValue* /*argv*/) {
  FileWrap* wrap = static_cast<FileWrap*>(JS_GetOpaque(this_val, m_class_id));
  if (!wrap) {
    return JS_EXCEPTION;
  }

  FileData* fdata = &wrap->fdata;
  if (fdata && fdata->fptr) {
    if (fdata->locked && fseek(fdata->fptr, 0L, SEEK_END)) {
      THROW_ERROR(ctx, "Seek failed");
    } else if (_fseek_nolock(fdata->fptr, 0L, SEEK_END))
      THROW_ERROR(ctx, "Seek failed");
  }
  // BUG(ejt): possible value leak
  return JS_DupValue(ctx, this_val);
}

// static functions
JSValue FileWrap::Open(JSContext* ctx, JSValue /*this_val*/, int argc, JSValue* argv) {
  if (argc < 2)
    THROW_ERROR(ctx, "Not enough parameters, 2 or more expected");
  if (!JS_IsString(argv[0]))
    THROW_ERROR(ctx, "Parameter 1 not a string");
  if (!JS_IsNumber(argv[1]))
    THROW_ERROR(ctx, "Parameter 2 not a mode");

  // Convert from JS params to C values
  const char* szFile = JS_ToCString(ctx, argv[0]);
  if (!szFile) {
    return JS_EXCEPTION;
  }

  int32_t mode;
  if (JS_ToInt32(ctx, &mode, argv[1])) {
    JS_FreeCString(ctx, szFile);
    return JS_EXCEPTION;
  }
  bool binary = false;
  bool autoflush = false;
  bool lockFile = false;
  if (argc > 2 && JS_IsBool(argv[2]))
    binary = !!JS_ToBool(ctx, argv[2]);
  if (argc > 3 && JS_IsBool(argv[3]))
    autoflush = !!JS_ToBool(ctx, argv[3]);
  if (argc > 4 && JS_IsBool(argv[4]))
    lockFile = !!JS_ToBool(ctx, argv[4]);

  // Check that the path looks basically ok, validation is handled later
  if (szFile[0] == '\0') {
    JS_FreeCString(ctx, szFile);
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
      JS_FreeCString(ctx, szFile);
      THROW_ERROR(ctx, "Invalid file mode");
  }

  if (binary)
    mode += 3;

  static const char* modes[] = {"rt", "w+t", "a+t", "rb", "w+b", "a+b"};
  FILE* fptr = fileOpenRelScript(szFile, modes[mode], ctx);

  // If fileOpenRelScript failed, it already reported the error
  if (fptr == NULL) {
    JS_FreeCString(ctx, szFile);
    return JS_EXCEPTION;
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
  JS_FreeCString(ctx, szFile);

  JSValue rval = FileWrap::Instantiate(ctx, JS_UNDEFINED, data);
  if (JS_IsException(rval)) {
    if (lockFile)
      fclose(fptr);
    else
      _fclose_nolock(fptr);
    free(data.path);
    return JS_EXCEPTION;
  }
  return rval;
}

D2BS_BINDING_INTERNAL(FileWrap, FileWrap::Initialize)