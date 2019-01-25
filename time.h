#ifndef _TIME_H_
#define _TIME_H_
#include "config.h"

void TimerAInit(void);          //定时器初始化
void TimerBInit(void);
void Clock_Init(void);          //系统时钟初始化，外部8M晶振
void Clock_Init_Inc(void);      //系统时钟初始化，内部RC晶振
void Clock_Init_Ex32768(void);  //系统时钟初始化，外部32.768K晶振

#endif
