#ifndef __MOTOR_H
#define __MOTOR_H

#define Motor_L 1			//左电机
#define Motor_R 2			//右电机
#define Motor_STOP 	0		//急停
#define Motor_FWD 	1		//正转
#define	Motor_REV 	2		//反转

void Motor_Init(void);
void Motor_Stop(void);
void Motor_Direction(uint8_t Motor_Id,uint8_t Direction);
void Motor_Speed(uint8_t Motor_Id,uint8_t Duty);

#endif
