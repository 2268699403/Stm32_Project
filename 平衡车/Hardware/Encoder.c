#include "stm32f10x.h" 

/*初始化转速变量*/
float RPM_L = 0;
float RPM_R = 0;

/**
  *函    数：初始化编码器接口PA6、PA7、PB6、PB7和TIM3、TIM4编码模式配置
  *参    数：无
  *返 回 值：无
  */
void Encoder_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 65536 - 1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 1 - 1;

	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStructure);

	TIM_EncoderInterfaceConfig(TIM3,TIM_EncoderMode_TI12,TIM_ICPolarity_Rising,TIM_ICPolarity_Falling);
	TIM_EncoderInterfaceConfig(TIM4,TIM_EncoderMode_TI12,TIM_ICPolarity_Rising,TIM_ICPolarity_Rising);
		
	TIM_Cmd(TIM3,ENABLE);
	TIM_Cmd(TIM4,ENABLE);

}


/**
  *函    数：转速(转/分钟)计算
  *参    数：无
  *返 回 值：无
  */
void Encoder_GetState(void)
{
	int16_t Count_L = TIM_GetCounter(TIM4);
	int16_t Count_R = TIM_GetCounter(TIM3);
	
	/*编码电机旋转一圈触发44次脉冲，电机减速比9.6*/
	RPM_L = (Count_L * 1200.0f) / (44.0f * 9.6f);
	RPM_R = (Count_R * 1200.0f) / (44.0f * 9.6f);
	
	TIM_SetCounter(TIM3, 0);
	TIM_SetCounter(TIM4, 0);	
}
