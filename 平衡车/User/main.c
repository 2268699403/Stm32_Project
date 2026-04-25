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
#include "NRF24L01.h"

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
	NRF24L01_Init();
	
	PID_Init(&PID_Angle);
	PID_Init(&PID_Speed);	
	PID_Init(&PID_Turn);	
	Key1_Mode = 0;
	
	while(1)
	{
		if(Key1_Mode == 1)
		{
			En = !En;
			Key1_Mode = 0;
		}
				
		/* 蓝牙串口数据解析 */
		USART2_ParseParam("Kp", &PID_Turn.Kp);
		USART2_ParseParam("Ki", &PID_Turn.Ki);
		USART2_ParseParam("Kd", &PID_Turn.Kd);	
		USART2_ParseParam("speed",&PID_Speed.Target);
		USART2_ParseParam("Turn",&PID_Turn.Target);
		
		
		if (NRF24L01_Receive() == 1)
		{
			uint8_t ID = NRF24L01_RxPacket[0];
			
			if (ID == 0x00)
			{
				int8_t LH = NRF24L01_RxPacket[1];
//				int8_t LV = NRF24L01_RxPacket[2];
//				int8_t RH = NRF24L01_RxPacket[3];
				int8_t RV = NRF24L01_RxPacket[4];
				
				PID_Speed.Target = LH;
				PID_Turn.Target  = -(RV * 10);	
			}			
		}

		
		
//		
		OLED_ShowFloatNum(0,16,PID_Turn.Kp,1,3,OLED_8X16);
		OLED_ShowFloatNum(0,32,PID_Turn.Ki,1,3,OLED_8X16);
		OLED_ShowFloatNum(0,48,PID_Turn.Kd,1,3,OLED_8X16);
		
		OLED_ShowSignedNum( 0,0,PID_Speed.Target,5,OLED_8X16);
		OLED_ShowSignedNum(60,0,PID_Turn.Target,5,OLED_8X16);
//		OLED_ShowSignedNum(70,32,PID_Turn.Out,5,OLED_8X16);		
		USART2_Printf("%.f,%.f,%.f\r\n",PID_Speed.Target,PID_Speed.Actual,PID_Speed.Out);
		OLED_Update();
	}
}
