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

enum PAGE_K_U{
    PAGE_K=1, //内核空间页
    PAGE_U=2  //用户空间页
};

#define PAGE_P 1 //PDE/PTE存在
#define PAGE_NP 0 //PDE/PTE不存在
#define PAGE_R 0 //此页可读和执行
#define PAGE_W 2 //此页可写，写和执行

#define PAGE_K0 0
#define PAGE_U4 4

#define PAGE_SIZE 4096

void* malloc_page(enum PAGE_K_U flag, uint32_t cnt);
void* kernel_page_get(uint32_t cnt);

void mem_init();

#endif
