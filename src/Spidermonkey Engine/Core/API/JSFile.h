#pragma once

#include "JSBaseObject.h"

enum {
  FILE_READ = 0,
  FILE_WRITE = 1,
  FILE_APPEND = 2,
};

class FileWrap : public BaseObject {
 public:
  // TODO: get rid of this
  struct FileData {
    int mode;
    char* path;
    bool autoflush, locked;
    FILE* fptr;
#if DEBUG
    const char* lockLocation;
    int line;
#endif
  };

  static JSObject* Instantiate(JSContext* ctx, const FileData& data);
  static void Initialize(JSContext* ctx, JS::HandleObject target);

 private:
  FileWrap(JSContext* ctx, JS::HandleObject obj, const FileData& data);
  virtual ~FileWrap();

  static void finalize(JSFreeOp* fop, JSObject* obj);

  static bool New(JSContext* ctx, JS::CallArgs& args);

  // properties
  static bool GetReadable(JSContext* ctx, JS::CallArgs& args);
  static bool GetWriteable(JSContext* ctx, JS::CallArgs& args);
  static bool GetSeekable(JSContext* ctx, JS::CallArgs& args);
  static bool GetMode(JSContext* ctx, JS::CallArgs& args);
  static bool GetBinaryMode(JSContext* ctx, JS::CallArgs& args);
  static bool GetLength(JSContext* ctx, JS::CallArgs& args);
  static bool GetPath(JSContext* ctx, JS::CallArgs& args);
  static bool GetPosition(JSContext* ctx, JS::CallArgs& args);
  static bool GetEOF(JSContext* ctx, JS::CallArgs& args);
  static bool GetAccessed(JSContext* ctx, JS::CallArgs& args);
  static bool GetCreated(JSContext* ctx, JS::CallArgs& args);
  static bool GetModified(JSContext* ctx, JS::CallArgs& args);
  static bool GetAutoFlush(JSContext* ctx, JS::CallArgs& args);
  static bool SetAutoFlush(JSContext* ctx, JS::CallArgs& args);

  // functions
  static bool Close(JSContext* ctx, JS::CallArgs& args);
  static bool Reopen(JSContext* ctx, JS::CallArgs& args);
  static bool Read(JSContext* ctx, JS::CallArgs& args);
  static bool ReadLine(JSContext* ctx, JS::CallArgs& args);
  static bool ReadAllLines(JSContext* ctx, JS::CallArgs& args);
  static bool ReadAll(JSContext* ctx, JS::CallArgs& args);
  static bool Write(JSContext* ctx, JS::CallArgs& args);
  static bool Seek(JSContext* ctx, JS::CallArgs& args);
  static bool Flush(JSContext* ctx, JS::CallArgs& args);
  static bool Reset(JSContext* ctx, JS::CallArgs& args);
  static bool End(JSContext* ctx, JS::CallArgs& args);

  // static functions
  static bool Open(JSContext* ctx, JS::CallArgs& args);

  static inline JSClassOps m_ops = {
      .addProperty = nullptr,
      .delProperty = nullptr,
      .enumerate = nullptr,
      .newEnumerate = nullptr,
      .resolve = nullptr,
      .mayResolve = nullptr,
      .finalize = finalize,
      .call = nullptr,
      .hasInstance = nullptr,
      .construct = nullptr,
      .trace = nullptr,
  };
  static inline JSClass m_class = {
      "File",
      JSCLASS_HAS_RESERVED_SLOTS(kInternalFieldCount),
      &m_ops,
  };
  static inline JSPropertySpec m_props[] = {
      JS_PSG("readable", trampoline<GetReadable>, JSPROP_ENUMERATE),
      JS_PSG("writeable", trampoline<GetWriteable>, JSPROP_ENUMERATE),
      JS_PSG("seekable", trampoline<GetSeekable>, JSPROP_ENUMERATE),
      JS_PSG("mode", trampoline<GetMode>, JSPROP_ENUMERATE),
      JS_PSG("binaryMode", trampoline<GetBinaryMode>, JSPROP_ENUMERATE),
      JS_PSG("length", trampoline<GetLength>, JSPROP_ENUMERATE),
      JS_PSG("path", trampoline<GetPath>, JSPROP_ENUMERATE),
      JS_PSG("position", trampoline<GetPosition>, JSPROP_ENUMERATE),
      JS_PSG("eof", trampoline<GetEOF>, JSPROP_ENUMERATE),
      JS_PSG("accessed", trampoline<GetAccessed>, JSPROP_ENUMERATE),
      JS_PSG("created", trampoline<GetCreated>, JSPROP_ENUMERATE),
      JS_PSG("modified", trampoline<GetModified>, JSPROP_ENUMERATE),
      JS_PSGS("autoflush", trampoline<GetAutoFlush>, trampoline<SetAutoFlush>, JSPROP_ENUMERATE),
      JS_PS_END,
  };

  static inline JSFunctionSpec m_methods[] = {
      JS_FN("close", trampoline<Close>, 0, JSPROP_ENUMERATE),
      JS_FN("reopen", trampoline<Reopen>, 0, JSPROP_ENUMERATE),
      JS_FN("read", trampoline<Read>, 1, JSPROP_ENUMERATE),
      JS_FN("readLine", trampoline<ReadLine>, 0, JSPROP_ENUMERATE),
      JS_FN("readAllLines", trampoline<ReadAllLines>, 0, JSPROP_ENUMERATE),
      JS_FN("readAll", trampoline<ReadAll>, 0, JSPROP_ENUMERATE),
      JS_FN("write", trampoline<Write>, 1, JSPROP_ENUMERATE),
      JS_FN("seek", trampoline<Seek>, 1, JSPROP_ENUMERATE),
      JS_FN("flush", trampoline<Flush>, 0, JSPROP_ENUMERATE),
      JS_FN("reset", trampoline<Reset>, 0, JSPROP_ENUMERATE),
      JS_FN("end", trampoline<End>, 0, JSPROP_ENUMERATE),
      JS_FS_END,
  };

  static inline JSFunctionSpec m_smethods[] = {
      JS_FN("open", trampoline<Open>, 2, JSPROP_ENUMERATE),
      JS_FS_END,
  };

  FileData fdata;
};
