#include "time.h"

#define UART_TIMEOUT 150

uint Timer_A_Flag = 0;           //修改这个参数可以定时的长度
uchar TimerOver_flag=0;  //定时器定时完成标志位

extern unsigned int Sleep_Time;

extern uchar RxCounter0;
uchar UART0_flag;
uint UART0_Rx_time_count;
/*******************************************
函数名称：定时器初始化
功    能：定时时间1小时，暂时不发心跳程序
参    数：无
返回值  ：无
********************************************/
void TimerAInit(void)
{
    TACCTL0 = CCIE;        // CCR0 interrupt enabled
    TACTL = TASSEL_1 + ID_3 + MC_1; //定时器A的时钟源选择ACLK(32.768KHz)，增计数模式,8分频
   // TACCR0 = 65535;                    //设定周期0.5S     1/32.768K*8*2048=0.5S   
     TACCR0 = 4095;// (1/32768)*4096
    CCTL0 |= CCIE;                   //使能CCR0中断——CCIE：使能CCR0中断

}

void TimerBInit(void)
{
    TBCCTL0 = CCIE;        // CCR0 interrupt enabled
    TBCTL = TBSSEL_1 + ID_3 + MC_1; //定时器A的时钟源选择ACLK(32.768KHz)，增计数模式,8分频
    TBCCR0 = 7; //2ms
    CCTL0 |= CCIE;                   //使能CCR0中断——CCIE：使能CCR0中断 
}


//***********************************************************************
//                   系统时钟初始化，外部8M晶振
//***********************************************************************
void Clock_Init(void)
{
  uchar i;
  BCSCTL1&=~XT2OFF;                 //打开XT2振荡器
  BCSCTL2|=SELM1+SELS;              //MCLK为8MHZ，SMCLK为8MHZ
  do{
    IFG1&=~OFIFG;                   //清楚振荡器错误标志
    for(i=0;i<100;i++)
       _NOP();
  }
  while((IFG1&OFIFG)!=0);           //如果标志位1，则继续循环等待
  IFG1&=~OFIFG;
}

//***********************************************************************
//                   系统时钟初始化，内部RC晶振
//***********************************************************************
void Clock_Init_Inc(void)
{
  //uchar i;

 // DCOCTL = DCO0 + DCO1 + DCO2;              // Max DCO
 // BCSCTL1 = RSEL0 + RSEL1 + RSEL2;          // XT2on, max RSEL

  DCOCTL = 0x60 + 0x00;                       //DCO约3MHZ，3030KHZ
  BCSCTL1 = DIVA_0 + 0x07;
  BCSCTL2 = SELM_2 + DIVM_0 + SELS + DIVS_0;
}

//***********************************************************************
//                   系统时钟初始化，外部32.768K晶振
//***********************************************************************
void Clock_Init_Ex32768(void)
{
  uchar i;

  BCSCTL2|=SELM1 + SELM0 + SELS;    //MCLK为32.768KHZ，SMCLK为8MHZ
  do{
    IFG1&=~OFIFG;                   //清楚振荡器错误标志
    for(i=0;i<100;i++)
       _NOP();
  }
  while((IFG1&OFIFG)!=0);           //如果标志位1，则继续循环等待
  IFG1&=~OFIFG;
}

/*******************************************
函数名称：定时器中断程序
功    能：发送温湿度信息周期定时
参    数：无
返回值  ：无
********************************************/
#pragma vector = TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
        
        Timer_A_Flag++;
        if(Timer_A_Flag >= Sleep_Time)    //当flag=4时为64s,当flag=15时为4分钟，当flag=75时为20分钟，当flag=225时为1小时
        {
             Timer_A_Flag=0;
             if(0==TimerOver_flag)
             {
               TimerOver_flag = 1;
               LPM2_EXIT;//退出LPM2模式
             }
            // ADC12IE = 0x01;  //打开ADC中断
        }
        //CCTL0 &= ~CCIE;
}

#pragma vector = TIMERB0_VECTOR
__interrupt void Timer_B (void)
{   
    if(UART0_Rx_time_count<=UART_TIMEOUT)
	{
          UART0_Rx_time_count++;
	}
	
	if((UART0_flag==0)&&(UART0_Rx_time_count==UART_TIMEOUT)&&(RxCounter0>0))
	{
		UART0_flag = 1;
		UART0_Rx_time_count++;
	}
}