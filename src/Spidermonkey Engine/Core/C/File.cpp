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
#include "StringWrap.h"

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

bool writeValue(FILE* fptr, JSContext* cx, JS::HandleValue value, bool isBinary, bool locking) {
  int len = 0, result;
  int32_t ival = 0;
  double dval = 0;
  bool bval;

  // TODO(ejt): better method to detect int or double
  if (value.isNullOrUndefined()) {
    if (locking)
      result = fwrite(&ival, sizeof(int), 1, fptr);
    else
      result = _fwrite_nolock(&ival, sizeof(int), 1, fptr);
    if (result == 1)
      return true;
  } else if (value.isString()) {
    StringWrap str(cx, value);
    if (locking)
      result = fwrite(str, sizeof(char), str.length(), fptr);
    else
      result = _fwrite_nolock(str, sizeof(char), str.length(), fptr);
    return str.length() == static_cast<size_t>(result);
  } else if (value.isNumber()) {
    if (isBinary) {
      if (value.isInt32()) {
        ival = value.toInt32();
        if (locking)
          result = fwrite(&ival, sizeof(int32_t), 1, fptr);
        else
          result = _fwrite_nolock(&dval, sizeof(int32_t), 1, fptr);
        return result == 1;
      } else if (value.isNumber()) {
        dval = value.toNumber();
        if (locking)
          result = fwrite(&dval, sizeof(double), 1, fptr);
        else
          result = _fwrite_nolock(&dval, sizeof(double), 1, fptr);
        return result == 1;
      }
      return false;
    } else {
      if (value.isInt32()) {
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
      } else if (value.isNumber()) {
        dval = value.toNumber();
        // double will never be a 64-char string, but I'd rather be safe than sorry
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
  } else if (value.isBoolean()) {
    if (!isBinary) {
      bval = value.toBoolean();
      const char* str = bval ? "true" : "false";
      if (locking)
        result = fwrite(str, sizeof(char), strlen(str), fptr);
      else
        result = _fwrite_nolock(str, sizeof(char), strlen(str), fptr);
      return (int)strlen(str) == result;
    } else {
      bval = value.toBoolean();
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
FILE* fileOpenRelScript(const char* filename, const char* mode, JSContext* cx) {
  FILE* f;
  char fullPath[_MAX_PATH + _MAX_FNAME];

  // Get the relative path
  if (getPathRelScript(filename, _MAX_PATH + _MAX_FNAME, fullPath) == NULL) {
    JS_ReportErrorASCII(cx, "Invalid file name");
    return NULL;
  }

  // Open the file
  if (fopen_s(&f, fullPath, mode) != 0 || f == NULL) {
    char message[128];
    _strerror_s(message, 128, NULL);
    JS_ReportErrorASCII(cx, "Couldn't open file %s: %s", filename, message);
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
char* getPathRelScript(const char* filename, int bufLen, char* fullPath) {
  char fullScriptPath[_MAX_PATH + _MAX_FNAME];
  char* relPath = NULL;
  int strLenScript;
  DWORD scrPathLen;

  strLenScript = strlen(Vars.szScriptPath);

  // Make the filename relative to the script path
  relPath = (char*)_malloca(strLenScript + strlen(filename) + 2);
  if (relPath == NULL) {
    return NULL;
  }
  sprintf_s(relPath, strLenScript + strlen(filename) + 2, "%s\\%s", Vars.szScriptPath, filename);

  // Transform to the full pathname
  GetFullPathNameA(relPath, bufLen, fullPath, NULL);

  // Get the full path of the script path, check it is the prefix of fullPath
  scrPathLen = GetFullPathNameA(Vars.szScriptPath, _MAX_PATH + _MAX_FNAME, fullScriptPath, NULL);

  // Check that fullScriptPath is the prefix of fullPath
  // As GetFullPathName seems to not add a trailing \, if there is not a
  // trailing \ in fullScriptPath check for it in fullPath
  if (strncmp(fullPath, fullScriptPath, scrPathLen) != 0 || (fullScriptPath[scrPathLen - 1] != '\\' && fullPath[scrPathLen] != '\\')) {
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
bool isValidPath(const char* name) {
  char fullPath[_MAX_PATH + _MAX_FNAME];

  // Use getPathRelScript to validate based on full paths
  if (getPathRelScript(name, _MAX_PATH + _MAX_FNAME, fullPath) == NULL)
    return false;

  return (!strstr(name, "..\\") && !strstr(name, "../") && (strcspn(name, "\":?*<>|") == strlen(name)));
}
