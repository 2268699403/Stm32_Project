#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "Timer.h"

extern volatile int num;

int main(void)
{
	OLED_Init();
	Timer_Init();

	while(1)
	{
		OLED_ShowNum(0,0,num,5,OLED_8X16);
		OLED_Update();

	}
}
