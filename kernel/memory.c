#include "memory.h"
#include "bitmap.h"
#include "print.h"
#include "debug.h"
#include "string.h"

// #define PAGE_SIZE 4096
#define BMP_BASE_ADDR 0xc009a000
#define KERNEL_VIRTUAL_START 0xc0100000

struct pool kernel_pool;
struct pool user_pool;
struct virtual_pool kernel_virtual_pool;

//初始化kernel/用户 物理/虚拟内存池
void mem_pool_init(uint32_t mem_total){
    put_str("mem_pool_init start....\n");

    uint32_t mem_used = PAGE_SIZE * 256 + 0x100000;  //低端1M + 页目录和页表所占内存
    uint32_t mem_free = mem_total - mem_used; //可用内存大小
    uint32_t mem_free_pg = mem_free / PAGE_SIZE; //可用页数

    uint32_t kernel_free_pg = mem_free_pg / 2;  //内核空间可用内存页数
    uint32_t user_free_pg = mem_free_pg - kernel_free_pg; //用户空间可用内存页
    
    uint32_t kp_bmp_size = kernel_free_pg / 8; //kernel_pool bmp.size大小
    uint32_t up_bmp_size = user_free_pg / 8;  //
 
    uint32_t kp_addr = mem_used; //kernel pool起始地址
    uint32_t up_addr = kp_addr + kernel_free_pg * PAGE_SIZE; //user pool起始地址

    //初始化kernel_pool和user_pool
    kernel_pool.addr = kp_addr;
    user_pool.addr = up_addr;

    kernel_pool.size = kernel_free_pg * PAGE_SIZE;
    user_pool.size = user_free_pg * PAGE_SIZE;

    kernel_pool.bmp.size = kp_bmp_size;
    user_pool.bmp.size = up_bmp_size;

    kernel_pool.bmp.addr = (uint8_t*)BMP_BASE_ADDR;
    user_pool.bmp.addr = (uint8_t*)(BMP_BASE_ADDR + kp_bmp_size);

    //设置kernel_virtual_pool
    kernel_virtual_pool.bmp.size = kernel_pool.bmp.size;
    kernel_virtual_pool.bmp.addr = (uint8_t*)(user_pool.bmp.addr + user_pool.bmp.size);
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

#define GET_PDE(vaddr) ((vaddr & 0xffc00000) >> 22)
#define GET_PTE(vaddr) ((vaddr & 0x003ff000) >> 12)

static uint32_t* pde_vaddr_get(uint32_t vaddr){ 
    uint32_t* pde_vaddr = (uint32_t*)((0xfffff000) + (GET_PDE(vaddr) * 4));
    return pde_vaddr;
}

static uint32_t* pte_vaddr_get(uint32_t vaddr){
    uint32_t* pte_vaddr = (uint32_t*)(0xffc00000 + ((vaddr & 0xffc00000) >> 10) + GET_PTE(vaddr) * 4);
    return pte_vaddr;
}

//获得cnt个虚拟页
static void* virtual_page_get(enum PAGE_K_U page_flag, uint32_t cnt){
    uint32_t cur_cnt = 0;
    int vaddr = 0;
    if(page_flag == PAGE_K){ //内核空间页
        int bit_ind = bitmap_alloc(&kernel_virtual_pool.bmp, cnt);
        if(bit_ind != -1){
            while(cur_cnt < cnt){
                bitmap_set(&kernel_virtual_pool.bmp, bit_ind + cur_cnt++, 1);
            }
            vaddr = (kernel_virtual_pool.addr + bit_ind * PAGE_SIZE);
        }else{
            return NULL;
        }
    }else{
        //用户空间页
        return NULL;
    }
    return (void*)vaddr;
}

//获得1个物理页
static void* phy_page_get(struct pool* cur_pool){
    uint32_t bit_ind = bitmap_alloc(&cur_pool->bmp, 1);
    put_int(bit_ind);
    put_char('\n');
    if(bit_ind == -1){
        return NULL;
    }
    else{
        bitmap_set(&cur_pool->bmp, bit_ind, 1);
        // put_int(cur_pool->addr + bit_ind * PAGE_SIZE);
        return (uint32_t*)(cur_pool->addr + bit_ind * PAGE_SIZE);
    }
}

//注册物理页到页表pte
static void page_register(uint32_t* phyaddr, uint32_t* vaddr){
    uint32_t v_addr = (uint32_t)vaddr;
    uint32_t phy_addr = (uint32_t)phyaddr;
    uint32_t* pde = pde_vaddr_get(v_addr);
    uint32_t* pte = pte_vaddr_get(v_addr);

    if(*pde & 0x00000001){ //PDE是否存在
        if(!(*pte & 0x00000001)){ //PTE是否存在
            *pte = (phy_addr | PAGE_U4 | PAGE_W | PAGE_P);
        }else{
            // Assert("page using!");
            return NULL;
        }
    }else{ //PDE不存在，现创建
        uint32_t pde_phyaddr = (uint32_t)phy_page_get(&kernel_pool);
        *pde = (pde_phyaddr | PAGE_U4 | PAGE_W | PAGE_P); 
        memset((void*)((int)pte & 0xfffff000), 0, PAGE_SIZE);
        if(!(*pte & 0x00000001)){ //PTE是否存在
            *pte = (phy_addr | PAGE_U4 | PAGE_W | PAGE_P);
        }else{
            Assert("page using!");
            return NULL;
        }
    }
}

//分配cnt个页内存，返回起始页虚拟地址
void* malloc_page(enum PAGE_K_U page_flag, uint32_t cnt){
    ASSERT(cnt > 0 && cnt < 3840);
    
    void* vaddr = virtual_page_get(page_flag, cnt);
    if(vaddr == NULL) return NULL;

    uint32_t _vaddr = (uint32_t)vaddr;
    struct pool* cur_pool = (page_flag == PAGE_K? &kernel_pool : &user_pool);
    while(cnt-- > 0){
        void* phy_addr = phy_page_get(cur_pool);
        if(phy_addr == NULL) return NULL;
        // put_int(phy_addr);
        page_register(phy_addr, (void*)_vaddr);
        _vaddr += PAGE_SIZE;
    }
    return vaddr;
}

//申请cnt个内核空间页
void* kernel_page_get(uint32_t cnt){
    void* vaddr = malloc_page(PAGE_K, cnt);
    if(vaddr != NULL){
        memset(vaddr, 0, cnt * PAGE_SIZE);
    }
    return vaddr;
}

//初始化内存管理
void mem_init(){
    put_str("mem_init start....\n");

    uint32_t* mem_total_addr = (uint32_t*)0xb06;
    put_str("mem_total: ");
    put_int(*mem_total_addr); //总物理内存(字节)
    put_char('\n');
    mem_pool_init(*mem_total_addr);

    put_str("mem_init done....\n");
}