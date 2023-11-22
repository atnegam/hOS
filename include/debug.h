#ifndef DEBUG__H
#define DEBUG__H

void assert(char* filename, int line, char* func, const char* condition);

#define Assert(...) assert(__FILE__, __LINE__, __func__, __VA_ARGS__); 

#ifdef NDEBUG   //非调试版本忽略ASSERT
    #define ASSERT(CONDITION) (void)(0)
#else
    #define ASSERT(CONDITION) if(CONDITION){ }else{ Assert(#CONDITION);}
#endif

#endif