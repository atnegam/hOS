#include "string.h"
#include "stdint.h"
#include "debug.h"
#include "stdvar.h"
//逐字节操作

void memset(void* _dst, uint8_t value, uint32_t cnt){
    ASSERT(_dst != NULL);
    uint8_t* dst = (uint8_t*)_dst; 
    while(cnt--) *dst++ = value;
}

void memcpy(const void* _src, void* _dst, uint32_t cnt){
    ASSERT(_src != NULL && _dst != NULL);
    const uint8_t* src = (uint8_t*)_src;
    uint8_t* dst = (uint8_t*)_dst;
    while(cnt--) *dst++ = *src++;
}

int memcmp(const void* _src1, const void* _src2, uint32_t cnt){
    ASSERT(_src1 != NULL && _src2 != NULL);
    uint8_t* src1 = (uint8_t*)_src1;
    uint8_t* src2 = (uint8_t*)_src2;
    while(cnt--){
        if(*src1 > *src2) return 1;
        if(*src1 < *src2) return -1;
        src1++;
        src2++;
    }
    return 0;
}

char* strcpy(const char* src, char* _dst){
    ASSERT(src != NULL);
    char* dst = _dst;
    while(*src != '\0'){
        *dst++ = *src++;
    }
    *dst = '\0';
    return dst;
}

int strlen(const char* str){
    ASSERT(str != NULL);
    int cnt = 0;
    while(*str++ != '\0') cnt++;
    return cnt;
}

int strcmp(const char* str1, const char* str2){
    ASSERT(str1 != NULL && str2 != NULL);
    while(*str1 != '\0' && *str2 != '\0'){
        if(*str1 > *str2) return 1;
        if(*str1 < *str2) return -1;
        str1++;
        str2++;
    }
    if(*str1 != '\0') return 1;
    else if(*str2 != '\0') return -1;
    else return 0;
}

char* strchr(const char* str, char c){
    ASSERT(str != NULL);
    while(*str != 0){
        if(*str == c) return (char*)str;
        str++;
    }
    return NULL; 
}

char* strlchr(const char* str, char c){
    ASSERT(str != NULL);
    const char* last_char = NULL;
    while(*str != '\0'){
        if(*str == c) last_char = str;
        str++;
    }
    return (char*)last_char;
}

char* strcat(char* src, char* _dst){
    ASSERT(src != NULL);
    char* dst = _dst;
    while(*_dst++ != '\0');
    _dst--;
    while(*src != '\0')
        *_dst++ = *src;
    *_dst = '\0';
    return dst;
}

int strchrcnt(const char* str, char c){
    ASSERT(str != NULL);
    int cnt = 0;
    while(*str != '\0'){
        if(*str == c) cnt++;
    }
    return cnt;
}