#ifndef IO_H
#define IO_H

#include "stdint.h"
//向prot写一个字节
static void outb(uint16_t port, uint8_t data){
    asm volatile ("outb %b0, %w1" : : "a" (data), "Nd"(port));
}

//将addr指向的cnt个字节写入port
static  void outbsw(uint16_t port, void* addr, uint32_t cnt){
    asm volatile ("cld; rep outsw" : "+S"(addr), "+c"(cnt) : "d"(port));
}

//从prot读出一个字节
static uint8_t inb(uint16_t port) {
    uint8_t data;
    asm volatile ("inb %w1, %b0" : "=a" (data) : "Nd" (port)); return data;
}

//将prot读出的数据写到addr处
static void insw(uint16_t port, void* addr, uint32_t cnt){
    asm volatile ("cld; rep insw" : "+D" (addr), "+c" (cnt) : "d" (port) : "memory");
}

#endif