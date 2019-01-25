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

enum stat state=take_in;//运行状态
enum error error_code=run_OK;//错误码


unsigned int Sleep_Time = 20;       //睡眠定时


unsigned char location;//有效命令位置
unsigned char cmd_len;//命令长度

extern char  DataBuffer[32];//flash数据缓存
extern unsigned char RxBuffer0[80];
extern unsigned char RxCounter0;
extern uchar UART0_flag;
extern uchar TimerOver_flag;  //定时器定时完成标志位
extern uint Timer_A_Flag;

// 传感器配置
extern unsigned char Sensor_AnswerFlag;  //收到传感器响应信号标志位，为1表示收到
//extern unsigned char Sensor_ErrorFlag;   //读取传感器错误标志
extern unsigned char Sensor_Data[6];    //温湿度传感器采集的数据量数组：湿度H，湿度L，温度H，温度L，校验和
unsigned char Sensor_Check;                 //参照校验和
unsigned int  Tmp1,Tmp2; //温度和湿度数据

unsigned char time_interval[6];
unsigned char PORT[3];//端口设置
unsigned char FRE[10];//频率 通道
unsigned char Power[3];//功率
unsigned char APPEUI[25]=" 2C 26 C5 01 C0 00 00 01";
unsigned char APPKEY[49]=" 00 11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff";
unsigned char MPA[7];//其它设置
unsigned char AT_CMD[80];

/***********************主函数***********************/
void main( void )
{
      WDTCTL = WDTPW + WDTHOLD;       //关闭看门狗
      Clock_Init();          //时钟初始化
      
      P4DIR |= BIT1;         //输出模式 WakeUp
      P4OUT |= BIT1;         //WakeUp
      
      P4DIR |= BIT0;                                                                                                                                                            ;         //输出模式，Mode
      P4OUT |= BIT0;         //mode
      
      delay_ms(150);         //等待射频模块初始化150ms

      P4DIR &= ~BIT2;        //输入模式，判断state
      P4DIR &= ~BIT3;        //输入模式，判断busy

      InitUART();            //串口初始化
      TimerAInit();          //定时器初始化
      TimerBInit();
      _EINT();               //是打开全局中断

     while(1)
     {
       switch (state)
        {
         case take_in:  
           
           
           if(0xcc==Flash_read_OneByte(Modify_Flag))
           {                
              P4OUT |= BIT0;               //P4第0位输出模式，输出高电平,进入指令模式  
              delay_ms(1000);
              
              PutString("AT+RESET\r\n");   //6.复位 （复位后会自动执行join动作）
              
              delay_ms(1000);
  
              P4OUT &= ~BIT0;        //7.进入透传模式(请注意：只有在透传模式下，模块才会执行加入操作)。
  
              delay_ms(1000);       //8.加入操作,若模块被选定为OTAA激活，复位设备后会自动执行加入网络（jion）操作
              
           }
         else
            {  
              P4OUT |= BIT0;               //P4第0位输出模式，输出高电平,进入指令模式  
              delay_ms(1000); 
              
              PutString("AT+OTAA=1\r\n");   //2.选择OTAA模式
              delay_ms(1000);
  
             sprintf(AT_CMD,"AT+APPEUI=%s\r\n",APPEUI);
             PutString(AT_CMD);
          //    PutString("AT+APPEUI= 2C 26 C5 01 C0 00 00 01\r\n");    //3.配置 AppEui
              delay_ms(1000);
  
              sprintf(AT_CMD,"AT+APPKEY=%s\r\n",APPKEY);
              PutString(AT_CMD); 
             // PutString("AT+APPKEY= 00 11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff\r\n");     //4.配置 AppKey
              delay_ms(1000);
  
              PutString("AT+band = 0,0\r\n");       //4.配置频率,现在中兴默认是band0的频率，以后网关是什么频率中兴会说明，或者是从0到3开始扫描
              delay_ms(1000);
  
              PutString("AT+confirm = 1\r\n");       //4.配置 发送消息为需要确认消息
              delay_ms(1000);
  
              PutString("AT+SAVE\r\n");   //5.保存，（以上任何参数发生变化，需要执行 SAVE 并复位后，才能生效）
              delay_ms(1000);
  
              PutString("AT+RESET\r\n");   //6.复位 （复位后会自动执行join动作）
              delay_ms(1000);
              
              while(0==P4IN&BIT2); 
              
              P4OUT &= ~BIT0;        //7.进入透传模式(请注意：只有在透传模式下，模块才会执行加入操作)。                             
              delay_ms(1000);       //8.加入操作,若模块被选定为OTAA激活，复位设备后会自动执行加入网络（jion）操作       
            }
            
           memset(RxBuffer0,0,RxCounter0);
           RxCounter0=0;   
           state=read_sensor;
           break;


         case read_sensor:
           
           Clear_Data();       //清除上次接收的数据
           Read_Sensor();	   // 读取传感器数据
           delay_ms(2000);     //两次采集间隔时间不低于2s
           Read_Sensor();	   // 读取传感器数据

           if(Sensor_AnswerFlag != 0)
           {
                Sensor_Check = Sensor_Data[0]+Sensor_Data[1]+Sensor_Data[2]+Sensor_Data[3];
                //校验成功
                if(Sensor_Check == Sensor_Data[4])
                {        
                    Tmp1 = Sensor_Data[2]*256+Sensor_Data[3];
                    Tmp2 = Sensor_Data[0]*256+Sensor_Data[1];
                    if(Tmp1>550)
                      Sleep_Time=1;
                    while(1)
                    {
                      if(P4IN&BIT3)               //判断busy
                      {
                        UART_PutStringTransparent(Tmp1,Tmp2); //用户等待 BUSY 引脚与 STAT 引脚为高电平，表明模块加入成功。
                        break;
                      }
                    } 
                    //等待busy，判断高位发送完成
                    //判断stat，如果低，读状态寄存器，（发送状态及结果），如果是高，正常的
                    while(0==P4IN&BIT2);
                    delay_ms(1000);
                }
                 else	//校验失败
                {
                    error_code=sensor_err;
                    state=handle;
                    break;
                }
          }
          else   // 传感器未连接
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
                       if(255!=(location=my_strstr(RxBuffer0,"MT1")))//请求修改例行上行数据间隔 <86400S
                       {
                           cmd_len=RxCounter0-3;
                           if(cmd_len>=5)
                           {           
                               memcpy(time_interval,&RxBuffer0[location+3],5);
                               Sleep_Time=(time_interval[0]-'0')*10000+(time_interval[1]-'0')*1000+(time_interval[2]-'0')*100+(time_interval[3]-'0')*10+(time_interval[4]-'0');
                          //     write_SegA(time_interval,FLASH_ADDRESS,5);     
                               P4OUT &= ~BIT0;        //进入透传模式
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
                       if(255!=(location=my_strstr(RxBuffer0,"MT2")))//请求修改例行数据上行电池level间隔 <86400S
                       {         
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                           P4OUT &= ~BIT0;        //进入透传模式
                           delay_ms(1000);
                           PutString("Reserved\r\n");
                           error_code=run_OK;
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"MAE")))//请求修改APPEUI
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
                               
                               P4OUT |= BIT0;//进入指令模式
                               delay_ms(100);
                               PutString(AT_CMD);
                               delay_ms(100);
                           //    Flash_write_OneByte(Modify_Flag,0xcc);
                               
                               PutString("AT+SAVE\r\n");  
                               delay_ms(1000);
                                
                               
                               P4OUT &= ~BIT0;        //进入透传模式
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
                       if(255!=(location=my_strstr(RxBuffer0,"MAK")))//请求修改APPKEY
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
                               
                               P4OUT |= BIT0;//进入指令模式
                               delay_ms(100);
                               PutString(AT_CMD);
                               delay_ms(200);
                               
                           //    Flash_write_OneByte(Modify_Flag,0xcc);
                               PutString("AT+SAVE\r\n");   //5.保存，（以上任何参数发生变化，需要执行 SAVE 并复位后，才能生效）
                               delay_ms(1000);
                               
                               P4OUT &= ~BIT0;        //进入透传模式
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
                       if(255!=(location=my_strstr(RxBuffer0,"MPT")))//请求修改端口号 0x15~0xDF
                       {        
                           memset(AT_CMD,0,80);
                           cmd_len=RxCounter0-3;
                           if(cmd_len>=2)
                           {
                               memcpy(PORT,&RxBuffer0[location+3],2);                                      
                               sprintf(AT_CMD,"AT+PORT=%s\r\n",PORT);
                               
                               P4OUT |= BIT0;//进入指令模式
                               delay_ms(100);
                               PutString(AT_CMD);
                               delay_ms(1000);
                               
                          //     Flash_write_OneByte(Modify_Flag,0xcc);
                               PutString("AT+SAVE\r\n");   //5.保存，（以上任何参数发生变化，需要执行 SAVE 并复位后，才能生效）
                               delay_ms(1000);
  
                               
                               while(0==P4IN&BIT2);
                               P4OUT &= ~BIT0;        //进入透传模式    
                               
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
                       if(255!=(location=my_strstr(RxBuffer0,"MFQ")))//请求修改频率或信道 信道：0~80 471500000
                       {
                           memset(AT_CMD,0,80);
                           cmd_len=RxCounter0-3;
                           if(cmd_len>=9)
                           {
                              memcpy(FRE,&RxBuffer0[3],9);//9
   
                               memset(AT_CMD,0,80);
                               sprintf(AT_CMD,"AT+CSQ=%s\r\n",FRE);
                               
                               P4OUT |= BIT0;//进入指令模式
                               delay_ms(100);
                               PutString(AT_CMD);
                               
                           //    Flash_write_OneByte(Modify_Flag,0xcc);
                               PutString("AT+SAVE\r\n");   //5.保存，（以上任何参数发生变化，需要执行 SAVE 并复位后，才能生效）
                               delay_ms(1000);
  

                               P4OUT &= ~BIT0;        //进入透传模式
                               delay_ms(1000);
                               
                               PutString("MFQOK\r\n");                          
         
                               error_code=run_OK;
                           }
                           
                           else if(strlen(RxBuffer0)>=2)
                           {
                               memcpy(FRE,&RxBuffer0[3],2);//2                       
                               memset(AT_CMD,0,80);
                               sprintf(AT_CMD,"AT+CSQ=%s\r\n",FRE);
                               
                               P4OUT |= BIT0;//进入指令模式
                               delay_ms(100);
                               PutString(AT_CMD);
                               
                          //     Flash_write_OneByte(Modify_Flag,0xcc);
                               PutString("AT+SAVE\r\n");   //5.保存，（以上任何参数发生变化，需要执行 SAVE 并复位后，才能生效）
                               delay_ms(1000);
  

                               P4OUT &= ~BIT0;        //进入透传模式
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
                       if(255!=(location=my_strstr(RxBuffer0,"MTP")))//请求修改发射功率
                       {
                           memset(AT_CMD,0,80);
                           cmd_len=RxCounter0-3;
                           if(cmd_len>=2)
                           {       
                             memcpy(FRE,&RxBuffer0[3],2);
                             sprintf(AT_CMD,"AT+CSQ=%s\r\n",FRE);
                             
                             P4OUT |= BIT0;//进入指令模式
                             delay_ms(100);
                             PutString(AT_CMD);
                             
                             PutString("AT+SAVE\r\n");   //5.保存，（以上任何参数发生变化，需要执行 SAVE 并复位后，才能生效）
                             delay_ms(1000);
   

                             P4OUT &= ~BIT0;        //进入透传模式
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
                       if(255!=(location=my_strstr(RxBuffer0,"MPA")))//其他设置
                       {            
                         memset(AT_CMD,0,80);
                         cmd_len=RxCounter0-3;
                         if(RxCounter0>=6)
                         {
                               memcpy(MPA,&RxBuffer0[3],6);
                               P4OUT |= BIT0;//进入指令模式
                               delay_ms(100);
                               if('0'==MPA[0])//SF
                                 {

                                 }
                               else
                                 {
                                   // strncpy(AT_Temp,MPA,2);
                                  //  sprintf(AT_CMD,"AT+=%s\r\n",AT_Temp);//命令未定
                                   // PutString(AT_CMD);
                                 }

                               switch(MPA[2])//消息类型
                               {
                                   case '0'://unconfirmed
                                       PutString("AT+confirm = 0\r\n");
                                       break;
                                   case '1'://confirmed
                                       PutString("AT+confirm = 1\r\n");
                                       break;
                                   case '2'://无动作
                                       break;
                                   default:
                                       break;
                               }

                               switch(MPA[3])//激活方式
                               {
                                   case '0'://ABP
                                       PutString("AT+OTAA=0\r\n");
                                       break;
                                   case '1'://OTAA
                                       PutString("AT+OTAA=1\r\n");
                                       break;
                                   case '2'://无动作
                                       break;
                                   default:
                                       break;
                               }

                               switch(MPA[4])//ADR
                               {
                                   case '0'://关闭ADR
                                       PutString("AT+ADR=0\r\n");
                                       break;
                                   case '1'://启动ADR
                                       PutString("AT+ADR=1\r\n");
                                       break;
                                   case '2'://无动作
                                       break;
                                   default:
                                       break;
                               }

                               switch(MPA[5])//Debug模式
                               {
                                   case '0'://关闭Debug模式
                                     PutString("Debug=0\r\n");
                                       break;
                                   case '1'://启动Debug模式
                                     PutString("Debug=1\r\n");
                                       break;
                                   case '2'://无动作
                                     PutString("Debug=2\r\n");
                                       break;
                                   default:
                                       break;
                               }
                               
                           //    Flash_write_OneByte(Modify_Flag,0xcc);
                               PutString("AT+SAVE\r\n");   //5.保存，（以上任何参数发生变化，需要执行 SAVE 并复位后，才能生效）
                               delay_ms(1000);
  

                               P4OUT &= ~BIT0;        //进入透传模式
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
                     
                       if(255!=(location=my_strstr(RxBuffer0,"RT1")))//请求例行数据上行间隔
                       { 
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;  
                         //  memset(time_interval,0,strlen(time_interval));    
                        //   Flash_read(FLASH_ADDRESS,5);
                        //   memcpy(time_interval,DataBuffer,5);
                            sprintf(AT_CMD,"T1%s\r\n",time_interval);
                            while(0==P4IN&BIT2);   
                               
                            P4OUT &= ~BIT0;        //进入透传模式
                            delay_ms(1000);
                               
                            PutString(AT_CMD);                             
                            error_code=run_OK;

                           error_code=run_OK;
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"RT2")))//请求电池数据上行间隔
                       {
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                           P4OUT &= ~BIT0;        //进入透传模式
                           delay_ms(1000);
                           PutString("T212345\r\n");//待定           
                           error_code=run_OK;
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"RAE")))//请求APPEUI
                       { 
                          memset(RxBuffer0,0,RxCounter0);
                          RxCounter0=0;                   
                             
                           memset(APPEUI,0,strlen(APPEUI));
                           memset(AT_CMD,0,80);
                           
                           P4OUT |= BIT0;//指令模式
                           delay_ms(100);
                           
                           PutString("AT+APPEUI?\r\n");
                           while(1!=UART0_flag);
                           
                           UART0_flag=0;
                           if(255!=(location=my_strstr(RxBuffer0,"+APPEUI:")))
                           { 
                               memcpy(AT_CMD,&RxBuffer0[location+8],24);
                               unpack_str(APPEUI,AT_CMD);
               
                               sprintf(AT_CMD,"APPEUI%s\r\n",APPEUI);
                               
                               P4OUT &= ~BIT0;//透传模式
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
                       if(255!=(location=my_strstr(RxBuffer0,"RAK")))//请求APPKEY
                       {
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;     
                           memset(AT_CMD,0,80);
                            
                           P4OUT |= BIT0;//指令模式
                           delay_ms(100);
                           PutString("AT+APPKEY?\r\n");
                           while(1!=UART0_flag);
                           
                           UART0_flag=0;
                           
                           if(255!=(location=my_strstr(RxBuffer0,"+APPKEY:")))
                           {

                               memcpy(AT_CMD,&RxBuffer0[location+8],48);        
                               unpack_str(APPKEY,AT_CMD);

                               sprintf(AT_CMD,"APPKEY%s\r\n",APPKEY);
                               P4OUT &= ~BIT0;//透传模式
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
                       if(255!=(location=my_strstr(RxBuffer0,"RPT")))//请求现行使用port
                       {
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                           
                           P4OUT |= BIT0;//指令模式  
                           delay_ms(100);          
                           PutString("AT+PORT?\r\n");
                           
                           while(1!=UART0_flag);   
                           UART0_flag=0;
                           if(255!=(location=my_strstr(RxBuffer0,"+PORT:")))
                           {
                               P4OUT &= ~BIT0;//透传模式
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
                       if(255!=(location=my_strstr(RxBuffer0,"RFQ")))//请求现行频率
                       {
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;

                           P4OUT |= BIT0;//指令模式
                           delay_ms(100);
                           PutString("AT+CSQ?\r\n");
                           
                           while(1!=UART0_flag);
                           UART0_flag=0;
                           if(255!=(location=my_strstr(RxBuffer0,"+CSQ: [")))
                           {
                               memcpy(FRE,&RxBuffer0[location+7],9);
                               sprintf(AT_CMD,"FREQ%s\r\n",FRE);

                               P4OUT &= ~BIT0;//透传模式
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
                       if(255!=(location=my_strstr(RxBuffer0,"RTP")))//请求现行发射功率
                       {
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0; 
                           P4OUT &= ~BIT0;        //进入透传模式
                           delay_ms(1000);
                           PutString("reserved\r\n");//待定
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"RSF")))//请求现行扩频因子
                       {
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                           P4OUT &= ~BIT0;        //进入透传模式
                           delay_ms(1000);
                           PutString("reserved\r\n");//待定
                       }
                       if(255!=(location=my_strstr(RxBuffer0,"RCF")))//请求消息包类型
                       {
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                           P4OUT |= BIT0;//指令模式 
                           delay_ms(100);       

                           PutString("AT+CONFIRM?\r\n");
                           while(1!=UART0_flag);       
                           UART0_flag=0;        
                            if(255!=(location=my_strstr(RxBuffer0,"+CONFIRM: ")))
                            {      
                               sprintf(AT_CMD,"CONFSTAT%c\r\n",RxBuffer0[location+10]);
                               P4OUT &= ~BIT0;//透传模式
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
                       if(255!=(location=my_strstr(RxBuffer0,"ROT")))//请求现行激活方式
                       {
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                           P4OUT |= BIT0;//指令模式
                           delay_ms(100);
                           PutString("AT+OTAA?\r\n");
                           while(1!=UART0_flag);
                          
                           UART0_flag=0;
                           if(255!=(location=my_strstr(RxBuffer0,"+OTAA: ")))
                           {              
                               sprintf(AT_CMD,"OTAA%c\r\n",RxBuffer0[location+7]);
                               P4OUT &= ~BIT0;//透传模式
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
                       if(255!=(location=my_strstr(RxBuffer0,"RAD")))//请求现行ADR状态
                       {
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                           
                           P4OUT |= BIT0;//指令模式   
                           delay_ms(100);        
                           PutString("AT+ADR?\r\n");
                           while(1!=UART0_flag);
                           
                           UART0_flag=0;             
                           if(255!=(location=my_strstr(RxBuffer0,"+ADR: ")))
                           {
                               sprintf(AT_CMD,"ADR%c\r\n",RxBuffer0[location+6]);
                               P4OUT &= ~BIT0;//透传模式
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
                       if(255!=(location=my_strstr(RxBuffer0,"RDB")))//请求现行是否debug模式
                       {
                           memset(RxBuffer0,0,RxCounter0);
                           RxCounter0=0;
                           P4OUT &= ~BIT0;        //进入透传模式
                           delay_ms(1000);
                           PutString("reserved\r\n");//待定
                           error_code=run_OK;
                       }
                       break;
                    default:
                       break;
                   }     
               }
             RxCounter0=0;//接收清零
             memset(RxBuffer0,0,strlen(RxBuffer0)); 
            state=handle;
           break;

         case handle:  
              
         P4OUT &= ~BIT0;        //进入透传模式 
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
             P4OUT &= ~BIT1;//模块睡眠
             Timer_A_Flag=0;
             TimerOver_flag=0;
             
             LPM2;  //进入LPM2模式  
             
             P4OUT |= BIT1;//唤醒模块 
             while(0==P4IN&BIT2);
             
             PutString("\r\n");            
             state=read_sensor;
          break;
        default:
           break;
         }//end switch(state)
      }//end while()
}//end main

