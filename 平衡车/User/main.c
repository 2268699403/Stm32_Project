#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "Timer.h"
#include "USART1.h"
#include "USART2.h"
#include "Key.h"


int main(void)
{
	OLED_Init();
	Timer_Init();
	Key_Init();
	
	while(1)
	{

	}
}
