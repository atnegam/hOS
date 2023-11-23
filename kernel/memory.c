#include "memory.h"
#include "bitmap.h"
#include "print.h"

#define PG_SIZE 4096
#define BMP_BASE_ADDR 0xc009a000
#define KERNEL_VIRTUAL_START 0xc0100000

struct pool kernel_pool;
struct pool user_pool;
struct virtual_pool kernel_virtual_pool;

void mem_pool_init(uint32_t mem_total){
    put_str("mem_pool_init start....\n");

    uint32_t mem_used = PG_SIZE * 256 + 0x100000;  //低端1M + 页目录和页表所占内存
    uint32_t mem_free = mem_total - mem_used; //可用内存大小
    uint16_t mem_free_pg = mem_free / PG_SIZE; //可用页数

    uint16_t kernel_free_pg = mem_free_pg / 2;  //内核空间可用内存页数
    uint16_t user_free_pg = mem_free_pg - kernel_free_pg; //用户空间可用内存页
    
    uint32_t kp_bmp_size = kernel_free_pg / 8; //kernel_pool bmp.size大小
    uint32_t up_bmp_size = user_free_pg / 8;  //

    uint32_t kp_addr = mem_used; //kernel pool起始地址
    uint32_t up_addr = kp_addr + kernel_free_pg * PG_SIZE; //user pool起始地址

    //初始化kernel_pool和user_pool
    kernel_pool.addr = kp_addr;
    user_pool.addr = up_addr;

    kernel_pool.size = kernel_free_pg * PG_SIZE;
    user_pool.size = user_free_pg * PG_SIZE;

    kernel_pool.bmp.size = kp_bmp_size;
    user_pool.bmp.size = up_bmp_size;

    kernel_pool.bmp.addr = (void*)BMP_BASE_ADDR;
    user_pool.bmp.addr = (void*)(BMP_BASE_ADDR + kp_bmp_size);

    //设置kernel_virtual_pool
    kernel_virtual_pool.bmp.size = kernel_pool.bmp.size;
    kernel_virtual_pool.bmp.addr = (void*)(user_pool.bmp.addr + user_pool.bmp.size);
    kernel_virtual_pool.addr = KERNEL_VIRTUAL_START;

    //初始化bitmap
    bitmap_init(&kernel_pool.bmp);
    bitmap_init(&user_pool.bmp);
    bitmap_init(&kernel_virtual_pool.bmp);

    //打印内存初始化信息
    put_str("kernel pool bitmap addr: ");
    put_int((int)kernel_pool.bmp.addr);
    put_char('\n');
    put_str("kernel pool bitmap size: ");
    put_int(kernel_pool.bmp.size);
    put_char('\n');
    put_str("kernel pool addr: ");
    put_int((int)kernel_pool.addr);
    put_char('\n');
    put_str("kernel_pool size: ");
    put_int(kernel_pool.size);
    put_char('\n');
    put_str("user pool bitmap addr: ");
    put_int((int)user_pool.bmp.addr);
    put_char('\n');
    put_str("uer pool bitmap size: ");
    put_int(user_pool.bmp.size);
    put_char('\n');
    put_str("user pool addr: ");
    put_int((int)user_pool.addr);
    put_char('\n');
    put_str("user size: ");
    put_int(user_pool.size);
    put_char('\n');

    put_str("mem_pool_init done\n");

}


void mem_init(){
    put_str("mem_init start....\n");

    uint32_t* mem_total_addr = (uint32_t*)0xb06;
    put_str("mem_total: ");
    put_int(*mem_total_addr); //总物理内存(字节)
    put_char('\n');
    mem_pool_init(*mem_total_addr);

    put_str("mem_init done....\n");
}