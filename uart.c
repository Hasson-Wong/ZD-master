#include "uart.h"


unsigned char RxBuffer0[80];
unsigned char RxCounter0;
extern uint UART0_Rx_time_count;
/*******************************************
����������InitUART
��    �ܣ���ʼ��UART�˿�
��    ������
����ֵ  ����
********************************************/
void InitUART(void)
{
    P3SEL |= 0x30;                            // ѡ��P3.4��P3.5��UARTͨ�Ŷ˿�
    UCTL0 |= CHAR+SWRST;                      // ѡ��8λ�ַ�,��λSWRST
    UTCTL0 |= SSEL0;                          // UCLK = ACLK
    UBR00 = 0x03;                             // ������9600
    UBR10 = 0x00;                             //
    UMCTL0 = 0x4A;                            // Modulation
    UCTL0 &= ~SWRST;                          // ��ʼ��UART״̬��
    IE1 |= URXIE0;                            // ʹ��USART0�Ľ����ж�
    ME1 |= UTXE0 + URXE0;                     // ʹ��USART0�ķ��ͺͽ���
}


/*******************************************
�������ƣ����ڽ����жϳ���
��    �ܣ�����GPRS����ʱ��
��    ������
����ֵ  ����
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
�������ƣ�Send1Char
��    �ܣ���Ƭ���������ⷢ��һ���ַ�
��    ����sendchar--Ҫ���͵��ַ�
����ֵ  ����
********************************************/
void SendChar(uchar sendchar)
{
      while (!(IFG1&UTXIFG0));    //�ȴ����ͼĴ���Ϊ��
      TXBUF0 = sendchar;
}

/*******************************************
�������ƣ�PutSting_WithNewLine
��    �ܣ���Ƭ�����ⷢ���ַ���������ָ��
��    ����ptr--ָ�����ַ�����ָ��
����ֵ  ����
********************************************/
void PutString_WithNewLine(uchar *ptr)
{
      while(*ptr != '\0')
      {
            SendChar(*ptr++);                     // ��������
      }
      while (!(IFG1 & UTXIFG0));
      TXBUF0 = '\n';                              //���ͻ���ָ��
}
/*******************************************
�������ƣ�PutSting
��    �ܣ���PC�������ַ������޻���
��    ����ptr--ָ�����ַ�����ָ��
����ֵ  ����
********************************************/
void PutString(uchar *ptr)
{
      while(*ptr != '\0')
      {
            SendChar(*ptr++);                     // ��������
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
  unsigned char a[5],i,b[5],j;    //���ͺ����ڲ�����

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

