#ifndef _TIME_H_
#define _TIME_H_
#include "config.h"

void TimerAInit(void);          //��ʱ����ʼ��
void TimerBInit(void);
void Clock_Init(void);          //ϵͳʱ�ӳ�ʼ�����ⲿ8M����
void Clock_Init_Inc(void);      //ϵͳʱ�ӳ�ʼ�����ڲ�RC����
void Clock_Init_Ex32768(void);  //ϵͳʱ�ӳ�ʼ�����ⲿ32.768K����

#endif
