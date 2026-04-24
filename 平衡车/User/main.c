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
	MPU6050_Init();
	MPU6050_Config();	
	Encoder_Init();
	Motor_Init();
	USART2_Init();
	Key_Init();
	LED_Init();
	PWM_Init();
	Timer_Init();
	
	PID_Init(&PID_Angle);
	PID_Init(&PID_Speed);	
		
	Key1_Mode = 0;
	
	while(1)
	{
		if(Key1_Mode == 1)
		{En = 1;Key1_Mode = 0;}
		if(Key2_Mode == 1)
		{PID_Speed.Kp += 0.001;Key2_Mode = 0;}
		if(Key3_Mode == 1)
		{PID_Speed.Ki += 0.0001;Key3_Mode = 0;}
		if(Key4_Mode == 1)
		{PID_Speed.Kd += 0.001;Key4_Mode = 0;}
				
		/* 蓝牙串口数据解析 */
		USART2_ParseParam("Kp", &PID_Speed.Kp);
		USART2_ParseParam("Ki", &PID_Speed.Ki);
		USART2_ParseParam("Kd", &PID_Speed.Kd);	
		USART2_ParseParam("speed",&PID_Speed.Target);
		
		
		OLED_ShowFloatNum(0, 0,PID_Speed.Kp,1,3,OLED_8X16);
		OLED_ShowFloatNum(0,16,PID_Speed.Ki,1,3,OLED_8X16);
		OLED_ShowFloatNum(0,32,PID_Speed.Kd,1,3,OLED_8X16);
		
		OLED_ShowSignedNum(70,0,PID_Speed.Target,5,OLED_8X16);
		OLED_ShowSignedNum(70,16,PID_Speed.Actual,5,OLED_8X16);
		OLED_ShowSignedNum(70,32,PID_Speed.Out,5,OLED_8X16);		
		USART2_Printf("%.f,%.f,%.f\r\n",PID_Speed.Target,PID_Speed.Actual,PID_Speed.Out);
		OLED_Update();
	}
}
