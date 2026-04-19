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
	Key_Init();
	
	while(1)
	{
		Key_Mode();
		OLED_ShowNum( 0,0,Key1_Mode,1,OLED_8X16);
		OLED_ShowNum(10,0,Key2_Mode,1,OLED_8X16);
		OLED_ShowNum(20,0,Key3_Mode,1,OLED_8X16);
		OLED_ShowNum(30,0,Key4_Mode,1,OLED_8X16);
		OLED_Update();
	}
}
