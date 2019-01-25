#include "flash.h"


char  DataBuffer[32];
//写 Flash 函数
void write_SegA(char * value,char *addr, char len)
{

  unsigned char i;

  FCTL1 = FWKEY + ERASE;      //允许擦除
  FCTL3 = FWKEY;              //解锁
  *addr=0;

  FCTL1 = FWKEY + WRT;        //允许写数据


  for(i=1;i<=len;i++)
  {
    //*Flash_ptr = value;     //把数据写入到Flash中
    //*Flash_ptr++;
    *addr++=*value++;
    _NOP();
  }

  FCTL1 = FWKEY;              //清除 WRT 位
  FCTL3 = FWKEY + LOCK;       //锁定数据
}

//读flash
void Flash_read(char *addr, char len)//char
{//读取到数组，此处堆栈应改大
  unsigned int i;
  for(i=0;i<len;i++)
  {
    DataBuffer[i]=*(addr+i);     //(uint*)
    _NOP();
  }
}

 char Flash_read_OneByte(char *addr)
{
  return *addr;
}
void Flash_write_OneByte(char *addr,char value)
{
   FCTL1 = FWKEY + ERASE;      //允许擦除
  FCTL3 = FWKEY;              //解锁
 // *Flash_ptr = 0;             //空写，启动擦除
  *addr=0;

  FCTL1 = FWKEY + WRT;        //允许写数据
  *addr=value;


  FCTL1 = FWKEY;              //清除 WRT 位
  FCTL3 = FWKEY + LOCK;       //锁定数据
}