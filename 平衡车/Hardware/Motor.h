#ifndef __MOTOR_H
#define __MOTOR_H

#define Motor_L 1			//左电机
#define Motor_R 0			//右电机
#define Motor_STOP 	0		//急停
#define Motor_FWD 	1		//正转
#define	Motor_REV 	2		//反转
#define AIN_1 GPIO_Pin_12
#define AIN_2 GPIO_Pin_13
#define BIN_1 GPIO_Pin_14
#define BIN_2 GPIO_Pin_15

void Motor_Init(void);
void Motor_Stop(void);
void Motor_Direction(uint8_t Motor_Id,uint8_t Direction);
void Motor_Speed(uint8_t Motor_Id,uint8_t Duty);

#endif
