#ifndef MEMORY__H
#define MEMORY__H
#include "bitmap.h"

struct virtual_pool{
    struct bitmap bmp;
    uint32_t addr;
}; //虚拟内存池

struct pool{
    struct bitmap bmp;
    uint32_t addr;
    uint32_t size;
}; //物理内存池

void mem_init();

#endif
