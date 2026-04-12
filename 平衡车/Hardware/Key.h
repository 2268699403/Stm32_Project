#ifndef __KEY_H
#define __KEY_H

#define Key_1 GPIO_Pin_1
#define Key_2 GPIO_Pin_0
#define Key_3 GPIO_Pin_5
#define Key_4 GPIO_Pin_4

extern uint8_t Key1_Mode;
extern uint8_t Key2_Mode;
extern uint8_t Key3_Mode;
extern uint8_t Key4_Mode;

void Key_Init(void);
void Key_Scan(void);
void Key_Mode(void);


#endif
