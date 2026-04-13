#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "Timer.h"
#include "USART1.h"
#include "USART2.h"
#include "Key.h"
#include "MPU6050.h"
#include "MPU6050_Reg.h"

int main(void)
{
	OLED_Init();
	MPU6050_Init();
	MPU6050_Config();
	MPU6050_Data Data;
	while(1)
	{
		MPU6050_GetRawData(&Data);
		OLED_ShowSignedNum(0,0,Data.Gyro_X,8,OLED_8X16);
		OLED_ShowSignedNum(0,16,Data.Gyro_Y,8,OLED_8X16);
		OLED_ShowSignedNum(0,32,Data.Gyro_Z,8,OLED_8X16);
		
		OLED_Update();		
	}
}
