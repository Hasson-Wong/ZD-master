#ifndef _AM2320_H_
#define _AM2320_H_

#include "config.h"

#define S_Temp "Temp:"
#define S_Rh "Rh:"
#define S_NotS "AT+CMSG=\"Sensor Not Connected\""

void Port_Init(void);
void Clear_Data (void); /*����ϴν��յ�����*/
unsigned char Read_SensorData(void);/*���ܣ� �����������͵ĵ����ֽ�*/
unsigned char Read_Sensor(void);/*���ܣ� ��������  */

#endif
