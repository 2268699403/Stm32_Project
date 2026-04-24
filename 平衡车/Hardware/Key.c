#include "stm32f10x.h"
#include "Timer.h"
#include "Key.h"

/*按键模式(1为短按，2为长按)*/
uint8_t Key1_Mode = 0;
uint8_t Key2_Mode = 0;
uint8_t Key3_Mode = 0;
uint8_t Key4_Mode = 0;

/*按键状态(1为松开，0为按下)*/
uint8_t State1 = 1;
uint8_t State2 = 1;
uint8_t State3 = 1;
uint8_t State4 = 1;

/*触发标志(触发置1)*/
uint8_t Pressed1  = 0;
uint8_t Released1 = 0;
uint8_t Pressed2  = 0;
uint8_t Released2 = 0;
uint8_t Pressed3  = 0;
uint8_t Released3 = 0;
uint8_t Pressed4  = 0;
uint8_t Released4 = 0;

/*按下时常(20ms+1)*/
uint8_t count1 = 0;
uint8_t count2 = 0;
uint8_t count3 = 0;
uint8_t count4 = 0;


/**
  *函    数：初始化按键GPIO配置
  *参    数：无
  *返 回 值：无
  */
void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = Key_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
		
	GPIO_InitStructure.GPIO_Pin = Key_2;
	GPIO_Init(GPIOB,&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = Key_3;
	GPIO_Init(GPIOA,&GPIO_InitStructure); 

	GPIO_InitStructure.GPIO_Pin = Key_4;
	GPIO_Init(GPIOA,&GPIO_InitStructure);		
}


/**
  *函    数：按键扫描函数，获取按键状态
  *参    数：无
  *返 回 值：无
  */
void Key_Scan(void)
{
	static uint8_t last_State1 = 1;
	static uint8_t last_State2 = 1;
	static uint8_t last_State3 = 1;
	static uint8_t last_State4 = 1;
	
	/* 显式初始化静态变量，确保启动时为1(松开状态) */
	static uint8_t Key_Scan_initialized = 0;
	if(!Key_Scan_initialized)
	{
		last_State1 = 1;
		last_State2 = 1;
		last_State3 = 1;
		last_State4 = 1;
		Key_Scan_initialized = 1;
	}

	/*获取按键上次状态*/	
	last_State1 = State1;
	last_State2 = State2;
	last_State3 = State3;
	last_State4 = State4;
	
	/*获取按键状态*/
	State1 = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1);
	State2 = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0);
	State3 = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5);
	State4 = GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_4);	
	
	/*按键Key1触发判断*/
	if(State1 == 0 && last_State1 == 1)
	{Pressed1 = 1;}
	else if(State1 == 1 && last_State1 == 0)
	{Released1 = 1;}
	else if(State1 == 0 && last_State1 == 0)
	{count1++;}
		
	/*按键Key2触发判断*/
	if(State2 == 0 && last_State2 == 1)
	{Pressed2 = 1;}
	else if(State2 == 1 && last_State2 == 0)
	{Released2 = 1;}
	else if(State2 == 0 && last_State2 == 0)
	{count2++;}
	
	/*按键Key3触发判断*/
	if(State3 == 0 && last_State3 == 1)
	{Pressed3 = 1;}
	else if(State3 == 1 && last_State3 == 0)
	{Released3 = 1;}
	else if(State3 == 0 && last_State3 == 0)
	{count3++;}
	
	/*按键Key4触发判断*/
	if(State4 == 0 && last_State4 == 1)
	{Pressed4 = 1;}
	else if(State4 == 1 && last_State4 == 0)
	{Released4 = 1;}
	else if(State4 == 0 && last_State4 == 0)
	{count4++;}

}


/**
  *函    数：按键触发模式(长按/短按)判断
  *参    数：无
  *返 回 值：无
  *注    意：调用该函数获取触发模式并处理完事件后，要清除对应Key_Mode
  */
void Key_Mode(void)
{
	/*判断Key1触发模式*/
	if(Pressed1 && !Key1_Mode && count1 >= 50)		//长按判断
	{
		Key1_Mode = 2;
		count1	  = 0;
		Pressed1  = 0;
		Released1 = 0;
	}
	if(Released1 && !Key1_Mode && count1 < 50)		//短按判断
	{
		    Key1_Mode = 1;  
			count1	  = 0;
			Pressed1  = 0;
			Released1 = 0;
	}
	
	/*判断Key2触发模式*/
	if(Pressed2 && !Key2_Mode && count2 >= 50)		//长按判断
	{
		Key2_Mode = 2;
		count2	  = 0;
		Pressed2  = 0;
		Released2 = 0;
	}
	if(Released2 && !Key2_Mode && count2 < 50)		//短按判断
	{
		    Key2_Mode = 1;  
			count2	  = 0;
			Pressed2  = 0;
			Released2 = 0;
	}
	
	/*判断Key3触发模式*/
	if(Pressed3 && !Key3_Mode && count3 >= 50)		//长按判断
	{
		Key3_Mode = 2;
		count3	  = 0;
		Pressed3  = 0;
		Released3 = 0;
	}
	if(Released3 && !Key3_Mode && count3 < 50)		//短按判断
	{
		    Key3_Mode = 1;  
			count3	  = 0;
			Pressed3  = 0;
			Released3 = 0;
	}
	
	/*判断Key4触发模式*/
	if(Pressed4 && !Key4_Mode && count4 >= 50)		//长按判断
	{
		Key4_Mode = 2;
		count4	  = 0;
		Pressed4  = 0;
		Released4 = 0;
	}
	if(Released4 && !Key4_Mode && count4 < 50)		//短按判断
	{
		    Key4_Mode = 1;  
			count4	  = 0;
			Pressed4  = 0;
			Released4 = 0;
	}
	
	
}

