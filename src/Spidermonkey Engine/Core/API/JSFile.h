#pragma once

#include "js32.h"

//////////////////////////////////////////////////////////////////
// file stuff
//////////////////////////////////////////////////////////////////

CLASS_CTOR(file);
CLASS_FINALIZER(file);

JSAPI_PROP(file_getProperty);
JSAPI_STRICT_PROP(file_setProperty);

JSAPI_FUNC(file_open);
JSAPI_FUNC(file_close);
JSAPI_FUNC(file_reopen);
JSAPI_FUNC(file_read);
JSAPI_FUNC(file_readLine);
JSAPI_FUNC(file_readAllLines);
JSAPI_FUNC(file_readAll);
JSAPI_FUNC(file_write);
JSAPI_FUNC(file_seek);
JSAPI_FUNC(file_flush);
JSAPI_FUNC(file_reset);
JSAPI_FUNC(file_end);

/**
File object:

File File.open(string path, mode [, bool binaryMode [, bool autoflush [, bool lockFile]]]) - static - open the specified file, return a File object, mode = one
of the constants of the File object listed below, binaryMode = default setting of file.binaryMode, autoflush = default setting of file.autoflush, lockFile = if
true lock the file on open and unlock it on close, so other threads/programs can't open it

File file.close() - object - close the current file
File file.reopen() - object - reopen the current file
string/int file.read(int num) - object - read num amount of characters or bytes from the file, if in non-binary mode this will be null terminated
string file.readLine() - object - read a single line from the file, assuming the line ends with \n or \r\n, throws an exception if the file is in binary mode
array file.readAllLines() - object - read the full contents of the file and split it up into an array of lines, and return the array of lines, throws an
exception if the file is in binary mode string/array file.readAll() - object - read the full contents of the file and return it as one big string or if in
binary mode, array File file.write(object contents, ...) - object - write the specified byte/strings/objects/array-of-bytes to a file, throws an exception if
not all of the parameters could be written to disk File file.seek(int bytes [, bool isLines [, bool fromStart]]) - object - seek the specified number of bytes,
or optionally lines, in the file, optionally from the start of the file, stops at the end of the file File file.flush() - object - flushes the remaining buffer
to disk File file.reset() - object - seek to the beginning of the file File file.end() - object - seek to the end of the file
**/

static JSCFunctionListEntry file_methods[] = {
    JS_FN("close", file_close, 0, FUNCTION_FLAGS),
    JS_FN("reopen", file_reopen, 0, FUNCTION_FLAGS),
    JS_FN("read", file_read, 1, FUNCTION_FLAGS),
    JS_FN("readLine", file_readLine, 0, FUNCTION_FLAGS),
    JS_FN("readAllLines", file_readAllLines, 0, FUNCTION_FLAGS),
    JS_FN("readAll", file_readAll, 0, FUNCTION_FLAGS),
    JS_FN("write", file_write, 1, FUNCTION_FLAGS),
    JS_FN("seek", file_seek, 1, FUNCTION_FLAGS),
    JS_FN("flush", file_flush, 0, FUNCTION_FLAGS),
    JS_FN("reset", file_reset, 0, FUNCTION_FLAGS),
    JS_FN("end", file_end, 0, FUNCTION_FLAGS),
};

static JSCFunctionListEntry file_s_methods[] = {
    JS_FN("open", file_open, 2, FUNCTION_FLAGS),
};

// ensure that read/write/append get the correct values
enum {
  FILE_READ = 0,
  FILE_WRITE = 1,
  FILE_APPEND = 2,
  FILE_READABLE,
  FILE_WRITEABLE,
  FILE_SEEKABLE,
  FILE_MODE,
  FILE_LENGTH,
  FILE_PATH,
  FILE_POSITION,
  FILE_EOF,
  FILE_AUTOFLUSH,
  FILE_BINARYMODE,
  FILE_ACCESSED,
  FILE_CREATED,
  FILE_MODIFIED,
};

/**
bool file.readable - object - can read from the file
bool file.writable - object - can write from the file
bool file.seekable - object - can seek within the file
int file.mode - object - the mode the file was opened in, one of the FILE_MODE constants of the file object
bool file.binaryMode - object - determines if the file is in binary mode for read operations (write operations happen based on the parameter specified)
bool file.autoflush - object - automatically flush the file buffer after each write, defaults to off
int file.length - object - the length of the file in bytes
string file.path - object - the name and path (relative to the scripts/ folder) of the file
int file.position - object - the current position in the file
bool file.eof - object - determines if the file is at end-of-file or not
**/

static JSCFunctionListEntry file_props[] = {
    JS_CGETSET_MAGIC_DEF("readable", file_getProperty, nullptr, FILE_READABLE),
    JS_CGETSET_MAGIC_DEF("writeable", file_getProperty, nullptr, FILE_WRITEABLE),
    JS_CGETSET_MAGIC_DEF("seekable", file_getProperty, nullptr, FILE_SEEKABLE),
    JS_CGETSET_MAGIC_DEF("mode", file_getProperty, nullptr, FILE_MODE),
    JS_CGETSET_MAGIC_DEF("binaryMode", file_getProperty, nullptr, FILE_BINARYMODE),
    JS_CGETSET_MAGIC_DEF("length", file_getProperty, nullptr, FILE_LENGTH),
    JS_CGETSET_MAGIC_DEF("path", file_getProperty, nullptr, FILE_PATH),
    JS_CGETSET_MAGIC_DEF("position", file_getProperty, nullptr, FILE_POSITION),
    JS_CGETSET_MAGIC_DEF("eof", file_getProperty, nullptr, FILE_EOF),
    JS_CGETSET_MAGIC_DEF("accessed", file_getProperty, nullptr, FILE_ACCESSED),
    JS_CGETSET_MAGIC_DEF("created", file_getProperty, nullptr, FILE_CREATED),
    JS_CGETSET_MAGIC_DEF("modified", file_getProperty, nullptr, FILE_MODIFIED),
    JS_CGETSET_MAGIC_DEF("autoflush", file_getProperty, file_setProperty, FILE_AUTOFLUSH),
};