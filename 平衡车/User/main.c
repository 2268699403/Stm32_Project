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
#include "Motor.h"
#include "PWM.h"
#include "Encoder.h"

int main(void)
{
	OLED_Init();
	Timer_Init();
	USART1_Init();
	MPU6050_Init();
	MPU6050_Config();
	while(1)
	{			
		OLED_ShowSignedNum(0, 0,Angle,5,OLED_8X16);
		OLED_ShowSignedNum(0,17,AngleAcc,5,OLED_8X16);
		OLED_ShowSignedNum(0,33,AngleGyro,5,OLED_8X16);
		OLED_ShowSignedNum(0, 49,Data.Gyro_Y,5,OLED_8X16);
				
		OLED_Update();
	}
}
