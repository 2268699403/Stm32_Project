#include "stm32f10x.h"       
#include "Timer.h"
#include "Key.h"
#include "Encoder.h"
#include "MPU6050.h"
#include "USART1.h"
#include "PID.h"
#include "Motor.h"
#include "LED.h"

uint8_t En = 0;

/**
  * 函    数：TIM1定时器初始化
  * 参    数：无
  * 返 回 值：无
  * 功能说明：配置TIM1为1ms定时中断
  *           系统时钟72MHz，预分频72-1，计数时钟1MHz
  *           自动重装载值1000-1，每1ms产生一次中断
  */
void Timer_Init()
{
	/* 使能TIM1时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
	/* 选择内部时钟作为TIM1的时钟源 */
	TIM_InternalClockConfig(TIM1);
	/* 配置定时器基本参数 */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;			// 时钟分频，不分频
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;		// 向上计数模式
	TIM_TimeBaseInitStructure.TIM_Period = 1000 - 1;					// 自动重装载值，计1000个数
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;					// 预分频系数，72MHz/72=1MHz
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;				// 重复计数器，高级定时器特有
	
	/* 应用配置 */
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStructure);
	/* 使能定时器更新中断 */
	TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);
	/* 清除可能存在的更新中断标志 */
	TIM_ClearITPendingBit(TIM1,TIM_IT_Update);
	/* 配置NVIC中断控制器 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;					// TIM1更新中断通道
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;						// 使能中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;			// 抢占优先级2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;					// 子优先级1
	
	/* 应用NVIC配置 */
	NVIC_Init(&NVIC_InitStructure);
	TIM_ClearFlag(TIM1, TIM_FLAG_Update);
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
	/* 使能定时器 */
	TIM_Cmd(TIM1,ENABLE);
	
}

/**
  * 函    数：TIM1更新中断服务函数
  * 参    数：无
  * 返 回 值：无
  * 功能说明：TIM1计数器溢出时自动调用，清除中断标志并累加计数器
  *           此函数名必须为TIM1_UP_IRQHandler，这是STM32标准库的约定
  */
void TIM1_UP_IRQHandler(void)
{	
	/* 检查是否为更新中断（计数器溢出） */
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
    {
		static int count_1 = 0;
		static int count_2 = 0;
		static int count_3 = 0;
		static uint8_t mpu_init_count = 0;  // MPU6050初始化计数器
		count_1++;
		count_2++;
		count_3++;
				
		if(count_1 >=10)
		{	
			/* MPU6050启动稳定等待 - 前500ms忽略数据 */
			if(mpu_init_count < 50)
			{
				mpu_init_count++;
				MPU6050_GetRawData(&Data);  // 读取但忽略数据
				Angle = 0;  // 强制保持角度为0
				AngleAcc = 0;
				AngleGyro = 0;
				count_1 = 0;
				TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
				return;  // 跳过本次处理
			}
			
			/* 读取MPU6050数据 */
			MPU6050_GetRawData(&Data);
			
			/* 角度保护 - 超过±50°立即禁用 */
			if(Angle <= -50 || Angle >= +50)
			{
				En = 0;  // 强制禁用运行
				LED_OFF();
				Motor_Stop();
				/* 清零PID积分项，防止残留值影响下次启动 */
				PID_Angle.ErrorInt = 0;
				PID_Speed.ErrorInt = 0;
				PID_Angle.Target = 0;  // 同时清零目标值
			}
			else if(En == 1)
			{
				LED_ON();
				PID_Angle_Update();  // 直立环PID调控
			}
			else
			{
				LED_OFF();
			}
			count_1 = 0;
		}
					
		if(count_2 >=20)
		{
			Key_Scan();						//读取按键状态
			Key_Mode();						//判断触发模式
			count_2 = 0;
		}
		
		if(count_3 >=50)
		{
			Encoder_GetState();				//读取编码电机转速
	
			if(En == 1)
			{PID_Speed_Update();}				//速度环PID调控
			
			count_3 = 0;
		}
							
        /* 清除中断标志 */
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);	        
    }
}

