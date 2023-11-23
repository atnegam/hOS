#ifndef STRING__H
#define STRING__H
#include "stdint.h"

void memset(void* _dst, uint8_t value, uint32_t cnt);
void memcpy(const void* _src, void* _dst, uint32_t cnt);
int memcmp(const void* _src1, const void* _src2, uint32_t cnt);

char* strcpy(const char* src, char* dst);
int strlen(const char* str);
int strcmp(const char* str1, const char* str2);
char* strchr(const char* str, char c);
char* strlchr(const char* str, char c);
char* strcat(char* src, char* dst);
int strchrcnt(const char* src, char c);

#endif