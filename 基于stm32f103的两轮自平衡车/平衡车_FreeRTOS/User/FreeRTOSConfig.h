/*
    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

*/


#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f10x.h"

//针对不同的编译器选用不同的stdint.h文件
#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
    #include <stdint.h>
    extern uint32_t SystemCoreClock;
#endif

//断言
#define configASSERT(x) if((x)==0) { while(1); }

/************************************************************************
 *               FreeRTOS内核参数配置选择
 *********************************************************************/
/* 1：RTOS使用抢占式调度，0：RTOS使用协作式调度（时间片）
 *
 * 注意在单核处理器上，系统可以分为抢占式和协作式两种。
 * 协作式操作系统中，任务主动释放CPU时切换到另一个任务
 * 任务的切换时间完全取决于正在运行的任务
 */
#define configUSE_PREEMPTION					  1

//1：使用时间片调度（默认是使能的）
#define configUSE_TIME_SLICING					1

/* 某些支持FreeRTOS的硬件提供了一种可选的要执行的任务的
 * 通用方法和特定硬件的方法，以下简称"特殊方法"。
 *
 * 通用方法：
 *      1.configUSE_PORT_OPTIMISED_TASK_SELECTION 为 0 或硬件不支持特殊方法时
 *      2.可用于所有FreeRTOS支持的硬件
 *      3.纯C实现，效率低于特殊方法
 *      4.无限制，需要更多任务优先级选项时
 * 特殊方法：
 *      1.必须将configUSE_PORT_OPTIMISED_TASK_SELECTION设置为1
 *      2.依赖于一个特定架构的汇编指令（一般是计算前导零[CLZ]指令）
 *      3.比通用方法更有效
 *      4.一般强制限制最大优先级数目为32
 * 一般如果你的硬件有前导零指令，就尽量使用的，MCU没有这些硬件指令的话还是乖乖设置为0。
 */
#define configUSE_PORT_OPTIMISED_TASK_SELECTION	        1

/* 1：启用低功耗tickless模式，0：保持系统运行，tick中断一直产生
 * 开启低功耗的模式可能会导致程序下载出现问题，因为芯片进入睡眠后，无法通过以下办法下载
 *
 * 解决方法：
 *      1.将开发板断电再重新上电
 *      2.按住复位按键，点击下载后再松手
 *
 *      1.通过设置跳帽 BOOT 0 接高电平(3.3V)
 *      2.重新上电，下载
 *
 * 			1.使用FlyMcu清除一次芯片后再下载
 *			STMISP -> 清除芯片(z)
 */
#define configUSE_TICKLESS_IDLE													0

/*
 * 写入实际的CPU内核时钟频率，也就是CPU指令执行频率，通常称为Fclk
 * Fclk为供给CPU内核的时钟信号，通俗来说，cpu主频为 XX MHz，
 * 即指该时钟信号，对应的，1/Fclk为cpu时间周期。
 */
#define configCPU_CLOCK_HZ						  (SystemCoreClock)

//RTOS系统节拍中断的频率。即一秒中断的次数，每次中断RTOS进行任务调度
#define configTICK_RATE_HZ						  (( TickType_t )1000)

//可使用的最大优先级
#define configMAX_PRIORITIES					  (32)

//空闲任务使用的堆栈大小
#define configMINIMAL_STACK_SIZE				((unsigned short)128)

//任务名字符串长度
#define configMAX_TASK_NAME_LEN					(16)

 //系统节拍计数器变量类型，1表示为16位无符号整形，0表示为32位无符号整形
#define configUSE_16_BIT_TICKS					0

//空闲任务放弃CPU使用权给同优先级的用户任务
#define configIDLE_SHOULD_YIELD					1

//设置队列
#define configUSE_QUEUE_SETS					  0

//开启任务通知功能，默认开启
#define configUSE_TASK_NOTIFICATIONS    1

//使用互斥信号量
#define configUSE_MUTEXES						    1

//使用递归互斥信号量
#define configUSE_RECURSIVE_MUTEXES			0

//为1时使用计数信号量
#define configUSE_COUNTING_SEMAPHORES		0

/* 设置可以注册的信号量和消息队列个数 */
#define configQUEUE_REGISTRY_SIZE				10

#define configUSE_APPLICATION_TASK_TAG		  0


/*****************************************************************
              FreeRTOS关于内存管理有关选项
*****************************************************************/
//支持动态内存分配
#define configSUPPORT_DYNAMIC_ALLOCATION        1
//支持静态内存（当前未使用，所有任务通过 xTaskCreate 动态创建）
#define configSUPPORT_STATIC_ALLOCATION			0
//系统所能够的堆大小
#define configTOTAL_HEAP_SIZE					((size_t)(8*1024))


/***************************************************************
             FreeRTOS与钩子函数有关的选项
**************************************************************/
/* 1：使用空闲钩子（Idle Hook）或回调函数，0：忽略空闲钩子
 *
 * 空闲任务钩子是一个由用户自己实现的函数，
 * FreeRTOS规定了函数名字和参数：void vApplicationIdleHook(void )，
 * 空闲任务每循环一次都会被调用一次。
 * 如果已经删除了RTOS任务，空闲钩子可以用来释放分配给它们的堆栈内存。
 * 因此必须保证空闲钩子函数可以被CPU执行。
 * 使用空闲钩子函数使CPU进入省电模式是很常见的。
 * 但不允许在钩子函数中调用可能阻塞的API函数。
 */
#define configUSE_IDLE_HOOK						0

/* 1：使用时间片钩子（Tick Hook），0：忽略时间片钩子
 *
 *
 * 时间片钩子是一个由用户自己实现的函数，
 * FreeRTOS规定了函数名字和参数：void vApplicationTickHook(void )
 * 时间片中断可以频率非常高的调用
 * 因此函数必须非常短小，不能大量使用堆栈。
 * 不能调用以"FromISR" 或 "FROM_ISR"结尾的API函数。
 */
 /*xTaskIncrementTick函数是在xPortSysTickHandler中断函数中被调用的。因此，vApplicationTickHook()函数执行的时间必须很短*/
#define configUSE_TICK_HOOK						0

//使用内存分配失败钩子函数
#define configUSE_MALLOC_FAILED_HOOK			1

/*
 * 大于0时启动堆栈溢出检测功能，要使用此功能
 * 用户需要提供一个栈溢出回调函数，如果使用的话
 * 值设置为1或者2，此为两种堆栈溢出检测方法 */
#define configCHECK_FOR_STACK_OVERFLOW			1


/********************************************************************
          FreeRTOS关于运行时和任务状态收集有关的选项
**********************************************************************/
//使能运行时统计功能
#define configGENERATE_RUN_TIME_STATS	        0
 //使能可视化跟踪调试
#define configUSE_TRACE_FACILITY				      0
/* 当configUSE_TRACE_FACILITY同时为1时，可以调用下面3个函数
 * prvWriteNameToBuffer()
 * vTaskList(),
 * vTaskGetRunTimeStats()
*/
#define configUSE_STATS_FORMATTING_FUNCTIONS	1


/********************************************************************
                FreeRTOS与协程有关的选项
*********************************************************************/
//使能协程，即协程可以和任务共存的文件croutine.c
#define configUSE_CO_ROUTINES 			          0
//协程的有效优先级数目
#define configMAX_CO_ROUTINE_PRIORITIES       ( 2 )


/***********************************************************************
                FreeRTOS与软件定时器有关的选项
**********************************************************************/
 //使能软件定时器
#define configUSE_TIMERS				            1
//软件定时器优先级
#define configTIMER_TASK_PRIORITY		        (configMAX_PRIORITIES-1)
//软件定时器队列长度
#define configTIMER_QUEUE_LENGTH		        10
//软件定时器任务堆栈大小
#define configTIMER_TASK_STACK_DEPTH	      (configMINIMAL_STACK_SIZE*2)

/************************************************************
            FreeRTOS可选函数选项
************************************************************/
#define INCLUDE_xTaskGetSchedulerState       1
#define INCLUDE_vTaskPrioritySet		         1
#define INCLUDE_uxTaskPriorityGet		         1
#define INCLUDE_vTaskDelete				           1
#define INCLUDE_vTaskCleanUpResources	       0
#define INCLUDE_vTaskSuspend			           1
#define INCLUDE_vTaskDelayUntil			         1
#define INCLUDE_vTaskDelay				           1
#define INCLUDE_eTaskGetState			           1
#define INCLUDE_xTimerPendFunctionCall	     0
//#define INCLUDE_xTaskGetCurrentTaskHandle       1
//#define INCLUDE_uxTaskGetStackHighWaterMark     0
//#define INCLUDE_xTaskGetIdleTaskHandle          0


/******************************************************************
            FreeRTOS与中断有关的选项
******************************************************************/
#ifdef __NVIC_PRIO_BITS
	#define configPRIO_BITS       		__NVIC_PRIO_BITS
#else
	#define configPRIO_BITS       		4
#endif
//中断优先级最大值
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY			15

//系统可管理的最高中断优先级
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY	5

#define configKERNEL_INTERRUPT_PRIORITY 		( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )	/* 240 */

#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )


/****************************************************************
            FreeRTOS与中断处理函数有关的选项
****************************************************************/
#define xPortPendSVHandler 	PendSV_Handler
#define vPortSVCHandler 	SVC_Handler
#define xPortSysTickHandler SysTick_Handler


/* 以下为使用Percepio Tracealyzer需要的定义，不需要时请将 configUSE_TRACE_FACILITY 设置为 0 */
#if ( configUSE_TRACE_FACILITY == 1 )
#include "trcRecorder.h"
#define INCLUDE_xTaskGetCurrentTaskHandle               1   // 这一行可选，设置此函数可在 Trace源中使用，默认值为0 表示不使能
#endif


#endif /* FREERTOS_CONFIG_H */
