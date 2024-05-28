#include <onix/types.h>
#include <onix/debug.h>
#include <onix/interrupt.h>
#include <onix/io.h>
#include <onix/time.h>
#include <onix/assert.h>
#include <onix/stdlib.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define CMOS_ADDR 0x70 // CMOS 地址寄存器
#define CMOS_DATA 0x71 // CMOS 数据寄存器

#define CMOS_SECOND 0x01
#define CMOS_MINUTE 0x03
#define CMOS_HOUR 0x05

#define CMOS_A 0x0a
#define CMOS_B 0x0b
#define CMOS_C 0x0c
#define CMOS_D 0x0d
#define CMOS_NMI 0x80

void set_alarm(u32 secs);

//用于从 CMOS（实时时钟芯片）中读取数据
u8 cmos_read(u8 addr)
{
    outb(CMOS_ADDR, CMOS_NMI | addr);//写入的数据是为了告诉 CMOS 要读取或写入哪个特定的寄存器
    return inb(CMOS_DATA);//从特定的寄存器读取数据
};

// 写 cmos 寄存器的值
void cmos_write(u8 addr, u8 value)
{
    outb(CMOS_ADDR, CMOS_NMI | addr);
    outb(CMOS_DATA, value);
}

static u32 volatile counter = 0;

// 实时时钟中断处理函数
void rtc_handler(int vector)
{
    // 实时时钟中断向量号
    assert(vector == 0x28);

    // 向中断控制器发送中断处理完成的信号
    send_eoi(vector);

    // 读 CMOS 寄存器 C，允许 CMOS 继续产生中断
    cmos_read(CMOS_C);

    set_alarm(1);

    LOGK("rtc handler %d...\n", counter++);
}

// 设置 secs 秒后发生实时时钟中断
void set_alarm(u32 secs)
{
    tm time;
    time_read(&time);

    u8 sec = secs % 60;
    secs /= 60;
    u8 min = secs % 60;
    secs /= 60;
    u32 hour = secs;

    time.tm_sec += sec;
    if (time.tm_sec >= 60)
    {
        time.tm_sec %= 60;
        time.tm_min += 1;
    }

    time.tm_min += min;
    if (time.tm_min >= 60)
    {
        time.tm_min %= 60;
        time.tm_hour += 1;
    }

    time.tm_hour += hour;
    if (time.tm_hour >= 24)
    {
        time.tm_hour %= 24;
    }
    //将更新后的小时数写入到 CMOS 的小时寄存器（CMOS_HOUR）中。在写入之前，使用 bin_to_bcd 函数将二进制表示的小时数转换为 BCD 格式（Binary-Coded Decimal），以符合 CMOS 寄存器的格式要求。
    cmos_write(CMOS_HOUR, bin_to_bcd(time.tm_hour));
    //将更新后的分钟数写入到 CMOS 的分钟寄存器（CMOS_MINUTE）中。同样，先将二进制表示的分钟数转换为 BCD 格式。
    cmos_write(CMOS_MINUTE, bin_to_bcd(time.tm_min));
    //将更新后的秒数写入到 CMOS 的秒寄存器（CMOS_SECOND）中。同样，先将二进制表示的秒数转换为 BCD 格式。
    cmos_write(CMOS_SECOND, bin_to_bcd(time.tm_sec));
}

void rtc_init()
{
    u8 prev;

    // cmos_write(CMOS_B, 0b01000010); // 打开周期中断
    cmos_write(CMOS_B, 0b00100010); // 打开闹钟中断
    cmos_read(CMOS_C); // 读 C 寄存器，以允许 CMOS 中断

    set_alarm(2);

    // 设置中断频率
    outb(CMOS_A, (inb(CMOS_A) & 0xf) | 0b1110);

    set_interrupt_handler(IRQ_RTC, rtc_handler);
    set_interrupt_mask(IRQ_RTC, true);
    set_interrupt_mask(IRQ_CASCADE, true);
}