#ifndef __MPU6050_H
#define __MPU6050_H
#include "MPU6050_Reg.h"

extern MPU6050_Data Data;
extern float AngleAcc;
extern float AngleGyro;
extern float Angle;
void MPU6050_Init(void);
uint8_t MPU6050_Config(void);
uint8_t MPU6050_GetRawData(MPU6050_Data *pData);
void I2C_WriteByte(uint8_t DevAddr,uint8_t RegAddr,uint8_t Data);
uint8_t I2C_ReadByte(uint8_t DevAddr,uint8_t RegAddr);


#endif
