#include <onix/onix.h>
#include <onix/io.h>
#include <onix/types.h>
#include<onix/console.h>

#define CRT_ADDR_REG 0x3d4
#define CRT_DATA_REG 0x3d5

#define CRT_CURSOR_H 0xe
#define CRT_CURSOR_L 0xf

char message[] = "hello onix!!!\n";

void kernel_init()
{
   console_init();
   while(true)
      console_write(message,sizeof(message)-1);
}