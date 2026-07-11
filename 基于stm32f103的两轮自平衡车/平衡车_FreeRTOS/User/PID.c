#include "stm32f10x.h"
#include "PID.h"
#include "MPU6050.h"
#include "Motor.h"
#include "Encoder.h"

float AveSpeed = 0, DifSpeed = 0;
int16_t PWM_L  = 0, PWM_R = 0;
int16_t AvePWM = 0, DifPWM = 0;


/* 直立环结构体配置 */
PID_struct PID_Angle = {
	.Kp = 3.5,
	.Ki = 0.3,
	.Kd = 6.5,
	.OutMax = 100,
	.OutMin = -100,
};


/* 速度环结构体配置 */
PID_struct PID_Speed = {
	.Kp = 0.065,
	.Ki = 0.003,
	.Kd = 0,
	.OutMax = 20,
	.OutMin = -20,
};


/* 转向环结构体配置 */
PID_struct PID_Turn = {
	.Kp = 0.05,
	.Ki = 0,
	.Kd = 0,
	.OutMax = 50,
	.OutMin = -50,
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

/**
  * 函    数：直立环PID更新
  * 参    数：无
  * 返 回 值：无
  */

void PID_Angle_Update(void)
{
	/* 获取MPU6050解算的当前角度作为实际值 */
	PID_Angle.Actual = Angle;
	/* 执行直立环PID运算，输出控制量 */
	PID_Update(&PID_Angle);
	/* 取反输出：当平衡车前倾时，需要输出的PWM趋势应为前进方向，与PID输出符号相反 */
	AvePWM = -PID_Angle.Out;	
	
	/* 电机差速混合：左电机 = 直立平均PWM - 转向差速/2，右电机 = 直立平均PWM + 转向差速/2 */
	PWM_L = AvePWM + DifPWM / 2;
	PWM_R = AvePWM - DifPWM / 2;
	
	/* PWM输出限幅保护，防止占空比超过100%导致电机异常 */
	if(PWM_L > 100){PWM_L = 100;} else if(PWM_L < -100){PWM_L = -100;}
	if(PWM_R > 100){PWM_R = 100;} else if(PWM_R < -100){PWM_R = -100;}
	
	/* 输出PWM控制电机旋转方向和速度 */
	Motor_Direction(Motor_L,PWM_L);
	Motor_Direction(Motor_R,PWM_R);
}


/**
  * 函    数：移动环PID更新
  * 参    数：无
  * 返 回 值：无
  */
void PID_Move_Update(void)
{
	
	AveSpeed = (RPM_L + RPM_R) / 2.0;
	DifSpeed = RPM_L - RPM_R;
	
	/* 速度环PID调控 */
	PID_Speed.Actual = AveSpeed;
	PID_Update(&PID_Speed);
	PID_Angle.Target = PID_Speed.Out;
	
	/* 转向环PID调控 */
	PID_Turn.Actual = DifSpeed;
	PID_Update(&PID_Turn);
	DifPWM = PID_Turn.Out;

}
