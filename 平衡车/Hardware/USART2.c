#include "stm32f10x.h"    
#include "stdio.h"
#include "stdarg.h"
#include "USART2.h"


/**
  * 函    数：UART2串口初始化
  * 参    数：无
  * 返 回 值：无
  * 功能说明：初始化UART2串口作为蓝牙传输接口，配置为8位数据位，1位停止位，无校验位
  *           使用PA2作为TX引脚，PA3作为RX引脚
  */
void USART2_Init(void)
{
	/* 使能GPIOA和USART2的时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	/* 配置GPIO引脚 */
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* 2.1 配置PA2为UART2_TX引脚，复用推挽输出模式 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	/* 配置PA3为UART2_RX引脚，浮空输入模式 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;

	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	/* 配置UART2串口参数 */
	USART_InitTypeDef USART_InitStructure;
	
	USART_InitStructure.USART_BaudRate = BaudRate;										// 波特率
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		// 无硬件流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;						// 收发模式
	USART_InitStructure.USART_Parity = USART_Parity_No;									// 无奇偶校验
	USART_InitStructure.USART_StopBits = USART_StopBits_1;								// 1位停止位
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;							// 8位数据位
	
	/* 应用配置到USART2 */
	USART_Init(USART2,&USART_InitStructure);
	/* 使能UART2 */
	USART_Cmd(USART2,ENABLE);
	/* 清除发送完成标志位，避免误触发 */
	USART_ClearFlag(USART2,USART_FLAG_TC);
}

/**
  * 函    数：UART2发送一个字节
  * 参    数：Data 要发送的字节数据
  * 返 回 值：无
  * 功能说明：向UART2发送一个字节，阻塞等待直到发送完成
  */
void USART2_SendByte(uint8_t Data)
{
	/* 将要发送的数据写入发送数据寄存器 */
	USART_SendData(USART2,Data);
	/* 等待发送缓冲区为空（表示数据已转移到移位寄存器） */
	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
}	


/**
  * 函    数：UART2发送字符串
  * 参    数：String 要发送的字符串
  * 返 回 值：无
  * 功能说明：通过UART2发送一个以'\0'结尾的字符串
  */
void USART2_SendString(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)	//遍历字符数组（字符串），遇到字符串结束标志位后停止
	{
		USART2_SendByte(String[i]);			//依次调用Serial_SendByte发送每个字节数据
	}
}


/**
  * 函    数：自定义格式化输出函数
  * 参    数：format 格式化字符串
  * 参    数：... 可变数量的参数
  * 返 回 值：无
  * 功能说明：将格式化后的字符串通过UART2发送，类似printf功能
  *           使用可变参数实现，支持多种格式符
  */
void USART2_Printf(char *format, ...)
{
	char String[128];									//定义字符数组
	va_list arg;										//定义可变参数列表数据类型的变量arg
	va_start(arg, format);								//从format开始，接收参数列表到arg变量
	vsnprintf(String,sizeof(String),format, arg);		//使用vsnprintf打印格式化字符串和参数列表到字符数组中
	va_end(arg);										//结束变量arg
	USART2_SendString(String);							//串口发送字符数组（字符串）
}

