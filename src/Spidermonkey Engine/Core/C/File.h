#ifndef __FILE_H__
#define __FILE_H__

#include "js32.h"

char* readLine(FILE* fptr, bool locking);
bool writeValue(FILE* fptr, JSContext* cx, JS::HandleValue value, bool isBinary, bool locking);
FILE* fileOpenRelScript(const char* filename, const char* mode, JSContext* cx);
char* getPathRelScript(const char* filename, int bufLen, char* fullPath);
bool isValidPath(const char* name);

#endif
