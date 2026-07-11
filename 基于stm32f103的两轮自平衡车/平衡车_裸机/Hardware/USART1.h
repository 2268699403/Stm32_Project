#ifndef __USART1_H
#define __USART1_H

#define BaudRate 9600

void USART1_Init(void);
void USART1_SendByte(uint8_t Data);
void USART1_SendString(char *String);
void USART1_Printf(char *format, ...);

#endif
