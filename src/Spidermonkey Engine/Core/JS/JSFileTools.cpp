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

#include "JSFileTools.h"

#include "Bindings.h"
#include "D2Helpers.h"
#include "File.h"
#include "Globals.h"
#include "StringWrap.h"

void FileToolsWrap::Initialize(JSContext* ctx, JS::HandleObject target) {
  static JSFunctionSpec smethods[] = {
      JS_FN("remove", trampoline<Remove>, 1, JSPROP_ENUMERATE),          //
      JS_FN("rename", trampoline<Rename>, 2, JSPROP_ENUMERATE),          //
      JS_FN("copy", trampoline<Copy>, 2, JSPROP_ENUMERATE),              //
      JS_FN("exists", trampoline<Exists>, 1, JSPROP_ENUMERATE),          //
      JS_FN("readText", trampoline<ReadText>, 1, JSPROP_ENUMERATE),      //
      JS_FN("writeText", trampoline<WriteText>, 2, JSPROP_ENUMERATE),    //
      JS_FN("appendText", trampoline<AppendText>, 2, JSPROP_ENUMERATE),  //
      JS_FS_END,
  };

  JS::RootedObject proto(ctx, JS_InitClass(ctx, target, nullptr, &m_class, trampoline<New>, 0, nullptr, nullptr, nullptr, smethods));
  if (!proto) {
    Log("failed to initialize FileTools");
    return;
  }
}

bool FileToolsWrap::New(JSContext* ctx, JS::CallArgs& /*args*/) {
  THROW_ERROR(ctx, "FileTools constructor is not callable");
}

// static functions
bool FileToolsWrap::Remove(JSContext* ctx, JS::CallArgs& args) {
  if (args.length() < 1 || !args[0].isString()) {
    JS_ReportErrorASCII(ctx, "You must supply a file name");
    return false;
  }

  StringWrap szFile(ctx, args[0]);
  if (!szFile) {
    THROW_ERROR(ctx, "failed to encode string");
  }
  char fullpath[_MAX_PATH + _MAX_FNAME];

  if (getPathRelScript(szFile, _MAX_PATH + _MAX_FNAME, fullpath) == NULL) {
    JS_ReportErrorASCII(ctx, "Invalid file name");
    return false;
  }

  remove(fullpath);
  args.rval().setUndefined();
  return true;
}

bool FileToolsWrap::Rename(JSContext* ctx, JS::CallArgs& args) {
  if (args.length() < 1 || !args[0].isString()) {
    THROW_ERROR(ctx, "You must supply an original file name");
  }
  if (args.length() < 2 || !args[1].isString())
    THROW_ERROR(ctx, "You must supply a new file name");

  StringWrap szOrig(ctx, args[0]);
  StringWrap szNewName(ctx, args[1]);
  if (!szOrig || !szNewName) {
    THROW_ERROR(ctx, "failed to encode string");
  }

  char porig[_MAX_PATH + _MAX_FNAME];
  char pnewName[_MAX_PATH + _MAX_FNAME];

  if (getPathRelScript(szOrig, _MAX_PATH + _MAX_FNAME, porig) == NULL) {
    THROW_ERROR(ctx, "Invalid original file name");
  }

  if (getPathRelScript(szNewName, _MAX_PATH + _MAX_FNAME, pnewName) == NULL) {
    THROW_ERROR(ctx, "Invalid new file name");
  }

  // hehe, get rid of "ignoring return value" warning
  rename(porig, pnewName);
  args.rval().setUndefined();
  return true;
}

bool FileToolsWrap::Copy(JSContext* ctx, JS::CallArgs& args) {
  if (args.length() < 1 || !args[0].isString()) {
    THROW_ERROR(ctx, "You must supply an original file name");
  }
  if (args.length() < 2 || !args[1].isString())
    THROW_ERROR(ctx, "You must supply a new file name");

  StringWrap szOrig(ctx, args[0]);
  StringWrap szNewName(ctx, args[0]);
  if (!szOrig || !szNewName) {
    THROW_ERROR(ctx, "failed to encode string");
  }

  char pnewName[_MAX_PATH + _MAX_FNAME];
  bool overwrite = false;

  if (args.length() > 2 && args[2].isBoolean())
    overwrite = args[2].toBoolean();

  if (getPathRelScript(szNewName, _MAX_PATH + _MAX_FNAME, pnewName) == NULL) {
    THROW_ERROR(ctx, "Invalid new file name");
  }

  if (overwrite && _access(pnewName, 0) == 0) {
    THROW_ERROR(ctx, "permission denied");
  }

  FILE* fptr1 = fileOpenRelScript(szOrig, "r", ctx);
  FILE* fptr2 = fileOpenRelScript(szNewName, "w", ctx);

  // If fileOpenRelScript failed, it already reported the error
  if (fptr1 == NULL || fptr2 == NULL)
    return false;

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
  args.rval().setUndefined();
  return true;
}

bool FileToolsWrap::Exists(JSContext* ctx, JS::CallArgs& args) {
  if (args.length() < 1 || !args[0].isString())
    THROW_ERROR(ctx, "Invalid file name");

  StringWrap szFile(ctx, args[0]);
  if (!szFile) {
    THROW_ERROR(ctx, "failed to encode string");
  }

  char fullpath[_MAX_PATH + _MAX_FNAME];

  if (getPathRelScript(szFile, _MAX_PATH + _MAX_FNAME, fullpath) == NULL) {
    THROW_ERROR(ctx, "Invalid file name");
  }

  args.rval().setBoolean(!(_access(fullpath, 0) != 0 && errno == ENOENT));
  return true;
}

bool FileToolsWrap::ReadText(JSContext* ctx, JS::CallArgs& args) {
  if (args.length() < 1 || !args[0].isString())
    THROW_ERROR(ctx, "You must supply a file name");

  StringWrap szFile(ctx, args[0]);
  if (!szFile) {
    THROW_ERROR(ctx, "failed to encode string");
  }

  FILE* fptr = fileOpenRelScript(szFile, "r", ctx);

  // If fileOpenRelScript failed, it already reported the error
  if (fptr == NULL)
    return false;

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
  args.rval().setString(JS_NewStringCopyZ(ctx, contents + offset));
  delete[] contents;
  return true;
}

bool FileToolsWrap::WriteText(JSContext* ctx, JS::CallArgs& args) {
  if (args.length() < 1 || !args[0].isString())
    THROW_ERROR(ctx, "You must supply a file name");

  EnterCriticalSection(&Vars.cFileSection);

  StringWrap szFile(ctx, args[0]);
  if (!szFile) {
    LeaveCriticalSection(&Vars.cFileSection);
    THROW_ERROR(ctx, "failed to encode string");
  }

  FILE* fptr = fileOpenRelScript(szFile, "w", ctx);
  bool result = true;

  // If fileOpenRelScript failed, it already reported the error
  if (fptr == NULL) {
    LeaveCriticalSection(&Vars.cFileSection);
    THROW_ERROR(ctx, "fptr == NULL");
  }

  for (uint32_t i = 1; i < args.length(); i++)
    if (!writeValue(fptr, ctx, args[i], false, true))
      result = false;

  fflush(fptr);
  fclose(fptr);

  LeaveCriticalSection(&Vars.cFileSection);

  args.rval().setBoolean(result);
  return true;
}

bool FileToolsWrap::AppendText(JSContext* ctx, JS::CallArgs& args) {
  if (args.length() < 1 || !args[0].isString())
    THROW_ERROR(ctx, "You must supply a file name");

  EnterCriticalSection(&Vars.cFileSection);

  StringWrap szFile(ctx, args[0]);
  if (!szFile) {
    LeaveCriticalSection(&Vars.cFileSection);
    THROW_ERROR(ctx, "failed to encode string");
  }

  FILE* fptr = fileOpenRelScript(szFile, "a+", ctx);
  bool result = true;

  // If fileOpenRelScript failed, it already reported the error
  if (fptr == NULL) {
    LeaveCriticalSection(&Vars.cFileSection);
    return false;
  }

  for (uint32_t i = 1; i < args.length(); i++)
    if (!writeValue(fptr, ctx, args[i], false, true))
      result = false;

  fflush(fptr);
  fclose(fptr);

  LeaveCriticalSection(&Vars.cFileSection);
  args.rval().setBoolean(result);
  return true;
}

D2BS_BINDING_INTERNAL(FileToolsWrap, FileToolsWrap::Initialize)
