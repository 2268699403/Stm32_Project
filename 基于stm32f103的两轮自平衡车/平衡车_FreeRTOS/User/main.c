/*
 * main.c
 * FreeRTOS 移植——平衡车主入口
 *
 * 全动态内存分配（configSUPPORT_STATIC_ALLOCATION = 0），
 * 空闲/定时器任务由内核自动从 heap_4 分配。
 *
 * === 任务架构（按优先级从高到低）===
 *  优先级  任务名          周期      栈(word)   职责
 *  ──────────────────────────────────────────────────
 *   4     Task_Balance    10ms       256       传感器+互补滤波+直立环PID
 *   3     Task_Speed      50ms       256       编码器+速度/转向环PID
 *   2     Task_Key        20ms       128       按键扫描+切换 En
 *   1     Task_Comm       10ms       256       NRF遥控+100ms OLED显示
 * ──────────────────────────────────────────────────
 *   LED 由 Task_Balance 控制：ON=运行, OFF=停止。
 *
 *   并发策略：不加锁 + 单写者原则。
 */

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "LED.h"
#include "OLED.h"
#include "MPU6050.h"
#include "Encoder.h"
#include "Motor.h"
#include "PWM.h"
#include "USART2.h"
#include "Key.h"
#include "NRF24L01.h"
#include "PID.h"


/*
 * 全局运行标志
 * En = 0：停止（电机停转，PID 清零）
 * En = 1：运行（控制环正常输出）
 *
 * 写者：Task_Key(短按切换) / Task_Balance(越界强置0)
 * 单字节原子赋值，无 RMW，竞争安全。
 */
uint8_t En = 0;


/*
 * 板级初始化——vTaskStartScheduler() 前单线程完成
 */
void BSP_Init(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

	/* 传感器 */
	MPU6050_Init();
	MPU6050_Config();

	/* 电机与编码器 */
	Encoder_Init();
	Motor_Init();
	PWM_Init();

	/* 通信 */
	NRF24L01_Init();
	/* USART2（蓝牙调参）暂不启用 */

	/* 人机交互 */
	Key_Init();
	LED_Init();
	OLED_Init();

	/* PID——必须最后（RAM 上电随机值需清零）*/
	PID_Init(&PID_Angle);
	PID_Init(&PID_Speed);
	PID_Init(&PID_Turn);
}


/*=============================================================================
 * 任务函数
 *============================================================================*/

/*
 * 直立环任务（10ms）
 *
 * 每周期：I2C 读 MPU6050 → 互补滤波 → Angle
 *         越界(±50°) → En=0 + Motor_Stop
 *         En==1 → PID_Angle_Update（电机驱动）
 *         En==0 → Motor_Stop + 清 PID_Angle
 *
 * LED: ON=运行, OFF=停止/越界
 */
void Task_Balance(void *arg)
{
	(void)arg;
	TickType_t LastWake = xTaskGetTickCount();

	while (1)
	{
		vTaskDelayUntil(&LastWake, pdMS_TO_TICKS(10));

		MPU6050_GetRawData(&Data);

		if (Angle <= -50.0f || Angle >= +50.0f)
		{
			En = 0;
			LED_OFF();
			Motor_Stop();
			PID_Angle.ErrorInt = 0;
			PID_Angle.Target   = 0;
		}
		else if (En == 1)
		{
			LED_ON();
			PID_Angle_Update();
		}
		else
		{
			LED_OFF();
			Motor_Stop();
			PID_Angle.ErrorInt = 0;
			PID_Angle.Target   = 0;
		}
	}
}


/*
 * 速度/转向环任务（50ms）
 *
 * 每个周期：Encoder_GetState → RPM_L/R
 *            En==1 → PID_Move_Update（→ DifPWM, PID_Angle.Target）
 *            En==0 → 自清 PID_Speed/Turn
 */
void Task_Speed(void *arg)
{
	(void)arg;
	TickType_t LastWake = xTaskGetTickCount();

	while (1)
	{
		vTaskDelayUntil(&LastWake, pdMS_TO_TICKS(50));

		Encoder_GetState();

		if (En == 1)
		{
			PID_Move_Update();
		}
		else
		{
			PID_Speed.ErrorInt = 0;
			PID_Speed.Target   = 0;
			PID_Turn.ErrorInt  = 0;
			PID_Turn.Target    = 0;
		}
	}
}


/*
 * 按键任务（20ms）
 *
 * Key_Scan → 读电平，消抖（内部累加器）
 * Key_Mode → 判断短按(Key1_Mode=1) / 长按(Key1_Mode=2)
 * 短按 → 切换 En
 */
void Task_Key(void *arg)
{
	(void)arg;
	TickType_t LastWake = xTaskGetTickCount();

	while (1)
	{
		vTaskDelayUntil(&LastWake, pdMS_TO_TICKS(20));

		Key_Scan();
		Key_Mode();

		if (Key1_Mode == 1)
		{
			En = !En;
			Key1_Mode = 0;
		}
	}
}


/*
 * 通信/显示任务（10ms 基准周期）
 *
 * NRF24L01 轮询（每 10ms）：接收遥控器摇杆 → PID_Speed.Target / PID_Turn.Target
 * OLED 刷新（每 100ms）：显示 Angle / En / ST·SO / AO
 */
void Task_Comm(void *arg)
{
	(void)arg;
	TickType_t LastWake = xTaskGetTickCount();
	uint8_t oled_count = 0;

	while (1)
	{
		vTaskDelayUntil(&LastWake, pdMS_TO_TICKS(10));

		/* NRF24L01 遥控接收 */
		if (NRF24L01_Receive() == 1)
		{
			uint8_t ID = NRF24L01_RxPacket[0];
			if (ID == 0x00)
			{
				int8_t LH = NRF24L01_RxPacket[1];
				int8_t RV = NRF24L01_RxPacket[4];
				PID_Speed.Target = LH * 2;
				PID_Turn.Target  = (RV * 10);
			}
		}

		/* OLED 刷新（每 10 周期 = 100ms）*/
		oled_count++;
		if (oled_count < 10) continue;
		oled_count = 0;

		OLED_Clear();

		OLED_ShowString(0, 0, "A:", OLED_8X16);
		OLED_ShowFloatNum(18, 0, Angle, 3, 2, OLED_8X16);

		OLED_ShowString(0, 16, "En:", OLED_8X16);
		OLED_ShowNum(26, 16, En, 1, OLED_8X16);

		OLED_ShowString(0, 32, "ST:", OLED_8X16);
		OLED_ShowFloatNum(26, 32, PID_Speed.Target, 3, 0, OLED_8X16);
		OLED_ShowString(74, 32, "SO:", OLED_8X16);
		OLED_ShowFloatNum(106, 32, PID_Speed.Out, 3, 0, OLED_8X16);

		OLED_ShowString(0, 48, "AO:", OLED_8X16);
		OLED_ShowFloatNum(26, 48, PID_Angle.Out, 3, 0, OLED_8X16);

		OLED_Update();
	}
}


int main(void)
{
	BSP_Init();

	if (xTaskCreate(Task_Balance, "Balance", 256, NULL, 4, NULL) != pdPASS)
		while (1);
	if (xTaskCreate(Task_Speed,   "Speed",   256, NULL, 3, NULL) != pdPASS)
		while (1);
	if (xTaskCreate(Task_Key,     "Key",     128, NULL, 2, NULL) != pdPASS)
		while (1);
	if (xTaskCreate(Task_Comm,    "Comm",    256, NULL, 1, NULL) != pdPASS)
		while (1);

	vTaskStartScheduler();
	while (1);
}
