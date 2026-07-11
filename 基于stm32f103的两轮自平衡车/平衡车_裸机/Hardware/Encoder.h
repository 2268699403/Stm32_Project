#ifndef __ENCODER_H
#define __ENCODER_H

extern float RPM_L;
extern float RPM_R;

void Encoder_Init(void);
void Encoder_GetState(void);

#endif
