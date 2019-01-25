#include "adc.h"


/********************************************
函数名称：Init_ADC
功    能：初始化ADC
参    数：无
返回值  ：无
********************************************/
void Init_ADC(void)
{
    P6SEL |= 0x03;                            // 使能ADC通道
    ADC12CTL0 = ADC12ON+SHT0_15+MSC;          // 打开ADC，设置采样时间
    ADC12CTL1 = SHP+CONSEQ_3;                 // 使用采样定时器
    //ADC12CTL1 |=0x06;                        //序列通道单词转换
    ADC12MCTL0=INCH_0;
    ADC12MCTL1=INCH_1+EOS;
    ADC12IE = 0x02;                           // 使能ADC中断
    ADC12CTL0 |= ENC;                         // 使能转换
    ADC12CTL0 |= ADC12SC;                     // 开始转换

    //_EINT();   //是打开全局中断
     //while((ADC12IFG & 0x0040) == 0); // 等待转换结束
    //_NOP(); // 处

    // ADC12 控制寄存器设置
    /* P6SEL |= 0x03;                            // 使能ADC通道
    ADC12CTL0 = ADC12ON + REFON + REF2_5V + SHT0_2;     // CONSEQ_1 表示当前模式为序列通道单次转换, 起始地址为 ADC12MCTL4, 结束地址 ADC12MCTL6
    ADC12CTL1 = ADC12SSEL_0 + SHP + CONSEQ_1 + CSTARTADD_4;   // 转换通道设置

    ADC12MCTL4 = SREF_1 + INCH_0; // 参考电压:V+=Vref+,V-=AVss ADC 通道:A0
    ADC12MCTL5 = SREF_1 + INCH_1; // 参考电压:V+=Vref+,V-=AVss ADC 通道:A1
    ADC12IE = 0x02;                           // 使能ADC中断
    ADC12CTL0 |= ENC + ADC12SC; // 转换使能开始转换
    while((ADC12IFG & 0x0040) == 0); // 等待转换结束
    _NOP(); // 处*/
}


