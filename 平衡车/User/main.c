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

int main(void)
{
	OLED_Init();
	Motor_Init();
	Motor_Speed(Motor_L,20);
	Motor_Speed(Motor_R,20);
	
	while(1)
	{
		Motor_Direction(Motor_L,Motor_FWD);
		Delay_ms(500);
		Motor_Direction(Motor_L,Motor_REV);
		Delay_ms(500);		
		Motor_Direction(Motor_L,Motor_STOP);
		Delay_ms(500);
		
		Motor_Direction(Motor_R,Motor_FWD);
		Delay_ms(500);
		Motor_Direction(Motor_R,Motor_REV);
		Delay_ms(500);		
		Motor_Direction(Motor_R,Motor_STOP);
		Delay_ms(500);
	}
}
