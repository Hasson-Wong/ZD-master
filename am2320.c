#include "am2320.h"


unsigned char Sensor_Data[6]={0x00,0x00,0x00,0x00,0x00,0x00};
unsigned int  Sys_CNT;
unsigned char Sensor_AnswerFlag;  //收到传感器响应信号标志位，为1表示收到           //
unsigned char Sensor_ErrorFlag;   //读取传感器错误标志


/*******************端口初始化**********************************************/
void Port_Init(void)
{
  P2DIR |= BIT0;//P2.0设置为输出
  P2OUT &= ~BIT0; //主机拉低
  delay_ms(1);//主机拉低时间(Min=800US Max=20Ms)
  P2OUT |= BIT0;  //释放总线,主机拉高
  delay_us(30);//释放总线时间：Min=20us Max=200us
  P2DIR &= ~BIT0; //P2.0设置为输入，下一步等待传感器响应信号
}


/********************清除上次接收的数据****************************************/
void Clear_Data (void)
  {
    int i;
    for(i=0;i<5;i++)
    {
       Sensor_Data[i] = 0x00;
     }//接收数据清零
  }

/********************功能:读传感器发送的单个字节******************************/
uchar Read_SensorData(void)
  {
    uchar i,cnt;
    uchar buffer,tmp;
    buffer = 0;
    for(i=0;i<8;i++)
    {
        cnt=0;
        while(!(P2IN & BIT0))	//检测上次低电平是否结束
        {
          if((++cnt) >= 254)
           {
              break;
           }
        }
        //延时Min=22us Max30us 跳过数据"0" 的高电平
        delay_us(30);//延时30us
        //判断传感器发送数据位
        tmp =0;//高电平延时不高于30us，说明是信号0，先默认是信号0
        if((P2IN & BIT0))
        {
          tmp = 1;//高电平延时高于30us，说明是信号1
        }
        cnt =0;
        while((P2IN & BIT0))		//等待高电平 结束
        {
            if(++cnt >= 200)
            {
              break;
            }
        }
        buffer <<=1;
        buffer |= tmp;
    }
    return buffer;
  }

/*****************************功能:读传感器***********************************/
uchar Read_Sensor(void)
  {
    uchar i;
    Port_Init();
    Sensor_AnswerFlag = 0;  // 传感器响应标志
    //判断从机是否有低电平响应信号，如不响应则跳出，响应则向下运行
    if(!(P2IN & BIT0))
    {
       Sensor_AnswerFlag = 1;//收到起始信号
       Sys_CNT = 0;
       //判断从机是否发出 80us 的低电平响应信号是否结束
       while(!(P2IN & BIT0))
       {
         if(++Sys_CNT>300) //防止进入死循环
         {
           Sensor_ErrorFlag = 1;
           return 0;
          }
        }
        Sys_CNT = 0;
        //判断从机是否发出 80us 的高电平，如发出则进入数据接收状态
        while((P2IN & BIT0))
        {
           if(++Sys_CNT>300) //防止进入死循环
           {
             Sensor_ErrorFlag = 1;
             return 0;
           }
        }
        // 数据接收	传感器共发送40位数据
        // 即5个字节 高位先送  5个字节分别为湿度高位 湿度低位 温度高位 温度低位 校验和
        // 校验和为：湿度高位+湿度低位+温度高位+温度低位
        for(i=0;i<5;i++)
        {
          Sensor_Data[i] = Read_SensorData();
        }
            Sensor_Data[5]='\0';
      }
      else
      {
        Sensor_AnswerFlag = 0;	  // 未收到传感器响应
      }
      return 1;
  }
