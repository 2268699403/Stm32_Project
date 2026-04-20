#ifndef __MOTOR_H
#define __MOTOR_H

#define Motor_L 1			//左电机
#define Motor_R 2			//右电机

void Motor_Init(void);
void Motor_Stop(void);
void Motor_Direction(uint8_t Motor_Id,int16_t Duty);
void Motor_Speed(uint8_t Motor_Id,uint8_t Duty);

#endif
