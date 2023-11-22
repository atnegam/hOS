#include "bitmap.h"
#include "stdint.h"
#include "debug.h"
#include "string.h"

//初始化bitmap
void bitmap_init(struct bitmap* bmp){
    memset(bmp->addr, 0, bmp->size);
}

//检查某位是否占用
bool bitmap_check(struct bitmap* bmp, uint32_t bit_ind){
    uint32_t byte_ind = bit_ind / 8;
    //超出情况？
    uint32_t offset = bit_ind % 8;
    return bmp->addr[byte_ind + 1]&(1 << (8 - offset));
}

//获取连续cnt个可用位，返回其中第一个位
uint32_t bitmap_alloc(struct bitmap* bmp, uint32_t cnt){
    uint8_t* addr = bmp->addr;
    uint32_t byte_ind = 0;
    while(byte_ind < bmp->size && *addr == 0xff){
        addr++;
        byte_ind++;
    } 
    if(byte_ind >= bmp->size) return -1; //超出
    int bit_ind = byte_ind * 8;
    int tmp = bit_ind;
    while(bitmap_check(bmp, bit_ind++));
    
    uint32_t available = bmp->size * 8 - bit_ind + 1;
    if(cnt > available) return -1; //可用位不够

    int cur_cnt = 0;
    
    bit_ind = tmp;
    uint32_t start_bit = bit_ind - 1;
    while(available--){
        if(cur_cnt == cnt) return start_bit + 1; //返回连续可用cnt个位的第一位的bit_ind
        if(!bitmap_check(bmp, ++bit_ind)){
            cur_cnt++;
        }else{
            cur_cnt = 0;
            start_bit = bit_ind;
        }
    }
    return -1;
}

//设置某位为value
void bitmap_set(struct bitmap* bmp, uint32_t bit_ind, uint8_t value){
    uint32_t byte_ind = bit_ind / 8;
    uint32_t offset = bit_ind % 8;
    if(value == 1)
        bmp->addr[byte_ind + 1] |= (1 << (8 - offset));
    else 
        bmp->addr[byte_ind + 1] &= ~(1 << (8 - offset));
}
