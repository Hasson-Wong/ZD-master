#ifndef _AM2320_H_
#define _AM2320_H_

#include "config.h"

#define S_Temp "Temp:"
#define S_Rh "Rh:"
#define S_NotS "AT+CMSG=\"Sensor Not Connected\""

void Port_Init(void);
void Clear_Data (void); /*清除上次接收的数据*/
unsigned char Read_SensorData(void);/*功能： 读传感器发送的单个字节*/
unsigned char Read_Sensor(void);/*功能： 读传感器  */

#endif
