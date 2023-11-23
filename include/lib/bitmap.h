#ifndef BITMAP__H
#define BITMAP__H
#include "stdint.h"
#include "stdvar.h"

struct bitmap{
    uint32_t size;
    uint8_t* addr;
};

void bitmap_init(struct bitmap* bmp);
bool bitmap_check(struct bitmap* bmp, uint32_t bit_ind);
uint32_t bitmap_alloc(struct bitmap* bmp, uint32_t cnt);
void bitmap_set(struct bitmap* bmp, uint32_t bit_ind, uint8_t value);


#endif