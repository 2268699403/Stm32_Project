#include "stm32f10x.h"  

/**
  * 函    数 初始化PWM配置
  * 参    数 无
  * 返 回 值 无
  */
void PWM_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	TIM_InternalClockConfig(TIM2);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 7200 - 1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 0;
	
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);
	
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	
	TIM_OC1Init(TIM2,&TIM_OCInitStructure);
	TIM_OC2Init(TIM2,&TIM_OCInitStructure); 
		
	TIM_Cmd(TIM2,ENABLE);
	
}

/**
  * 函    数 设置TIM2通道1的PWM占空比
  * 参    数 duty: 占空比 (0~100)
  * 返 回 值 无
  */
void TIM2_SetPWM1_Duty(uint8_t duty)
{
    uint16_t ccr = (TIM2->ARR + 1) * duty / 100;  	// 计算比较寄存器值
    TIM_SetCompare1(TIM2, ccr);            		  	// 设置TIM2_CH1占空比
}	

/**
  * 函    数 设置TIM2通道2的PWM占空比
  * 参    数 duty: 占空比 (0~100)
  * 返 回 值 无
  */
void TIM2_SetPWM2_Duty(uint8_t duty)
{
    uint16_t ccr = (TIM2->ARR + 1) * duty / 100; 	 // 计算比较寄存器值
    TIM_SetCompare2(TIM2, ccr);              		 // 设置TIM2_CH2占空比
}

