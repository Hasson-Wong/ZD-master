#include "adc.h"


/********************************************
�������ƣ�Init_ADC
��    �ܣ���ʼ��ADC
��    ������
����ֵ  ����
********************************************/
void Init_ADC(void)
{
    P6SEL |= 0x03;                            // ʹ��ADCͨ��
    ADC12CTL0 = ADC12ON+SHT0_15+MSC;          // ��ADC�����ò���ʱ��
    ADC12CTL1 = SHP+CONSEQ_3;                 // ʹ�ò�����ʱ��
    //ADC12CTL1 |=0x06;                        //����ͨ������ת��
    ADC12MCTL0=INCH_0;
    ADC12MCTL1=INCH_1+EOS;
    ADC12IE = 0x02;                           // ʹ��ADC�ж�
    ADC12CTL0 |= ENC;                         // ʹ��ת��
    ADC12CTL0 |= ADC12SC;                     // ��ʼת��

    //_EINT();   //�Ǵ�ȫ���ж�
     //while((ADC12IFG & 0x0040) == 0); // �ȴ�ת������
    //_NOP(); // ��

    // ADC12 ���ƼĴ�������
    /* P6SEL |= 0x03;                            // ʹ��ADCͨ��
    ADC12CTL0 = ADC12ON + REFON + REF2_5V + SHT0_2;     // CONSEQ_1 ��ʾ��ǰģʽΪ����ͨ������ת��, ��ʼ��ַΪ ADC12MCTL4, ������ַ ADC12MCTL6
    ADC12CTL1 = ADC12SSEL_0 + SHP + CONSEQ_1 + CSTARTADD_4;   // ת��ͨ������

    ADC12MCTL4 = SREF_1 + INCH_0; // �ο���ѹ:V+=Vref+,V-=AVss ADC ͨ��:A0
    ADC12MCTL5 = SREF_1 + INCH_1; // �ο���ѹ:V+=Vref+,V-=AVss ADC ͨ��:A1
    ADC12IE = 0x02;                           // ʹ��ADC�ж�
    ADC12CTL0 |= ENC + ADC12SC; // ת��ʹ�ܿ�ʼת��
    while((ADC12IFG & 0x0040) == 0); // �ȴ�ת������
    _NOP(); // ��*/
}


