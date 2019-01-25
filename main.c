#include <string.h>
#include <stdio.h>
#include "config.h"
#include "flash.h"
#include "am2320.h"
#include "uart.h"
#include "time.h"
#include "adc.h"


enum error
{
   run_OK,
   takein_err,
   sensor_err,
   cmd_err,
   run_err
};

enum stat
{
  take_in,
  read_sensor,
  cmd,
  sleep,
  handle
};

enum stat state=take_in;//����״̬
enum error error_code=run_OK;//������


unsigned int Sleep_Time = 20;       //˯�߶�ʱ


unsigned char location;//��Ч����λ��
unsigned char cmd_len;//�����

extern char  DataBuffer[32];//flash���ݻ���
extern unsigned char RxBuffer0[80];
extern unsigned char RxCounter0;
extern uchar UART0_flag;
extern uchar TimerOver_flag;  //��ʱ����ʱ��ɱ�־λ
extern uint Timer_A_Flag;

// ����������
extern unsigned char Sensor_AnswerFlag;  //�յ���������Ӧ�źű�־λ��Ϊ1��ʾ�յ�
//extern unsigned char Sensor_ErrorFlag;   //��ȡ�����������־
extern unsigned char Sensor_Data[6];    //��ʪ�ȴ������ɼ������������飺ʪ��H��ʪ��L���¶�H���¶�L��У���
unsigned char Sensor_Check;                 //����У���
unsigned int  Tmp1,Tmp2; //�¶Ⱥ�ʪ������

unsigned char time_interval[6];
unsigned char PORT[3];//�˿�����
unsigned char FRE[10];//Ƶ�� ͨ��
unsigned char Power[3];//����
unsigned char APPEUI[25]=" 2C 26 C5 01 C0 00 00 01";
unsigned char APPKEY[49]=" 00 11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff";
unsigned char MPA[7];//��������
unsigned char AT_CMD[80];

/***********************������***********************/
void main( void )
{
      WDTCTL = WDTPW + WDTHOLD;       //�رտ��Ź�
      Clock_Init();          //ʱ�ӳ�ʼ��
      
      P4DIR |= BIT1;         //���ģʽ WakeUp
      P4OUT |= BIT1;         //WakeUp
      
      P4DIR |= BIT0;                                                                                                                                                            ;         //���ģʽ��Mode
      P4OUT |= BIT0;         //mode
      
      delay_ms(150);         //�ȴ���Ƶģ���ʼ��150ms

      P4DIR &= ~BIT2;        //����ģʽ���ж�state
      P4DIR &= ~BIT3;        //����ģʽ���ж�busy

      InitUART();            //���ڳ�ʼ��
      TimerAInit();          //��ʱ����ʼ��
      TimerBInit();
      _EINT();               //�Ǵ�ȫ���ж�

     while(1)
     {
       switch (state)
        {
         case take_in:  
           
           
           if(0xcc==Flash_read_OneByte(Modify_Flag))
           {                
              P4OUT |= BIT0;               //P4��0λ���ģʽ������ߵ�ƽ,����ָ��ģʽ  
              delay_ms(1000);
              
              PutString("AT+RESET\r\n");   //6.��λ ����λ����Զ�ִ��join������
              
              delay_ms(1000);
  
              P4OUT &= ~BIT0;        //7.����͸��ģʽ(��ע�⣺ֻ����͸��ģʽ�£�ģ��Ż�ִ�м������)��
  
              delay_ms(1000);       //8.�������,��ģ�鱻ѡ��ΪOTAA�����λ�豸����Զ�ִ�м������磨jion������
              
           }
         else
            {  
              P4OUT |= BIT0;               //P4��0λ���ģʽ������ߵ�ƽ,����ָ��ģʽ  
              delay_ms(1000); 
              
              PutString("AT+OTAA=1\r\n");   //2.ѡ��OTAAģʽ
              delay_ms(1000);
  
             sprintf(AT_CMD,"AT+APPEUI=%s\r\n",APPEUI);
             PutString(AT_CMD);
          //    PutString("AT+APPEUI= 2C 26 C5 01 C0 00 00 01\r\n");    //3.���� AppEui
              delay_ms(1000);
  
              sprintf(AT_CMD,"AT+APPKEY=%s\r\n",APPKEY);
              PutString(AT_CMD); 
             // PutString("AT+APPKEY= 00 11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff\r\n");     //4.���� AppKey
              delay_ms(1000);
  
              PutString("AT+band = 0,0\r\n");       //4.����Ƶ��,��������Ĭ����band0��Ƶ�ʣ��Ժ�������ʲôƵ�����˻�˵���������Ǵ�0��3��ʼɨ��
              delay_ms(1000);
  
              PutString("AT+confirm = 1\r\n");       //4.���� ������ϢΪ��Ҫȷ����Ϣ
              delay_ms(1000);
  
              PutString("AT+SAVE\r\n");   //5.���棬�������κβ��������仯����Ҫִ�� SAVE ����λ�󣬲�����Ч��
              delay_ms(1000);
  
              PutString("AT+RESET\r\n");   //6.��λ ����λ����Զ�ִ��join������
              delay_ms(1000);
              
              while(0==P4IN&BIT2); 
              
              P4OUT &= ~BIT0;        //7.����͸��ģʽ(��ע�⣺ֻ����͸��ģʽ�£�ģ��Ż�ִ�м������)��                             
              delay_ms(1000);       //8.�������,��ģ�鱻ѡ��ΪOTAA�����λ�豸����Զ�ִ�м������磨jion������       
            }
            
           memset(RxBuffer0,0,RxCounter0);
           RxCounter0=0;   
           state=read_sensor;
           break;


         case read_sensor:
           
           Clear_Data();       //����ϴν��յ�����
           Read_Sensor();	   // ��ȡ����������
           delay_ms(2000);     //���βɼ����ʱ�䲻����2s
           Read_Sensor();	   // ��ȡ����������

           if(Sensor_AnswerFlag != 0)
           {
                Sensor_Check = Sensor_Data[0]+Sensor_Data[1]+Sensor_Data[2]+Sensor_Data[3];
                //У��ɹ�
                if(Sensor_Check == Sensor_Data[4])
                {        
                    Tmp1 = Sensor_Data[2]*256+Sensor_Data[3];
                    Tmp2 = Sensor_Data[0]*256+Sensor_Data[1];
                    if(Tmp1>550)
                      Sleep_Time=1;
                    while(1)
                    {
                      if(P4IN&BIT3)               //�ж�busy
                      {
                        UART_PutStringTransparent(Tmp1,Tmp2); //�û��ȴ� BUSY ������ STAT ����Ϊ�ߵ�ƽ������ģ�����ɹ���
                        break;
                      }
                    } 
                    //�ȴ�busy���жϸ�λ�������
                    //�ж�stat������ͣ���״̬�Ĵ�����������״̬�������������Ǹߣ�������
                    while(0==P4IN&BIT2);
                    delay_ms(1000);
                }
                 else	//У��ʧ��
                {
                    error_code=sensor_err;
                    state=handle;
                    break;
                }
          }
          else   // ������δ����
          {
               error_code=sensor_err;
               state=handle;
               break;
          }
            state=cmd;
           break;
           
         case cmd:
               if(1==UART0_flag)
               {
                   UART0_flag=0;                 
                   switch(RxBuffer0[0])
                   {
                   case 'M':                     
                       if(255!=(location=my_strstr(RxBuffer0,"MT1")))//�����޸������������ݼ�� <86400S
                       {
                           cmd_len=RxCounter0-3;
                           if(cmd_len>=5)
                           {           
                               memcpy(time_interval,&RxBuffer0[location+3],5);
                               Sleep_Time=(time_interval[0]-'0')*10000+(time_interval[1]-'0')*1000+(time_interval[2]-'0')*100+(time_interval[3]-'0')*10+(time_interval[4]-'0');
                          //     write_SegA(time_interval,FLASH_ADDRESS,5);     
                               P4OUT &= ~BIT0;        //����͸��ģʽ
                               delay_ms(1000);                          
                               PutString("MT1OK");                              
                               error_code=run_OK;
                           }
                           else
                           {  
                               error_code=cmd_err;
                               break;
                           }             
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;          
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"MT2")))//�����޸������������е��level��� <86400S
                       {         
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                           P4OUT &= ~BIT0;        //����͸��ģʽ
                           delay_ms(1000);
                           PutString("Reserved\r\n");
                           error_code=run_OK;
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"MAE")))//�����޸�APPEUI
                       {    
                           memset(AT_CMD,0,80);  
                           memset(APPEUI,0,25); 
                           cmd_len=RxCounter0-3;
                           if(cmd_len>=16)
                           {
                               memcpy(AT_CMD,&RxBuffer0[location+3],16);              
                               make_str(APPEUI,AT_CMD);
                               memset(AT_CMD,0,80);      
                               sprintf(AT_CMD,"AT+APPEUI=%s\r\n",APPEUI);
                               
                               P4OUT |= BIT0;//����ָ��ģʽ
                               delay_ms(100);
                               PutString(AT_CMD);
                               delay_ms(100);
                           //    Flash_write_OneByte(Modify_Flag,0xcc);
                               
                               PutString("AT+SAVE\r\n");  
                               delay_ms(1000);
                                
                               
                               P4OUT &= ~BIT0;        //����͸��ģʽ
                               delay_ms(1000);

                               PutString("MAEOK\r\n");  
                     
                               error_code=run_OK;
                           }
                           else
                           {
                               error_code=cmd_err;
                               break;
                           }      
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                           memset(APPEUI,0,strlen(APPEUI));
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"MAK")))//�����޸�APPKEY
                       {
                           memset(AT_CMD,0,80);
                           memset(APPKEY,0,49);   
                           cmd_len=RxCounter0-3;
                           if(cmd_len>=32)
                           {
                               memcpy(AT_CMD,&RxBuffer0[location+3],32);                             

                               make_str(APPKEY,AT_CMD);
                               memset(AT_CMD,0,80);
                               sprintf(AT_CMD,"AT+APPKEY=%s\r\n",APPKEY);
                               
                               P4OUT |= BIT0;//����ָ��ģʽ
                               delay_ms(100);
                               PutString(AT_CMD);
                               delay_ms(200);
                               
                           //    Flash_write_OneByte(Modify_Flag,0xcc);
                               PutString("AT+SAVE\r\n");   //5.���棬�������κβ��������仯����Ҫִ�� SAVE ����λ�󣬲�����Ч��
                               delay_ms(1000);
                               
                               P4OUT &= ~BIT0;        //����͸��ģʽ
                               delay_ms(1000);
                               
                               PutString("MAKOK\r\n");
                               error_code=run_OK;
                           }
                           else
                           {
                               error_code=cmd_err;
                               break;
                           }                  
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                           memset(APPKEY,0,strlen(APPKEY));
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"MPT")))//�����޸Ķ˿ں� 0x15~0xDF
                       {        
                           memset(AT_CMD,0,80);
                           cmd_len=RxCounter0-3;
                           if(cmd_len>=2)
                           {
                               memcpy(PORT,&RxBuffer0[location+3],2);                                      
                               sprintf(AT_CMD,"AT+PORT=%s\r\n",PORT);
                               
                               P4OUT |= BIT0;//����ָ��ģʽ
                               delay_ms(100);
                               PutString(AT_CMD);
                               delay_ms(1000);
                               
                          //     Flash_write_OneByte(Modify_Flag,0xcc);
                               PutString("AT+SAVE\r\n");   //5.���棬�������κβ��������仯����Ҫִ�� SAVE ����λ�󣬲�����Ч��
                               delay_ms(1000);
  
                               
                               while(0==P4IN&BIT2);
                               P4OUT &= ~BIT0;        //����͸��ģʽ    
                               
                               delay_ms(1000);
                               
                               PutString("MPTOK\r\n");
                               error_code=run_OK;
                           }
                           else
                           {
                                error_code=cmd_err;
                                break;
                           }
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                           memset(PORT,0,strlen(PORT));
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"MFQ")))//�����޸�Ƶ�ʻ��ŵ� �ŵ���0~80 471500000
                       {
                           memset(AT_CMD,0,80);
                           cmd_len=RxCounter0-3;
                           if(cmd_len>=9)
                           {
                              memcpy(FRE,&RxBuffer0[3],9);//9
   
                               memset(AT_CMD,0,80);
                               sprintf(AT_CMD,"AT+CSQ=%s\r\n",FRE);
                               
                               P4OUT |= BIT0;//����ָ��ģʽ
                               delay_ms(100);
                               PutString(AT_CMD);
                               
                           //    Flash_write_OneByte(Modify_Flag,0xcc);
                               PutString("AT+SAVE\r\n");   //5.���棬�������κβ��������仯����Ҫִ�� SAVE ����λ�󣬲�����Ч��
                               delay_ms(1000);
  

                               P4OUT &= ~BIT0;        //����͸��ģʽ
                               delay_ms(1000);
                               
                               PutString("MFQOK\r\n");                          
         
                               error_code=run_OK;
                           }
                           
                           else if(strlen(RxBuffer0)>=2)
                           {
                               memcpy(FRE,&RxBuffer0[3],2);//2                       
                               memset(AT_CMD,0,80);
                               sprintf(AT_CMD,"AT+CSQ=%s\r\n",FRE);
                               
                               P4OUT |= BIT0;//����ָ��ģʽ
                               delay_ms(100);
                               PutString(AT_CMD);
                               
                          //     Flash_write_OneByte(Modify_Flag,0xcc);
                               PutString("AT+SAVE\r\n");   //5.���棬�������κβ��������仯����Ҫִ�� SAVE ����λ�󣬲�����Ч��
                               delay_ms(1000);
  

                               P4OUT &= ~BIT0;        //����͸��ģʽ
                               delay_ms(1000);
                               PutString("MFQOK\r\n");
                               
                               error_code=run_OK;
                           }
                           else
                           {
                               error_code=cmd_err;
                               break;
                           }       
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                           memset(FRE,0,strlen(FRE));
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"MTP")))//�����޸ķ��书��
                       {
                           memset(AT_CMD,0,80);
                           cmd_len=RxCounter0-3;
                           if(cmd_len>=2)
                           {       
                             memcpy(FRE,&RxBuffer0[3],2);
                             sprintf(AT_CMD,"AT+CSQ=%s\r\n",FRE);
                             
                             P4OUT |= BIT0;//����ָ��ģʽ
                             delay_ms(100);
                             PutString(AT_CMD);
                             
                             PutString("AT+SAVE\r\n");   //5.���棬�������κβ��������仯����Ҫִ�� SAVE ����λ�󣬲�����Ч��
                             delay_ms(1000);
   

                             P4OUT &= ~BIT0;        //����͸��ģʽ
                             delay_ms(1000);             
                             PutString("MTPOK\r\n");;
                             
                             error_code=run_OK;
                           }
                           else
                           {
                               error_code=cmd_err;
                               break;
                           }
                           memset(RxBuffer0,0,RxCounter0); 
                           RxCounter0=0; 
                           memset(FRE,0,strlen(FRE));      
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"MPA")))//��������
                       {            
                         memset(AT_CMD,0,80);
                         cmd_len=RxCounter0-3;
                         if(RxCounter0>=6)
                         {
                               memcpy(MPA,&RxBuffer0[3],6);
                               P4OUT |= BIT0;//����ָ��ģʽ
                               delay_ms(100);
                               if('0'==MPA[0])//SF
                                 {

                                 }
                               else
                                 {
                                   // strncpy(AT_Temp,MPA,2);
                                  //  sprintf(AT_CMD,"AT+=%s\r\n",AT_Temp);//����δ��
                                   // PutString(AT_CMD);
                                 }

                               switch(MPA[2])//��Ϣ����
                               {
                                   case '0'://unconfirmed
                                       PutString("AT+confirm = 0\r\n");
                                       break;
                                   case '1'://confirmed
                                       PutString("AT+confirm = 1\r\n");
                                       break;
                                   case '2'://�޶���
                                       break;
                                   default:
                                       break;
                               }

                               switch(MPA[3])//���ʽ
                               {
                                   case '0'://ABP
                                       PutString("AT+OTAA=0\r\n");
                                       break;
                                   case '1'://OTAA
                                       PutString("AT+OTAA=1\r\n");
                                       break;
                                   case '2'://�޶���
                                       break;
                                   default:
                                       break;
                               }

                               switch(MPA[4])//ADR
                               {
                                   case '0'://�ر�ADR
                                       PutString("AT+ADR=0\r\n");
                                       break;
                                   case '1'://����ADR
                                       PutString("AT+ADR=1\r\n");
                                       break;
                                   case '2'://�޶���
                                       break;
                                   default:
                                       break;
                               }

                               switch(MPA[5])//Debugģʽ
                               {
                                   case '0'://�ر�Debugģʽ
                                     PutString("Debug=0\r\n");
                                       break;
                                   case '1'://����Debugģʽ
                                     PutString("Debug=1\r\n");
                                       break;
                                   case '2'://�޶���
                                     PutString("Debug=2\r\n");
                                       break;
                                   default:
                                       break;
                               }
                               
                           //    Flash_write_OneByte(Modify_Flag,0xcc);
                               PutString("AT+SAVE\r\n");   //5.���棬�������κβ��������仯����Ҫִ�� SAVE ����λ�󣬲�����Ч��
                               delay_ms(1000);
  

                               P4OUT &= ~BIT0;        //����͸��ģʽ
                               delay_ms(1000);
                               PutString("MPAOK\r\n");
                               
                               error_code=run_OK;
                             }
                          else
                             {
                                  error_code=cmd_err;
                                  break;
                             } 
                           memset(RxBuffer0,0,RxCounter0);  
                           RxCounter0=0;
                           memset(MPA,0,strlen(MPA));           
                       } 
                       break;
                       
                   case 'R':
                     
                       if(255!=(location=my_strstr(RxBuffer0,"RT1")))//���������������м��
                       { 
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;  
                         //  memset(time_interval,0,strlen(time_interval));    
                        //   Flash_read(FLASH_ADDRESS,5);
                        //   memcpy(time_interval,DataBuffer,5);
                            sprintf(AT_CMD,"T1%s\r\n",time_interval);
                            while(0==P4IN&BIT2);   
                               
                            P4OUT &= ~BIT0;        //����͸��ģʽ
                            delay_ms(1000);
                               
                            PutString(AT_CMD);                             
                            error_code=run_OK;

                           error_code=run_OK;
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"RT2")))//�������������м��
                       {
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                           P4OUT &= ~BIT0;        //����͸��ģʽ
                           delay_ms(1000);
                           PutString("T212345\r\n");//����           
                           error_code=run_OK;
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"RAE")))//����APPEUI
                       { 
                          memset(RxBuffer0,0,RxCounter0);
                          RxCounter0=0;                   
                             
                           memset(APPEUI,0,strlen(APPEUI));
                           memset(AT_CMD,0,80);
                           
                           P4OUT |= BIT0;//ָ��ģʽ
                           delay_ms(100);
                           
                           PutString("AT+APPEUI?\r\n");
                           while(1!=UART0_flag);
                           
                           UART0_flag=0;
                           if(255!=(location=my_strstr(RxBuffer0,"+APPEUI:")))
                           { 
                               memcpy(AT_CMD,&RxBuffer0[location+8],24);
                               unpack_str(APPEUI,AT_CMD);
               
                               sprintf(AT_CMD,"APPEUI%s\r\n",APPEUI);
                               
                               P4OUT &= ~BIT0;//͸��ģʽ
                               delay_ms(1000);
                               while(0==P4IN&BIT2);
                               
                               PutString(AT_CMD);                             
                               error_code=run_OK;

                           }else
                           {
                               error_code=cmd_err;
                               break;
                           }   
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"RAK")))//����APPKEY
                       {
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;     
                           memset(AT_CMD,0,80);
                            
                           P4OUT |= BIT0;//ָ��ģʽ
                           delay_ms(100);
                           PutString("AT+APPKEY?\r\n");
                           while(1!=UART0_flag);
                           
                           UART0_flag=0;
                           
                           if(255!=(location=my_strstr(RxBuffer0,"+APPKEY:")))
                           {

                               memcpy(AT_CMD,&RxBuffer0[location+8],48);        
                               unpack_str(APPKEY,AT_CMD);

                               sprintf(AT_CMD,"APPKEY%s\r\n",APPKEY);
                               P4OUT &= ~BIT0;//͸��ģʽ
                               delay_ms(100);
                               while(0==P4IN&BIT2);
                               PutString(AT_CMD);
                               
                               error_code=run_OK;

                           }else
                           {
                               error_code=cmd_err;
                               break;
                           }
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"RPT")))//��������ʹ��port
                       {
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                           
                           P4OUT |= BIT0;//ָ��ģʽ  
                           delay_ms(100);          
                           PutString("AT+PORT?\r\n");
                           
                           while(1!=UART0_flag);   
                           UART0_flag=0;
                           if(255!=(location=my_strstr(RxBuffer0,"+PORT:")))
                           {
                               P4OUT &= ~BIT0;//͸��ģʽ
                               delay_ms(100);
                               while(0==P4IN&BIT2);
                               
                               memcpy(PORT,&RxBuffer0[location+6],2);
                               sprintf(AT_CMD,"PORT%s\r\n",PORT); 
                               PutString(AT_CMD);
                               
                               error_code=run_OK;

                           }else
                           {
                               error_code=cmd_err;
                               break;
                           }
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                           memset(PORT,0,strlen(PORT));
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"RFQ")))//��������Ƶ��
                       {
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;

                           P4OUT |= BIT0;//ָ��ģʽ
                           delay_ms(100);
                           PutString("AT+CSQ?\r\n");
                           
                           while(1!=UART0_flag);
                           UART0_flag=0;
                           if(255!=(location=my_strstr(RxBuffer0,"+CSQ: [")))
                           {
                               memcpy(FRE,&RxBuffer0[location+7],9);
                               sprintf(AT_CMD,"FREQ%s\r\n",FRE);

                               P4OUT &= ~BIT0;//͸��ģʽ
                               delay_ms(100);
                               while(0==P4IN&BIT2);                       
                               PutString(AT_CMD);
        
                               error_code=run_OK;

                           }else
                           {
                               error_code=cmd_err;
                               break;
                           }
                            memset(RxBuffer0,0,RxCounter0);
                            RxCounter0=0;
                            memset(FRE,0,strlen(FRE)); 
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"RTP")))//�������з��书��
                       {
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0; 
                           P4OUT &= ~BIT0;        //����͸��ģʽ
                           delay_ms(1000);
                           PutString("reserved\r\n");//����
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"RSF")))//����������Ƶ����
                       {
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                           P4OUT &= ~BIT0;        //����͸��ģʽ
                           delay_ms(1000);
                           PutString("reserved\r\n");//����
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"RCF")))//������Ϣ������
                       {
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                           P4OUT |= BIT0;//ָ��ģʽ 
                           delay_ms(100);       

                           PutString("AT+CONFIRM?\r\n");
                           while(1!=UART0_flag);       
                           UART0_flag=0;        
                            if(255!=(location=my_strstr(RxBuffer0,"+CONFIRM: ")))
                            {      
                               sprintf(AT_CMD,"CONFSTAT%c\r\n",RxBuffer0[location+10]);
                               P4OUT &= ~BIT0;//͸��ģʽ
                               delay_ms(100);
                               while(0==P4IN&BIT2); 
                               PutString(AT_CMD);

                               error_code=run_OK;

                            }else
                            {
                                error_code=cmd_err;
                                break;
                            }
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"ROT")))//�������м��ʽ
                       {
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                           P4OUT |= BIT0;//ָ��ģʽ
                           delay_ms(100);
                           PutString("AT+OTAA?\r\n");
                           while(1!=UART0_flag);
                          
                           UART0_flag=0;
                           if(255!=(location=my_strstr(RxBuffer0,"+OTAA: ")))
                           {              
                               sprintf(AT_CMD,"OTAA%c\r\n",RxBuffer0[location+7]);
                               P4OUT &= ~BIT0;//͸��ģʽ
                               delay_ms(100);
                               while(0==P4IN&BIT2); 
                               PutString(AT_CMD);

                               error_code=run_OK;
                           }else
                            {
                               error_code=cmd_err;
                               break;
                            }
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0; 
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"RAD")))//��������ADR״̬
                       {
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                           
                           P4OUT |= BIT0;//ָ��ģʽ   
                           delay_ms(100);        
                           PutString("AT+ADR?\r\n");
                           while(1!=UART0_flag);
                           
                           UART0_flag=0;             
                           if(255!=(location=my_strstr(RxBuffer0,"+ADR: ")))
                           {
                               sprintf(AT_CMD,"ADR%c\r\n",RxBuffer0[location+6]);
                               P4OUT &= ~BIT0;//͸��ģʽ
                               delay_ms(100);
                               while(0==P4IN&BIT2); 
                               PutString(AT_CMD);

                               error_code=run_OK;
                           }else
                           {
                              error_code=cmd_err;
                              break;
                           }
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"RDB")))//���������Ƿ�debugģʽ
                       {
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                           P4OUT &= ~BIT0;        //����͸��ģʽ
                           delay_ms(1000);
                           PutString("reserved\r\n");//����
                           error_code=run_OK;
                       }
                       break;
                    default:
                       break;
                   }     
               }
             RxCounter0=0;//��������
             memset(RxBuffer0,0,strlen(RxBuffer0)); 
            state=handle;
           break;

         case handle:  
              
         P4OUT &= ~BIT0;        //����͸��ģʽ 
         delay_ms(1000);       
           switch(error_code)
           {
           case run_OK:
               state=sleep;
               break;
           case takein_err:  
               PutString("takein_err\r\n");       
               state=take_in;
               break;
           case sensor_err: 
             PutString("sensor_err\r\n");
               state=read_sensor;
             break;
           case cmd_err:      
           PutString("cmd_err\r\n");  
               state=read_sensor;
              break;
           case run_err:
             PutString("run_err\r\n");
               state=take_in;
              break;
           default:
              break;
           }
           break;
          case sleep:
             P4OUT &= ~BIT1;//ģ��˯��
             Timer_A_Flag=0;
             TimerOver_flag=0;
             
             LPM2;  //����LPM2ģʽ  
             
             P4OUT |= BIT1;//����ģ�� 
             while(0==P4IN&BIT2);
             
             PutString("\r\n");            
             state=read_sensor;
          break;
        default:
           break;
         }//end switch(state)
      }//end while()
}//end main

