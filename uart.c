#include "uart.h"


unsigned char RxBuffer0[80];
unsigned char RxCounter0;
extern uint UART0_Rx_time_count;
/*******************************************
：函数名称InitUART
功    能：初始化UART端口
参    数：无
返回值  ：无
********************************************/
void InitUART(void)
{
    P3SEL |= 0x30;                            // 选择P3.4和P3.5做UART通信端口
    UCTL0 |= CHAR+SWRST;                      // 选择8位字符,复位SWRST
    UTCTL0 |= SSEL0;                          // UCLK = ACLK
    UBR00 = 0x03;                             // 波特率9600
    UBR10 = 0x00;                             //
    UMCTL0 = 0x4A;                            // Modulation
    UCTL0 &= ~SWRST;                          // 初始化UART状态机
    IE1 |= URXIE0;                            // 使能USART0的接收中断
    ME1 |= UTXE0 + URXE0;                     // 使能USART0的发送和接收
}


/*******************************************
函数名称：串口接收中断程序
功    能：发送GPRS心跳时钟
参    数：无
返回值  ：无
********************************************/
#pragma vector=UART0RX_VECTOR
__interrupt void usart0_rx (void)
{ 
  while (!(IFG1 & UTXIFG0));                // USART0 TX buffer ready
  if(RxCounter0<80)
    RxBuffer0[RxCounter0++] = RXBUF0;  
   UART0_Rx_time_count=0;  
}

/*******************************************
函数名称：Send1Char
功    能：单片机串口向外发送一个字符
参    数：sendchar--要发送的字符
返回值  ：无
********************************************/
void SendChar(uchar sendchar)
{
      while (!(IFG1&UTXIFG0));    //等待发送寄存器为空
      TXBUF0 = sendchar;
}

/*******************************************
函数名称：PutSting_WithNewLine
功    能：单片机向外发送字符串并换行指令
参    数：ptr--指向发送字符串的指针
返回值  ：无
********************************************/
void PutString_WithNewLine(uchar *ptr)
{
      while(*ptr != '\0')
      {
            SendChar(*ptr++);                     // 发送数据
      }
      while (!(IFG1 & UTXIFG0));
      TXBUF0 = '\n';                              //发送换行指令
}
/*******************************************
函数名称：PutSting
功    能：向PC机发送字符串，无换行
参    数：ptr--指向发送字符串的指针
返回值  ：无
********************************************/
void PutString(uchar *ptr)
{
      while(*ptr != '\0')
      {
            SendChar(*ptr++);                     // 发送数据
      }
}

void PutStringOn(uchar *ptr,uchar len)
{
  while(len--)
  {
    SendChar(*ptr++); 
  }
}

void UART_PutStringTransparent(unsigned int num1,unsigned int num2)
{
  unsigned char a[5],i,b[5],j;    //发送函数内部变量

  a[4] = '0'+num1%10;
  a[3] = '.';
  a[2] = '0'+num1/10%10;
  a[1] = '0'+num1/100%10;
  a[0] = ';';

  b[4] = '0'+num2%10;
  b[3] = '.';
  b[2] = '0'+num2/10%10;
  b[1] = '0'+num2/100%10;
  b[0] = ';';

 // PutString("Temp: \r\n");
  for(i=0;i<5;i++)
  {
      SendChar(a[i]);
  }

  //PutString("Rh: \r\n");
  for(j=0;j<5;j++)
  {
      SendChar(b[j]);
  }
}




unsigned char my_strstr(unsigned char * source,unsigned  char * dest)
{
	unsigned char res;
	unsigned char i,j;
	for(i=0;(source[i]!='\0')&&(i<254);i++)
	{
		res = i;
		j=0;
		while(source[i++]==dest[j++])
		{
			if(dest[j]=='\0')
			{
				return res;
			}
		}
		i = res;
	}
	return 255;
}


void make_str(unsigned char * dst,unsigned char * src)
{
    int i=1,j=0;
    dst[0]=' ';
    while( (src[j]!='\0') && (i<254) )
    {
        if(0==i%3)
            dst[i++]=' ';
        else
            dst[i++]=src[j++];
    }
    dst[i++]='\0';
}

void unpack_str(unsigned char * dst,unsigned char * src)
{
    int i=0,j=0;
    while( (src[j]!='\0') && (i<254) )
    {
        j++;     
        if(src[j]==' ')
          continue;
        else
        dst[i++]=src[j];
    }
    dst[i++]='\0';
}

