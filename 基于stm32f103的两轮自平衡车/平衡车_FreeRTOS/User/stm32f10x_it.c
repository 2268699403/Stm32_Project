/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "stm32f10x.h"
#include "USART2.h"

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
/* SVC_Handler 已由 FreeRTOS port.c 提供，此处注释掉以避免重复定义 */
//void SVC_Handler(void)
//{
//}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
/* PendSV_Handler 已由 FreeRTOS port.c 提供，此处注释掉以避免重复定义 */
//void PendSV_Handler(void)
//{
//}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
/* SysTick_Handler 已由 FreeRTOS port.c 提供，此处注释掉以避免重复定义 */
//void SysTick_Handler(void)
//{
//}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/*
 * USART2 全局中断服务函数——蓝牙接收
 *
 * 触发条件：USART2 收到 1 字节数据（RXNE 标志置位）
 * 优先级：  抢占 1（由 USART2_Init 中 NVIC_Init 配置）
 *
 * FreeRTOS 兼容性说明：
 *   此 ISR 优先级为 1，高于 configMAX_SYSCALL_INTERRUPT_PRIORITY=5，
 *   属于"高优先级中断区"——内核不可屏蔽区。
 *   因此此 ISR 内绝对不能调用任何以 FromISR 结尾的 FreeRTOS API
 *   （如 xQueueSendFromISR、xTaskNotifyFromISR），
 *   否则会破坏内核临界区导致死机。
 *
 *   当前只做"存字节到环形缓冲区"，不调任何 RTOS API，安全。
 *   Task_Comm 任务通过 USART2_ParseParam() 轮询缓冲区解析命令。
 */
void USART2_IRQHandler(void)
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		uint8_t data = USART_ReceiveData(USART2);

		if((USART2_RxWritePointer + 1) % USART2_RX_BUFFER_SIZE != USART2_RxReadPointer)
		{
			USART2_RxBuffer[USART2_RxWritePointer] = data;
			USART2_RxWritePointer++;
			USART2_RxWritePointer %= USART2_RX_BUFFER_SIZE;
		}

		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	}
}

/**
  * @}
  */

/**
  * @brief  This function handles malloc failed hook for FreeRTOS.
  * @param  None
  * @retval None
  */
void vApplicationMallocFailedHook(void)
{
  /* Out of memory: halt here, set breakpoint to locate */
  while (1)
  {
  }
}

/**
  * @brief  This function handles stack overflow hook for FreeRTOS.
  * @param  xTask: task handle
  * @param  pcTaskName: task name
  * @retval None
  */
void vApplicationStackOverflowHook(void *xTask, signed char *pcTaskName)
{
  /* Stack overflow detected: halt here, check pcTaskName to locate */
  (void)xTask;
  (void)pcTaskName;
  while (1)
  {
  }
}


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
