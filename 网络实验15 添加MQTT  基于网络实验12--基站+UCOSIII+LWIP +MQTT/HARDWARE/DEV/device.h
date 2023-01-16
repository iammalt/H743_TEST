#ifndef __DEVICE_H__
#define __DEVICE_H__
#include "sys.h"
/************************ DHT11 数据类型定义******************************/
typedef struct
{
  uint8_t  humi_high8bit;                //原始数据：湿度高8位
  uint8_t  humi_low8bit;                 //原始数据：湿度低8位
  uint8_t  temp_high8bit;                 //原始数据：温度高8位
  uint8_t  temp_low8bit;                 //原始数据：温度高8位
  uint8_t  check_sum;                     //校验和
  double    humidity;        //实际湿度
  double    temperature;     //实际温度  
} Device_Data_TypeDef;

#endif 

