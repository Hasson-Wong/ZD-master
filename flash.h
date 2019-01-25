#ifndef _FLASH_H_
#define _FLASH_H_
#include "config.h"

#define FLASH_ADDRESS (char *)0x1080
#define Modify_Flag   (char *)0x1085

/*
#define APPEUI_ADDR   (char *)0x01000
#define APPKEY_ADDR   (char *)0x01010
#define TIME_ADDR     (char *)0x01030
#define PORT_ADDR     (char *)0x01035
#define FRE_ADDR      (char *)0x01037
#define POWER_ADDR    (char *)0x01040
#define MPA_ADDR      (char *)0x01042
*/

void write_SegA(char * value,char *addr, char len);//Ð´flash
void Flash_read(char *addr, char len);//¶Áflash
char Flash_read_OneByte(char *addr);
void Flash_write_OneByte(char *addr,char value);
#endif
