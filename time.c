#include "time.h"

#define UART_TIMEOUT 150

uint Timer_A_Flag = 0;           //�޸�����������Զ�ʱ�ĳ���
uchar TimerOver_flag=0;  //��ʱ����ʱ��ɱ�־λ

extern unsigned int Sleep_Time;

extern uchar RxCounter0;
uchar UART0_flag;
uint UART0_Rx_time_count;
/*******************************************
�������ƣ���ʱ����ʼ��
��    �ܣ���ʱʱ��1Сʱ����ʱ������������
��    ������
����ֵ  ����
********************************************/
void TimerAInit(void)
{
    TACCTL0 = CCIE;        // CCR0 interrupt enabled
    TACTL = TASSEL_1 + ID_3 + MC_1; //��ʱ��A��ʱ��Դѡ��ACLK(32.768KHz)��������ģʽ,8��Ƶ
   // TACCR0 = 65535;                    //�趨����0.5S     1/32.768K*8*2048=0.5S   
     TACCR0 = 4095;// (1/32768)*4096
    CCTL0 |= CCIE;                   //ʹ��CCR0�жϡ���CCIE��ʹ��CCR0�ж�

}

void TimerBInit(void)
{
    TBCCTL0 = CCIE;        // CCR0 interrupt enabled
    TBCTL = TBSSEL_1 + ID_3 + MC_1; //��ʱ��A��ʱ��Դѡ��ACLK(32.768KHz)��������ģʽ,8��Ƶ
    TBCCR0 = 7; //2ms
    CCTL0 |= CCIE;                   //ʹ��CCR0�жϡ���CCIE��ʹ��CCR0�ж� 
}


//***********************************************************************
//                   ϵͳʱ�ӳ�ʼ�����ⲿ8M����
//***********************************************************************
void Clock_Init(void)
{
  uchar i;
  BCSCTL1&=~XT2OFF;                 //��XT2����
  BCSCTL2|=SELM1+SELS;              //MCLKΪ8MHZ��SMCLKΪ8MHZ
  do{
    IFG1&=~OFIFG;                   //������������־
    for(i=0;i<100;i++)
       _NOP();
  }
  while((IFG1&OFIFG)!=0);           //�����־λ1�������ѭ���ȴ�
  IFG1&=~OFIFG;
}

//***********************************************************************
//                   ϵͳʱ�ӳ�ʼ�����ڲ�RC����
//***********************************************************************
void Clock_Init_Inc(void)
{
  //uchar i;

 // DCOCTL = DCO0 + DCO1 + DCO2;              // Max DCO
 // BCSCTL1 = RSEL0 + RSEL1 + RSEL2;          // XT2on, max RSEL

  DCOCTL = 0x60 + 0x00;                       //DCOԼ3MHZ��3030KHZ
  BCSCTL1 = DIVA_0 + 0x07;
  BCSCTL2 = SELM_2 + DIVM_0 + SELS + DIVS_0;
}

//***********************************************************************
//                   ϵͳʱ�ӳ�ʼ�����ⲿ32.768K����
//***********************************************************************
void Clock_Init_Ex32768(void)
{
  uchar i;

  BCSCTL2|=SELM1 + SELM0 + SELS;    //MCLKΪ32.768KHZ��SMCLKΪ8MHZ
  do{
    IFG1&=~OFIFG;                   //������������־
    for(i=0;i<100;i++)
       _NOP();
  }
  while((IFG1&OFIFG)!=0);           //�����־λ1�������ѭ���ȴ�
  IFG1&=~OFIFG;
}

/*******************************************
�������ƣ���ʱ���жϳ���
��    �ܣ�������ʪ����Ϣ���ڶ�ʱ
��    ������
����ֵ  ����
********************************************/
#pragma vector = TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
        
        Timer_A_Flag++;
        if(Timer_A_Flag >= Sleep_Time)    //��flag=4ʱΪ64s,��flag=15ʱΪ4���ӣ���flag=75ʱΪ20���ӣ���flag=225ʱΪ1Сʱ
        {
             Timer_A_Flag=0;
             if(0==TimerOver_flag)
             {
               TimerOver_flag = 1;
               LPM2_EXIT;//�˳�LPM2ģʽ
             }
            // ADC12IE = 0x01;  //��ADC�ж�
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