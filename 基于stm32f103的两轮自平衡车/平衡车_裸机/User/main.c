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

/**
  * 函    数：主函数，系统初始化及主循环
  * 参    数：无
  * 返 回 值：无
  * 功能说明：完成各模块初始化后，在主循环中处理按键控制、蓝牙参数解析、
  *           遥控器数据接收以及OLED数据显示
  */
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
	
	int8_t x = 0;
	
	while(1)
	{
		/* 按键1短按切换系统启停状态 */
		if(Key1_Mode == 1)
		{
			En = !En;
			Key1_Mode = 0;
		}
		
		OLED_ShowSignedNum(0,0,x,3,OLED_8X16);		
		
				
		/* 蓝牙串口数据解析：通过蓝牙在线调节PID参数及速度/转向目标值 */
//		USART2_ParseParam("Kp", &PID_Angle.Kp);
//		USART2_ParseParam("Ki", &PID_Angle.Ki);
//		USART2_ParseParam("Kd", &PID_Angle.Kd);	
//		USART2_ParseParam("speed",&PID_Speed.Target);
//		USART2_ParseParam("Turn",&PID_Turn.Target);
		
		/* 无线遥控器数据接收：解析数据包，设置速度目标和转向目标 */
		if (NRF24L01_Receive() == 1)
		{
			uint8_t ID = NRF24L01_RxPacket[0];
			
			if (ID == 0x00)
			{
				int8_t LH = NRF24L01_RxPacket[1];
//				int8_t LV = NRF24L01_RxPacket[2];		// 垂直摇杆（预留）
//				int8_t RH = NRF24L01_RxPacket[3];		// 右水平摇杆（预留）
				int8_t RV = NRF24L01_RxPacket[4];
				
				PID_Speed.Target = LH * 2;		// 左摇杆垂直控制速度
				PID_Turn.Target  = (RV * 3);	// 右摇杆水平控制转向
			}			
		}

		OLED_ShowFloatNum(0,0,PID_Angle.Actual,3,2,OLED_8X16);
		OLED_ShowFloatNum(0,16,PID_Speed.Actual,3,2,OLED_8X16);
//		
//		OLED_ShowSignedNum( 0,0,PID_Angle.Actual,5,OLED_8X16);
//		OLED_ShowSignedNum(60,0,PID_Speed.Actual,5,OLED_8X16);
//		USART2_Printf("%.f,%.f,%.f\r\n",PID_Turn.Target,PID_Turn.Actual,PID_Turn.Out);
		OLED_Update();
	}
}
