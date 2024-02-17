//#pragma once
//
//#include "js32.h"
//
//enum {
//  FILE_READ = 0,
//  FILE_WRITE = 1,
//  FILE_APPEND = 2,
//};
//
//class FileWrap {
// public:
//   // TODO: get rid of this
//  struct FileData {
//    int mode;
//    char* path;
//    bool autoflush, locked;
//    FILE* fptr;
//#if DEBUG
//    char* lockLocation;
//    int line;
//#endif
//  };
//
//  static JSValue Instantiate(JSContext* ctx, JSValue new_target, const FileData& data);
//  static void Initialize(JSContext* ctx, JSValue target);
//
// private:
//  FileWrap(JSContext* ctx, const FileData& data);
//  virtual ~FileWrap();
//
//  // properties
//  static JSValue GetReadable(JSContext* ctx, JSValue this_val);
//  static JSValue GetWriteable(JSContext* ctx, JSValue this_val);
//  static JSValue GetSeekable(JSContext* ctx, JSValue this_val);
//  static JSValue GetMode(JSContext* ctx, JSValue this_val);
//  static JSValue GetBinaryMode(JSContext* ctx, JSValue this_val);
//  static JSValue GetLength(JSContext* ctx, JSValue this_val);
//  static JSValue GetPath(JSContext* ctx, JSValue this_val);
//  static JSValue GetPosition(JSContext* ctx, JSValue this_val);
//  static JSValue GetEOF(JSContext* ctx, JSValue this_val);
//  static JSValue GetAccessed(JSContext* ctx, JSValue this_val);
//  static JSValue GetCreated(JSContext* ctx, JSValue this_val);
//  static JSValue GetModified(JSContext* ctx, JSValue this_val);
//  static JSValue GetAutoFlush(JSContext* ctx, JSValue this_val);
//  static JSValue SetAutoFlush(JSContext* ctx, JSValue this_val, JSValue val);
//
//  // functions
//  static JSValue Close(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
//  static JSValue Reopen(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
//  static JSValue Read(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
//  static JSValue ReadLine(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
//  static JSValue ReadAllLines(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
//  static JSValue ReadAll(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
//  static JSValue Write(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
//  static JSValue Seek(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
//  static JSValue Flush(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
//  static JSValue Reset(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
//  static JSValue End(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
//
//  // static functions
//  static JSValue Open(JSContext* ctx, JSValue this_val, int argc, JSValue* argv);
//
//  static inline JSClassID m_class_id = 0;
//  static inline JSCFunctionListEntry m_proto_funcs[] = {
//      JS_CGETSET_DEF("readable", GetReadable, nullptr),
//      JS_CGETSET_DEF("writeable", GetWriteable, nullptr),
//      JS_CGETSET_DEF("seekable", GetSeekable, nullptr),
//      JS_CGETSET_DEF("mode", GetMode, nullptr),
//      JS_CGETSET_DEF("binaryMode", GetBinaryMode, nullptr),
//      JS_CGETSET_DEF("length", GetLength, nullptr),
//      JS_CGETSET_DEF("path", GetPath, nullptr),
//      JS_CGETSET_DEF("position", GetPosition, nullptr),
//      JS_CGETSET_DEF("eof", GetEOF, nullptr),
//      JS_CGETSET_DEF("accessed", GetAccessed, nullptr),
//      JS_CGETSET_DEF("created", GetCreated, nullptr),
//      JS_CGETSET_DEF("modified", GetModified, nullptr),
//      JS_CGETSET_DEF("autoflush", GetAutoFlush, SetAutoFlush),
//
//      JS_FN("close", Close, 0, FUNCTION_FLAGS),
//      JS_FN("reopen", Reopen, 0, FUNCTION_FLAGS),
//      JS_FN("read", Read, 1, FUNCTION_FLAGS),
//      JS_FN("readLine", ReadLine, 0, FUNCTION_FLAGS),
//      JS_FN("readAllLines", ReadAllLines, 0, FUNCTION_FLAGS),
//      JS_FN("readAll", ReadAll, 0, FUNCTION_FLAGS),
//      JS_FN("write", Write, 1, FUNCTION_FLAGS),
//      JS_FN("seek", Seek, 1, FUNCTION_FLAGS),
//      JS_FN("flush", Flush, 0, FUNCTION_FLAGS),
//      JS_FN("reset", Reset, 0, FUNCTION_FLAGS),
//      JS_FN("end", End, 0, FUNCTION_FLAGS),
//  };
//
//  static inline JSCFunctionListEntry m_static_funcs[] = {
//      JS_FN("open", Open, 2, FUNCTION_FLAGS),
//  };
//
//  FileData fdata;
//};
