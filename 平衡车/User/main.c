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
#include "Motor.h"
#include "PID.h"

int main(void)
{
	OLED_Init();
	Timer_Init();
	Motor_Init();
	USART2_Init();
	MPU6050_Init();
	MPU6050_Config();	
	Key_Init();
	LED_Init();
	PWM_Init();
	
	while(1)
	{
		
		if(Key2_Mode == 1){PID_Angle.Kp += 0.1;Key2_Mode = 0;}
		else if(Key2_Mode == 2){PID_Angle.Kp -= 0.1;Key2_Mode = 0;}
		
		if(Key3_Mode == 1){PID_Angle.Ki += 0.01;Key3_Mode = 0;}
		else if(Key3_Mode == 2){PID_Angle.Ki -= 0.1;Key3_Mode = 0;}
		
		if(Key4_Mode == 1){PID_Angle.Kd += 0.1;Key4_Mode = 0;}
		else if(Key4_Mode == 2){PID_Angle.Kd -= 0.1;Key4_Mode = 0;}
		
		
		OLED_ShowFloatNum(0,0,Angle,3,3,OLED_8X16);
		
		OLED_ShowFloatNum(0,16,PID_Angle.Kp,2,2,OLED_8X16);
		OLED_ShowFloatNum(0,32,PID_Angle.Ki,2,2,OLED_8X16);
		OLED_ShowFloatNum(0,48,PID_Angle.Kd,2,2,OLED_8X16);
		
		OLED_ShowSignedNum(60,16,PID_Angle.Target,5,OLED_8X16);
		OLED_ShowSignedNum(60,32,PID_Angle.Actual,5,OLED_8X16);
		OLED_ShowSignedNum(60,48,PID_Angle.Out,5,OLED_8X16);		
		USART2_Printf("%.f,%.f,%.f\r\n",PID_Angle.Target,PID_Angle.Actual,PID_Angle.Out);
		OLED_Update();
	}
}
