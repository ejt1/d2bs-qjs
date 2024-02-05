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

#include <cstdio>
#include <io.h>
#include <errno.h>
#include <windows.h>
#include <Shlwapi.h>

#include <assert.h>

#include "JSFileTools.h"
#include "Engine.h"
#include "File.h"
#include "Helpers.h"

EMPTY_CTOR(filetools)

JSAPI_FUNC(filetools_remove) {
  if (argc < 1 || !JS_IsString(argv[0]))
    THROW_ERROR(ctx, "You must supply a file name");

  const char* szFile = JS_ToCString(ctx, argv[0]);
  if (!szFile) {
    return JS_EXCEPTION;
  }
  char fullpath[_MAX_PATH + _MAX_FNAME];

  if (getPathRelScript(szFile, _MAX_PATH + _MAX_FNAME, fullpath) == NULL) {
    JS_FreeCString(ctx, szFile);
    THROW_ERROR(ctx, "Invalid file name");
  }
  JS_FreeCString(ctx, szFile);

  remove(fullpath);
  return JS_UNDEFINED;
}

JSAPI_FUNC(filetools_rename) {
  if (argc < 1 || !JS_IsString(argv[0]))
    THROW_ERROR(ctx, "You must supply an original file name");
  if (argc < 2 || !JS_IsString(argv[1]))
    THROW_ERROR(ctx, "You must supply a new file name");

  const char* szOrig = JS_ToCString(ctx, argv[0]);
  const char* szNewName = JS_ToCString(ctx, argv[1]);
  if (!szOrig || !szNewName) {
    return JS_EXCEPTION;
  }

  char porig[_MAX_PATH + _MAX_FNAME];
  char pnewName[_MAX_PATH + _MAX_FNAME];

  if (getPathRelScript(szOrig, _MAX_PATH + _MAX_FNAME, porig) == NULL) {
    JS_FreeCString(ctx, szOrig);
    JS_FreeCString(ctx, szNewName);
    THROW_ERROR(ctx, "Invalid original file name");
  }

  if (getPathRelScript(szNewName, _MAX_PATH + _MAX_FNAME, pnewName) == NULL) {
    JS_FreeCString(ctx, szOrig);
    JS_FreeCString(ctx, szNewName);
    THROW_ERROR(ctx, "Invalid new file name");
  }
  JS_FreeCString(ctx, szOrig);
  JS_FreeCString(ctx, szNewName);

  // hehe, get rid of "ignoring return value" warning
  if (rename(porig, pnewName)) {
    return JS_UNDEFINED;
  }
  return JS_UNDEFINED;
}

JSAPI_FUNC(filetools_copy) {
  if (argc < 1 || !JS_IsString(argv[0]))
    THROW_ERROR(ctx, "You must supply an original file name");
  if (argc < 2 || !JS_IsString(argv[1]))
    THROW_ERROR(ctx, "You must supply a new file name");

  const char* szOrig = JS_ToCString(ctx, argv[0]);
  const char* szNewName = JS_ToCString(ctx, argv[1]);
  if (!szOrig || !szNewName) {
    return JS_EXCEPTION;
  }

  char pnewName[_MAX_PATH + _MAX_FNAME];
  bool overwrite = false;

  if (argc > 2 && JS_IsBool(argv[2]))
    overwrite = !!JS_ToBool(ctx, argv[2]);

  if (getPathRelScript(szNewName, _MAX_PATH + _MAX_FNAME, pnewName) == NULL) {
    JS_FreeCString(ctx, szOrig);
    JS_FreeCString(ctx, szNewName);
    THROW_ERROR(ctx, "Invalid new file name");
  }

  if (overwrite && _access(pnewName, 0) == 0) {
    JS_FreeCString(ctx, szOrig);
    JS_FreeCString(ctx, szNewName);
    return JS_UNDEFINED;
  }

  FILE* fptr1 = fileOpenRelScript(szOrig, "r", ctx);
  FILE* fptr2 = fileOpenRelScript(szNewName, "w", ctx);
  JS_FreeCString(ctx, szOrig);
  JS_FreeCString(ctx, szNewName);

  // If fileOpenRelScript failed, it already reported the error
  if (fptr1 == NULL || fptr2 == NULL)
    return JS_EXCEPTION;

  while (!feof(fptr1)) {
    int ch = fgetc(fptr1);
    if (ferror(fptr1)) {
      THROW_ERROR(ctx, "Read Error");
      break;
    } else {
      if (!feof(fptr1))
        fputc(ch, fptr2);
      if (ferror(fptr2)) {
        THROW_ERROR(ctx, "Write Error");
        break;
      }
    }
  }
  if (ferror(fptr1) || ferror(fptr2)) {
    clearerr(fptr1);
    clearerr(fptr2);
    fflush(fptr2);
    fclose(fptr2);
    fclose(fptr1);
    remove(pnewName);  // delete the partial file so it doesnt look like we succeeded
    THROW_ERROR(ctx, "File copy failed");
  }

  fflush(fptr2);
  fclose(fptr2);
  fclose(fptr1);
  return JS_UNDEFINED;
}

JSAPI_FUNC(filetools_exists) {
  if (argc < 1 || !JS_IsString(argv[0]))
    THROW_ERROR(ctx, "Invalid file name");

  const char* szFile = JS_ToCString(ctx, argv[0]);
  if (!szFile) {
    return JS_EXCEPTION;
  }

  char fullpath[_MAX_PATH + _MAX_FNAME];

  if (getPathRelScript(szFile, _MAX_PATH + _MAX_FNAME, fullpath) == NULL) {
    JS_FreeCString(ctx, szFile);
    THROW_ERROR(ctx, "Invalid file name");
  }
  JS_FreeCString(ctx, szFile);

  return JS_NewBool(ctx, !(_access(fullpath, 0) != 0 && errno == ENOENT));
}

JSAPI_FUNC(filetools_readText) {
  if (argc < 1 || !JS_IsString(argv[0]))
    THROW_ERROR(ctx, "You must supply a file name");

  const char* szFile = JS_ToCString(ctx, argv[0]);
  if (!szFile) {
    return JS_EXCEPTION;
  }

  FILE* fptr = fileOpenRelScript(szFile, "r", ctx);
  JS_FreeCString(ctx, szFile);

  // If fileOpenRelScript failed, it already reported the error
  if (fptr == NULL)
    return JS_EXCEPTION;

  uint32_t size, readCount;
  char* contents;

  // Determine file size
  fseek(fptr, 0, SEEK_END);
  size = ftell(fptr);
  rewind(fptr);

  // Allocate and read the string. Need to set last char to \0 since fread
  // doesn't.
  contents = new char[size + 1];
  readCount = fread(contents, sizeof(char), size, fptr);
  assert(readCount <= size);  // Avoid SEGFAULT
  contents[readCount] = 0;
  fclose(fptr);

  // Check to see if we had an error
  if (ferror(fptr)) {
    delete[] contents;
    THROW_ERROR(ctx, "Read failed");
  }

  int offset = 0;
  if (readCount > 2 && contents[0] == '\xEF' && contents[1] == '\xBB' && contents[2] == '\xBF') {  // skip BOM
    offset = 3;
  }

  // Convert to JSVAL cleanup and return
  JSValue rval = JS_NewString(ctx, contents + offset);
  delete[] contents;
  return rval;
}

JSAPI_FUNC(filetools_writeText) {
  if (argc < 1 || !JS_IsString(argv[0]))
    THROW_ERROR(ctx, "You must supply a file name");

  EnterCriticalSection(&Vars.cFileSection);

  const char* szFile = JS_ToCString(ctx, argv[0]);
  if (!szFile) {
    LeaveCriticalSection(&Vars.cFileSection);
    return JS_EXCEPTION;
  }

  FILE* fptr = fileOpenRelScript(szFile, "w", ctx);
  JS_FreeCString(ctx, szFile);
  bool result = true;

  // If fileOpenRelScript failed, it already reported the error
  if (fptr == NULL) {
    LeaveCriticalSection(&Vars.cFileSection);
    return JS_EXCEPTION;
  }

  for (int i = 1; i < argc; i++)
    if (!writeValue(fptr, ctx, argv[i], false, true))
      result = false;

  fflush(fptr);
  fclose(fptr);

  LeaveCriticalSection(&Vars.cFileSection);

  return JS_NewBool(ctx, result);
}

JSAPI_FUNC(filetools_appendText) {
  if (argc < 1 || !JS_IsString(argv[0]))
    THROW_ERROR(ctx, "You must supply a file name");

  EnterCriticalSection(&Vars.cFileSection);

  const char* szFile = JS_ToCString(ctx, argv[0]);
  if (!szFile) {
    LeaveCriticalSection(&Vars.cFileSection);
    return JS_EXCEPTION;
  }

  FILE* fptr = fileOpenRelScript(szFile, "a+", ctx);
  JS_FreeCString(ctx, szFile);
  bool result = true;

  // If fileOpenRelScript failed, it already reported the error
  if (fptr == NULL) {
    LeaveCriticalSection(&Vars.cFileSection);
    return JS_EXCEPTION;
  }

  for (int i = 1; i < argc; i++)
    if (!writeValue(fptr, ctx, argv[i], false, true))
      result = false;

  fflush(fptr);
  fclose(fptr);

  LeaveCriticalSection(&Vars.cFileSection);

  return JS_NewBool(ctx, result);
}
