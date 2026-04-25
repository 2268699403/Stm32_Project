#ifndef __PID_H
#define __PID_H

typedef struct {
	
	float Target;		//目标值
	float Actual;		//实际值
	float Out;			//输出值
	
	float Error0;		//本次误差
	float Error1;		//上次误差
	float ErrorInt;		//累计误差
		
	float Kp;			//比例系数
	float Ki;			//积分系数
	float Kd;			//微分系数
		
	float OutMax;		//最大输出值
	float OutMin;		//最小输出值

} PID_struct;

extern PID_struct PID_Angle;		//直立环配置
extern PID_struct PID_Speed;		//速度环配置
extern PID_struct PID_Turn;			//转向环配置

void PID_Init(PID_struct *p);
void PID_Update(PID_struct *p);
void PID_Angle_Update(void);
void PID_Move_Update(void);



#endif
