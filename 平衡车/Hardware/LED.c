#include "stm32f10x.h" 
#include "LED.h"

/**
  *函    数：板载LED(PC13)引脚初始化
  *参    数：无
  *返 回 值：无
  */
void LED_Init(void)												
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);		
	
	GPIO_InitTypeDef GPIO_InitStructure;					
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;			
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			
	GPIO_Init(GPIOC,&GPIO_InitStructure);						
	
	GPIO_SetBits(GPIOC,GPIO_Pin_13);
}


/**
  *函    数：打开板载LED
  *参    数：无
  *返 回 值：无
  */
void LED_ON(void)
{
	GPIO_ResetBits(GPIOC,GPIO_Pin_13);
}


/**
  *函    数：关闭板载LED
  *参    数：无
  *返 回 值：无
  */
void LED_OFF(void)
{
	GPIO_SetBits(GPIOC,GPIO_Pin_13);
}
