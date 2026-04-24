#ifndef __USART2_H
#define __USART2_H

#define BaudRate 9600
#define USART2_RX_BUFFER_SIZE 128

void USART2_Init(void);

// 发送相关函数
void USART2_SendByte(uint8_t Data);
void USART2_SendString(char *String);
void USART2_Printf(char *format, ...);

// 接收缓冲区变量声明
extern uint8_t USART2_RxBuffer[USART2_RX_BUFFER_SIZE];
extern uint8_t USART2_RxWritePointer;
extern uint8_t USART2_RxReadPointer;
extern uint8_t USART2_RxFlag;

// 接收相关函数
void USART2_Receive_Init(void);                                    // 初始化接收功能
uint8_t USART2_GetRxFlag(void);                                    // 获取接收完成标志
void USART2_ClearRxFlag(void);                                     // 清除接收完成标志
uint8_t USART2_ParseParam(char *param_name, float *param_value);   // 解析参数并赋值

#endif
