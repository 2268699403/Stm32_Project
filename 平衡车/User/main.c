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
	Encoder_Init();
	Motor_Init();
	Timer_Init();
	
	while(1)
	{
		
		Motor_Speed(Motor_L,10);
		Motor_Speed(Motor_R,10);
				
		Motor_Direction(Motor_L,Motor_FWD);
		Motor_Direction(Motor_R,Motor_FWD);
		uint16_t encoder1_count = TIM_GetCounter(TIM3);
		OLED_ShowFloatNum(0,0,RPM_L,5,5,OLED_8X16);
		uint16_t encoder2_count = TIM_GetCounter(TIM4);
		OLED_ShowFloatNum(0,17,RPM_R,5,5,OLED_8X16);
		OLED_Update();
		
//		USART2_Printf("RPM_L: %.2f\r\n",RPM_L);
//		USART2_Printf("RPM_R: %.2f\r\n",RPM_R);
	}
}
