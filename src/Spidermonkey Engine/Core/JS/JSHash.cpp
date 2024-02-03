#include "JSHash.h"
#include "Hash.h"
#include "File.h"
#include "Engine.h"
#include "Helpers.h"

JSValue js_hash(JSContext* ctx, int argc, JSValue* argv, char* (*hashfn)(const char*)) {
  JSValue rval = JS_NULL;
  if (argc != 1)
    THROW_ERROR(ctx, "Invalid arguments");

  const char* input = JS_ToCString(ctx, argv[0]);
  char* result = hashfn(input);
  if (result && result[0])
    rval = JS_NewString(ctx, result);
  delete[] result;
  JS_FreeCString(ctx, input);
  return rval;
}

JSValue js_hash_file(JSContext* ctx, int argc, JSValue* argv, char*(hashfn)(const char*)) {
  JSValue rval = JS_NULL;

  if (argc != 1)
    THROW_ERROR(ctx, "Invalid arguments");

  const char* szFile = JS_ToCString(ctx, argv[0]);
  if (!szFile) {
    return JS_EXCEPTION;
  }
  if (!(szFile && szFile[0] && isValidPath(szFile))) {
    JS_FreeCString(ctx, szFile);
    THROW_ERROR(ctx, "Invalid file path!");
  }

  char path[_MAX_FNAME + _MAX_PATH];
  sprintf_s(path, _countof(path), "%s\\%s", Vars.szScriptPath, szFile);

  char* result = hashfn(path);
  if (result && result[0])
    rval = JS_NewString(ctx, result);
  delete[] result;
  return rval;
}

JSAPI_FUNC(my_md5) {
  return js_hash(ctx, argc, argv, md5);
}

JSAPI_FUNC(my_sha1) {
  return js_hash(ctx, argc, argv, sha1);
}

JSAPI_FUNC(my_sha256) {
  return js_hash(ctx, argc, argv, sha256);
}

JSAPI_FUNC(my_sha384) {
  return js_hash(ctx, argc, argv, sha384);
}

JSAPI_FUNC(my_sha512) {
  return js_hash(ctx, argc, argv, sha512);
}

JSAPI_FUNC(my_md5_file) {
  return js_hash_file(ctx, argc, argv, md5_file);
}

JSAPI_FUNC(my_sha1_file) {
  return js_hash_file(ctx, argc, argv, sha1_file);
}

JSAPI_FUNC(my_sha256_file) {
  return js_hash_file(ctx, argc, argv, sha256_file);
}

JSAPI_FUNC(my_sha384_file) {
  return js_hash_file(ctx, argc, argv, sha384_file);
}

JSAPI_FUNC(my_sha512_file) {
  return js_hash_file(ctx, argc, argv, sha512_file);
}
