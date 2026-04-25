#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Timer.h"
#include "Key.h"
#include "AD.h"
#include "NRF24L01.h"

uint8_t KeyNum;

uint16_t AD_LH, AD_LV, AD_RH, AD_RV;
int8_t LH, LV, RH, RV;
uint8_t KEY;

uint8_t Flag;

uint8_t SendFlag;
uint8_t SuccessRatio;

int8_t DataProcess(uint16_t ADValue);
uint8_t CalculateSuccessRatio(uint8_t SendFlag);

int main(void)
{
	OLED_Init();
	NRF24L01_Init();
	Key_Init();
	AD_Init();
	
	Timer_Init();
	
	
	while (1)
	{
		KeyNum = Key_GetNum();
		if (KeyNum)
		{
			KEY = KeyNum;
		}
		
		AD_LH = AD_GetValue(ADC_Channel_0);
		AD_LV = AD_GetValue(ADC_Channel_1);
		AD_RH = AD_GetValue(ADC_Channel_2);
		AD_RV = AD_GetValue(ADC_Channel_3);
		
		LH = DataProcess(AD_LH);
		LV = DataProcess(AD_LV);
		RH = DataProcess(AD_RH);
		RV = DataProcess(AD_RV);
		
		if (Flag == 1)
		{
			NRF24L01_TxPacket[0] = 0x00;
			NRF24L01_TxPacket[1] = LH;
			NRF24L01_TxPacket[2] = LV;
			NRF24L01_TxPacket[3] = RH;
			NRF24L01_TxPacket[4] = RV;
			NRF24L01_TxPacket[5] = KEY;
			
			SendFlag = NRF24L01_Send();
			
			SuccessRatio = CalculateSuccessRatio(SendFlag);
			
			if (SendFlag == 1)
			{
				KEY = 0;
			}
			
			Flag = 0;
		}
		
		OLED_Printf(0, 0, OLED_8X16, "Sig:%02d", SuccessRatio);
		OLED_Printf(0, 16, OLED_8X16, "LH:%+04d", LH);
		OLED_Printf(0, 32, OLED_8X16, "LV:%+04d", LV);
		OLED_Printf(64, 16, OLED_8X16, "RH:%+04d", RH);
		OLED_Printf(64, 32, OLED_8X16, "RV:%+04d", RV);
		OLED_Update();
	}
}

int8_t DataProcess(uint16_t ADValue)
{
	int16_t Value;
	
	Value = ADValue - 2048;
	
	//-2048 ~ 2047
	
	if (Value > 100)
	{
		Value -= 100;
	}
	else if (Value < -100)
	{
		Value += 100;
	}
	else
	{
		Value = 0;
	}
	
	//-1948 ~ 1947
	
	Value = Value * 101 / 1949;
	
	//-100 ~ 100
	
	return Value;
}

uint8_t CalculateSuccessRatio(uint8_t SendFlag)
{
	static uint8_t SendFlagArray[10];
	static uint8_t p;
	uint8_t i, SuccessCount;
	
	SendFlagArray[p] = SendFlag;
	p ++;
	p %= 10;
	
	SuccessCount = 0;
	for (i = 0; i < 10; i ++)
	{
		if (SendFlagArray[i] == 1)
		{
			SuccessCount ++;
		}
	}
	
	return SuccessCount;
}

void TIM1_UP_IRQHandler(void)
{
	static uint16_t Count;
	
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		Key_Tick();
		
		Count ++;
		if (Count >= 100)
		{
			Count = 0;
			
			Flag = 1;
		}
		
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}
