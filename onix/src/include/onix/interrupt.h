#ifndef ONIX_INTERRUPT_H
#define ONIX_INTERRUPT_H

#include<onix/types.h>

#define IDT_SIZE 256

typedef struct gate_t {
    u16 offset0;    //段内偏移0~15位
    u16 selector;   //代码段选择子
    u8 reserved;    //保留不用
    u8 type : 4;    //任务门/中断门/陷阱门
    u8 segment : 1; //segment = 0 表示系统段
    u8 DPL : 2;     //使用int命令访问的最低权限
    u8 present : 1; //是否有效
    u16 offset1;    //段内偏移16~31位
}_packed gate_t;

void interrupt_init();

typedef void *handler_t; // 中断处理函数
#endif