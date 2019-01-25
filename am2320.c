#include "am2320.h"


unsigned char Sensor_Data[6]={0x00,0x00,0x00,0x00,0x00,0x00};
unsigned int  Sys_CNT;
unsigned char Sensor_AnswerFlag;  //�յ���������Ӧ�źű�־λ��Ϊ1��ʾ�յ�           //
unsigned char Sensor_ErrorFlag;   //��ȡ�����������־


/*******************�˿ڳ�ʼ��**********************************************/
void Port_Init(void)
{
  P2DIR |= BIT0;//P2.0����Ϊ���
  P2OUT &= ~BIT0; //��������
  delay_ms(1);//��������ʱ��(Min=800US Max=20Ms)
  P2OUT |= BIT0;  //�ͷ�����,��������
  delay_us(30);//�ͷ�����ʱ�䣺Min=20us Max=200us
  P2DIR &= ~BIT0; //P2.0����Ϊ���룬��һ���ȴ���������Ӧ�ź�
}


/********************����ϴν��յ�����****************************************/
void Clear_Data (void)
  {
    int i;
    for(i=0;i<5;i++)
    {
       Sensor_Data[i] = 0x00;
     }//������������
  }

/********************����:�����������͵ĵ����ֽ�******************************/
uchar Read_SensorData(void)
  {
    uchar i,cnt;
    uchar buffer,tmp;
    buffer = 0;
    for(i=0;i<8;i++)
    {
        cnt=0;
        while(!(P2IN & BIT0))	//����ϴε͵�ƽ�Ƿ����
        {
          if((++cnt) >= 254)
           {
              break;
           }
        }
        //��ʱMin=22us Max30us ��������"0" �ĸߵ�ƽ
        delay_us(30);//��ʱ30us
        //�жϴ�������������λ
        tmp =0;//�ߵ�ƽ��ʱ������30us��˵�����ź�0����Ĭ�����ź�0
        if((P2IN & BIT0))
        {
          tmp = 1;//�ߵ�ƽ��ʱ����30us��˵�����ź�1
        }
        cnt =0;
        while((P2IN & BIT0))		//�ȴ��ߵ�ƽ ����
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

/*****************************����:��������***********************************/
uchar Read_Sensor(void)
  {
    uchar i;
    Port_Init();
    Sensor_AnswerFlag = 0;  // ��������Ӧ��־
    //�жϴӻ��Ƿ��е͵�ƽ��Ӧ�źţ��粻��Ӧ����������Ӧ����������
    if(!(P2IN & BIT0))
    {
       Sensor_AnswerFlag = 1;//�յ���ʼ�ź�
       Sys_CNT = 0;
       //�жϴӻ��Ƿ񷢳� 80us �ĵ͵�ƽ��Ӧ�ź��Ƿ����
       while(!(P2IN & BIT0))
       {
         if(++Sys_CNT>300) //��ֹ������ѭ��
         {
           Sensor_ErrorFlag = 1;
           return 0;
          }
        }
        Sys_CNT = 0;
        //�жϴӻ��Ƿ񷢳� 80us �ĸߵ�ƽ���緢����������ݽ���״̬
        while((P2IN & BIT0))
        {
           if(++Sys_CNT>300) //��ֹ������ѭ��
           {
             Sensor_ErrorFlag = 1;
             return 0;
           }
        }
        // ���ݽ���	������������40λ����
        // ��5���ֽ� ��λ����  5���ֽڷֱ�Ϊʪ�ȸ�λ ʪ�ȵ�λ �¶ȸ�λ �¶ȵ�λ У���
        // У���Ϊ��ʪ�ȸ�λ+ʪ�ȵ�λ+�¶ȸ�λ+�¶ȵ�λ
        for(i=0;i<5;i++)
        {
          Sensor_Data[i] = Read_SensorData();
        }
            Sensor_Data[5]='\0';
      }
      else
      {
        Sensor_AnswerFlag = 0;	  // δ�յ���������Ӧ
      }
      return 1;
  }
