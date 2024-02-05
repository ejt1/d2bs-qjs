#pragma once
#pragma comment(lib, "Crypt32")

#include <windows.h>
#include <wincrypt.h>

char* HashString(char* string, ALG_ID algo);
char* HashFile(const char* file, ALG_ID algo);
char* md5(const char* string);
char* sha1(const char* string);
char* sha256(const char* string);
char* sha384(const char* string);
char* sha512(const char* string);
char* md5_file(const char* path);
char* sha1_file(const char* path);
char* sha256_file(const char* path);
char* sha384_file(const char* path);
char* sha512_file(const char* path);
