#pragma once
#pragma comment(lib, "Crypt32")

#include <windows.h>
#include <wincrypt.h>

char* HashString(char* string, ALG_ID algo);
char* HashFile(wchar_t* file, ALG_ID algo);
char* md5(const char* string);
char* sha1(const char* string);
char* sha256(const char* string);
char* sha384(const char* string);
char* sha512(const char* string);
char* md5_file(wchar_t* path);
char* sha1_file(wchar_t* path);
char* sha256_file(wchar_t* path);
char* sha384_file(wchar_t* path);
char* sha512_file(wchar_t* path);
