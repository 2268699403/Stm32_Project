#include "stm32f10x.h"
#include "PID.h"
#include "MPU6050.h"
#include "Motor.h"

PID_struct PID_Angle = {
	.Kp = 2.5,
	.Ki = 0.07,
	.Kd = 3,
	.OutMax = 100,
	.OutMin = -100,
};

/**
  * 函    数：初始化结构体初值
  * 参    数：结构体地址
  * 返 回 值：无
  */
void PID_Init(PID_struct *p)
{
	p->Target = 0;
	p->Actual = 0;
	p->Out	  = 0;
	p->Error0 = 0;
	p->Error1 = 0;
	p->ErrorInt = 0;
}


/**
  * 函    数：PID运算
  * 参    数：结构体地址
  * 返 回 值：无
  */
void PID_Update(PID_struct *p)
{
	/*误差计算*/
	p->Error1 = p->Error0;					//上次误差
	p->Error0 = p->Target - p->Actual;		//本次误差
	
	/*积分分离*/
	if (p->Ki != 0)
	{
		p->ErrorInt += p->Error0;
	}
	else
	{
		p->ErrorInt = 0;
	}
	
	/*PID控制器输出计算*/
	p->Out = p->Kp * p->Error0					//比例项：Kp × 当前误差	
		   + p->Ki * p->ErrorInt				//积分项：Ki × 误差积分
		   + p->Kd * (p->Error0 - p->Error1);	//微分项：Kd × 误差变化率
	
	/*输出限幅*/
	if (p->Out > p->OutMax) {p->Out = p->OutMax;}
	if (p->Out < p->OutMin) {p->Out = p->OutMin;}
}

void PID_Angle_Update(void)
{
	int16_t PWM_L = 0,PWM_R = 0;
	int16_t AvePWM = 0,DifPWM = 0;	
	
	PID_Angle.Actual = Angle;
	PID_Update(&PID_Angle);
	AvePWM = -PID_Angle.Out;
	
	PWM_L = AvePWM + DifPWM / 2;
	PWM_R = AvePWM - DifPWM / 2;
	
	if(PWM_L > 100){PWM_L = 100;} else if(PWM_L < -100){PWM_L = -100;}
	if(PWM_R > 100){PWM_R = 100;} else if(PWM_R < -100){PWM_R = -100;}
	
	Motor_Direction(Motor_L,PWM_L);
	Motor_Direction(Motor_R,PWM_R);
}


