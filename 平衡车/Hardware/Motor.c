#include "stm32f10x.h" 
#include "Motor.h"
#include "PWM.h"

/**
  * 函    数：电机驱动初始化
  * 参    数：无
  * 返 回 值：无
  */
void Motor_Init(void)
{
	PWM_Init();
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	TIM2_SetPWM1_Duty(0);
	TIM2_SetPWM2_Duty(0);
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	GPIO_ResetBits(GPIOB,GPIO_Pin_13);
	GPIO_ResetBits(GPIOB,GPIO_Pin_14);
	GPIO_ResetBits(GPIOB,GPIO_Pin_15);
}

/**
  * 函    数：电机刹车
  * 参    数：无
  * 返 回 值：无
  */
void Motor_Stop(void)
{
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
	GPIO_SetBits(GPIOB,GPIO_Pin_13);
	GPIO_SetBits(GPIOB,GPIO_Pin_14);
	GPIO_SetBits(GPIOB,GPIO_Pin_15);		
}


/**
  * 函    数：控制电机旋转
  * 参    数：Motor_Id：1(Motor_L)为左电机，2(Motor_R)为右电机
* 参    数：Duty：PWM占空比，取值为-100~100，Duty > 0正转，Duty < 0反转
  * 返 回 值：无
  * 注    意：正反转是基于运动方向定义的，正转为驱动平衡车前进的旋转方向，反转为驱动平衡车后退的方向。
  *			  实际左电机正转为逆时针旋转，右电机正转为顺时针旋转。
  */
void Motor_Direction(uint8_t Motor_Id,int16_t Duty)
{
	/*判断左电机*/
	 if(Motor_Id == Motor_L)
	{
		if(Duty < 5 && Duty > 1){Duty = 5;}
		else if(Duty < -1 && Duty > -5){Duty = -5;}
		
		if(Duty > 1)			//正转
		{
			TIM2_SetPWM1_Duty(Duty);			
			GPIO_SetBits(GPIOB,GPIO_Pin_14);
			GPIO_ResetBits(GPIOB,GPIO_Pin_15);					
		}
		else if(Duty < -1)		//反转
		{
			TIM2_SetPWM1_Duty(-Duty);
			GPIO_SetBits(GPIOB,GPIO_Pin_15);
			GPIO_ResetBits(GPIOB,GPIO_Pin_14);					
		}
		else					//停转
		{
			GPIO_ResetBits(GPIOB,GPIO_Pin_14);
			GPIO_ResetBits(GPIOB,GPIO_Pin_15);
		}
	}	
	
	/*判断右电机*/
	else if(Motor_Id == Motor_R)
	{	
		if(Duty < 5 && Duty > 1){Duty = 5;}
		else if(Duty < -1 && Duty > -5){Duty = -5;}
		
		if(Duty > 0)			//正转
		{
			TIM2_SetPWM2_Duty(Duty);
			GPIO_SetBits(GPIOB,GPIO_Pin_13);
			GPIO_ResetBits(GPIOB,GPIO_Pin_12);
		}
		else if(Duty < 0)		//反转
		{
			TIM2_SetPWM2_Duty(-Duty);
			GPIO_SetBits(GPIOB,GPIO_Pin_12);
			GPIO_ResetBits(GPIOB,GPIO_Pin_13);
		}
		else					//停转
		{
			GPIO_ResetBits(GPIOB,GPIO_Pin_12);
			GPIO_ResetBits(GPIOB,GPIO_Pin_13);					
		}

	}
	
}
