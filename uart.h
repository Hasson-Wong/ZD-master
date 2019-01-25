#ifndef _UART_H_
#define _UART_H_
#include "config.h"

void InitUART(void);
void SendChar(uchar sendchar);
void PutString(uchar *ptr);
void PutStringOn(uchar *ptr,uchar len);
void PutString_WithNewLine(uchar *ptr);
void UART_PutStringTransparent(unsigned int num1,unsigned int num2);   //Í¸´«Êý¾Ý
unsigned char my_strstr(unsigned char * source, unsigned char * dest);
void make_str(unsigned char * dst,unsigned char * src);
void unpack_str(unsigned char * dst,unsigned char * src);


#endif
