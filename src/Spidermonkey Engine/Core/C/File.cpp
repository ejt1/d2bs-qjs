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
#include <cstring>
#include <cstdlib>

#include <string>

#include <Windows.h>

#include "File.h"
#include "Engine.h"
#include "Helpers.h"

char* readLine(FILE* fptr, bool locking) {
  if (feof(fptr))
    return NULL;
  std::string buffer;
  char c = 0;
  // grab all the characters in this line
  do {
    if (locking)
      c = (char)fgetc(fptr);
    else
      c = (char)_fgetc_nolock(fptr);
    // append the new character unless it's a carriage return
    if (c != '\r' && c != '\n' && !feof(fptr))
      buffer.append(1, c);
  } while (!feof(fptr) && c != '\n');
  return _strdup(buffer.c_str());
}

bool writeValue(FILE* fptr, JSContext* cx, jsval value, bool isBinary, bool locking) {
  int len = 0, result;
  int32 ival = 0;
  jsdouble dval = 0;
  bool bval;

  // TODO(ejt): better method to detect int or double
  if (JS_IsNull(value) || JS_IsUndefined(value)) {
    if (locking)
      result = fwrite(&ival, sizeof(int), 1, fptr);
    else
      result = _fwrite_nolock(&ival, sizeof(int), 1, fptr);
    if (result == 1)
      return true;
  } else if (JS_IsString(value)) {
    size_t len2;
    const char* str = JS_ToCStringLen(cx, &len2, value);
    if (locking)
      result = fwrite(str, sizeof(char), len2, fptr);
    else
      result = _fwrite_nolock(str, sizeof(char), len2, fptr);
    JS_FreeCString(cx, str);
    return len2 == static_cast<size_t>(result);
  } else if (JS_IsNumber(value)) {
    if (isBinary) {
      if (!JS_ToInt32(cx, &ival, value)) {
        if (locking)
          result = fwrite(&ival, sizeof(int32), 1, fptr);
        else
          result = _fwrite_nolock(&dval, sizeof(int32), 1, fptr);
        return result == 1;
      } else if (!JS_ToFloat64(cx, &dval, value)) {
        if (locking)
          result = fwrite(&dval, sizeof(jsdouble), 1, fptr);
        else
          result = _fwrite_nolock(&dval, sizeof(jsdouble), 1, fptr);
        return result == 1;
      }
      return false;
    } else {
      if (!JS_ToInt32(cx, &ival, value)) {
        char* str = new char[16];
        _itoa_s(ival, str, 16, 10);
        len = strlen(str);
        if (locking)
          result = fwrite(str, sizeof(char), len, fptr);
        else
          result = _fwrite_nolock(str, sizeof(char), len, fptr);
        delete[] str;
        if (result == len)
          return true;
      } else if (!JS_ToFloat64(cx, &dval, value)) {
        // jsdouble will never be a 64-char string, but I'd rather be safe than sorry
        char* str = new char[64];
        sprintf_s(str, 64, "%.16f", dval);
        len = strlen(str);
        if (locking)
          result = fwrite(str, sizeof(char), len, fptr);
        else
          result = _fwrite_nolock(str, sizeof(char), len, fptr);
        delete[] str;
        if (result == len)
          return true;
      }
      return false;
    }
  } else if (JS_IsBool(value)) {
    if (!isBinary) {
      bval = !!JS_ToBool(cx, value);
      const char* str = bval ? "true" : "false";
      if (locking)
        result = fwrite(str, sizeof(char), strlen(str), fptr);
      else
        result = _fwrite_nolock(str, sizeof(char), strlen(str), fptr);
      return (int)strlen(str) == result;
    } else {
      bval = !!JS_ToBool(cx, value);
      if (locking)
        result = fwrite(&bval, sizeof(bool), 1, fptr);
      else
        result = _fwrite_nolock(&bval, sizeof(bool), 1, fptr);
      return result == 1;
    }
  }

  return false;
}

/** Safely open a file relative to the script dir.
 *
 * In theory this is the only function used to open files that is exposed to
 * javascript.
 *
 * \param filename Name of the file to open relative to the script folder
 *
 * \param mode Mode to open in. See fopen_s.
 *
 * \param cx JSContext that is running. Used to throw errors.
 *
 * \return The file pointer.
 */
FILE* fileOpenRelScript(const wchar_t* filename, const wchar_t* mode, JSContext* cx) {
  FILE* f;
  wchar_t fullPath[_MAX_PATH + _MAX_FNAME];

  // Get the relative path
  if (getPathRelScript(filename, _MAX_PATH + _MAX_FNAME, fullPath) == NULL) {
    JS_ReportError(cx, "Invalid file name");
    return NULL;
  }

  // Open the file
  if (_wfopen_s(&f, fullPath, mode) != 0 || f == NULL) {
    char message[128];
    _strerror_s(message, 128, NULL);
    JS_ReportError(cx, "Couldn't open file %ls: %s", filename, message);
    return NULL;
  }

  return f;
}

/** Get the full path relative to the script path. Does the validation check.
 *
 * \param filename Name of the file to open relative to the script folder.
 *
 * \param bufLen Length of the output buffer.
 *
 * \param fullPath Buffer where the full path will be stored. Empty string if
 *    location is invalid.
 *
 * \return fullPath on success or NULL on failure.
 */
wchar_t* getPathRelScript(const wchar_t* filename, int bufLen, wchar_t* fullPath) {
  wchar_t fullScriptPath[_MAX_PATH + _MAX_FNAME];
  wchar_t* relPath;
  int strLenScript;
  DWORD scrPathLen;

  strLenScript = wcslen(Vars.szScriptPath);

  // Make the filename relative to the script path
  relPath = (wchar_t*)_alloca((strLenScript + wcslen(filename) + 2) * 2);  // *2 for wide chars
  wcscpy_s(relPath, strLenScript + wcslen(filename) + 2, Vars.szScriptPath);
  relPath[strLenScript] = L'\\';
  wcscpy_s(relPath + strLenScript + 1, wcslen(filename) + 1, filename);

  // Transform to the full pathname
  GetFullPathNameW(relPath, bufLen, fullPath, NULL);

  // Get the full path of the script path, check it is the prefix of fullPath
  scrPathLen = GetFullPathNameW(Vars.szScriptPath, _MAX_PATH + _MAX_FNAME, fullScriptPath, NULL);

  // Check that fullScriptPath is the prefix of fullPath
  // As GetFullPathName seems to not add a trailing \, if there is not a
  // trailing \ in fullScriptPath check for it in fullPath
  if (wcsncmp(fullPath, fullScriptPath, scrPathLen) != 0 || (fullScriptPath[scrPathLen - 1] != '\\' && fullPath[scrPathLen] != '\\')) {
    fullPath[0] = '\0';
    return NULL;
  }

  return fullPath;
}

/** Check that the full path of the script path is the prefix of the fullpath
 * of name. Also checks that there is no ..\ or ../ sequences or ":?*<>| chars.
 *
 * \param name The file/path to validate.
 *
 * \return true if path is valid, false otherwise.
 */
bool isValidPath(const wchar_t* name) {
  wchar_t fullPath[_MAX_PATH + _MAX_FNAME];

  // Use getPathRelScript to validate based on full paths
  if (getPathRelScript(name, _MAX_PATH + _MAX_FNAME, fullPath) == NULL)
    return false;

  return (!wcsstr(name, L"..\\") && !wcsstr(name, L"../") && (wcscspn(name, L"\":?*<>|") == wcslen(name)));
}