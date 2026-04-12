#ifndef __USART2_H
#define __USART2_H

#define BaudRate 9600

void USART2_Init(void);
void USART2_SendByte(uint8_t Data);
void USART2_SendString(char *String);
void Serial_Printf(char *format, ...);


#endif
