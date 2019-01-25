#include "flash.h"


char  DataBuffer[32];
//д Flash ����
void write_SegA(char * value,char *addr, char len)
{

  unsigned char i;

  FCTL1 = FWKEY + ERASE;      //�������
  FCTL3 = FWKEY;              //����
  *addr=0;

  FCTL1 = FWKEY + WRT;        //����д����


  for(i=1;i<=len;i++)
  {
    //*Flash_ptr = value;     //������д�뵽Flash��
    //*Flash_ptr++;
    *addr++=*value++;
    _NOP();
  }

  FCTL1 = FWKEY;              //��� WRT λ
  FCTL3 = FWKEY + LOCK;       //��������
}

//��flash
void Flash_read(char *addr, char len)//char
{//��ȡ�����飬�˴���ջӦ�Ĵ�
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
   FCTL1 = FWKEY + ERASE;      //�������
  FCTL3 = FWKEY;              //����
 // *Flash_ptr = 0;             //��д����������
  *addr=0;

  FCTL1 = FWKEY + WRT;        //����д����
  *addr=value;


  FCTL1 = FWKEY;              //��� WRT λ
  FCTL3 = FWKEY + LOCK;       //��������
}