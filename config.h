#ifndef _CONFIG_H_
#define _CONFIG_H_
#include <msp430F149.h>

#define OK 1
#define ERROR 0

//延时函数，IAR自带，经常使用到
#define CPU_F ((double)8000000)   //外部高频晶振8MHZ
//#define CPU_F ((double)32768)   //外部低频晶振32.768KHZ
#define delay_us(x) __delay_cycles((long)(CPU_F*(double)x/1000000.0))
#define delay_ms(x) __delay_cycles((long)(CPU_F*(double)x/1000.0))

//自定义数据结构，方便使用
#define uchar unsigned char
#define uint  unsigned int
#define ulong unsigned long
//串口波特率计算，当BRCLK=CPU_F时用下面的公式可以计算，否则要根据设置加入分频系数
#define baud           9600                                //设置波特率的大小
#define baud_setting   (uint)((ulong)CPU_F/((ulong)baud))  //波特率计算公式
#define baud_h         (uchar)(baud_setting>>8)            //提取高位
#define baud_l         (uchar)(baud_setting)               //低位

//RS485控制管脚，CTR用于控制RS485处于收或者发状态
#define RS485_CTR1      P5OUT |= BIT2;          //控制线置高，RS485发送状态
#define RS485_CTR0      P5OUT &= ~BIT2;         //控制线置低，RS485接收状态

#endif
