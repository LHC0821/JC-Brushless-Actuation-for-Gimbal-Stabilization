#ifndef __JC_H
#define __JC_H

#include <stdint.h>
#include "uart.h" // 包含底层 USART2_SendArray 和 USART3_SendArray 的声明

/* ================================================================= */
/* USART 2 电机专用控制接口                                          */
/* ================================================================= */

void JC_Motor2_CloseLoop(void);
void JC_Motor2_ReadVoltage(void);
void JC_Motor2_SetAbsAngle(int32_t angle);
void JC_Motor2_SetRelAngle(int32_t angle);
void JC_Motor2_PVTCommand(int32_t angle, uint16_t velocity, uint8_t torque_per);
void JC_Motor2_openLoop(void);

/* ================================================================= */
/* USART 3 电机专用控制接口                                          */
/* ================================================================= */

void JC_Motor3_CloseLoop(void);
void JC_Motor3_ReadVoltage(void);
void JC_Motor3_SetAbsAngle(int32_t angle);
void JC_Motor3_SetRelAngle(int32_t angle);
void JC_Motor3_PVTCommand(int32_t angle, uint16_t velocity, uint8_t torque_per);
void JC_Motor3_openLoop(void);

//test
void JC_Motor_Stop(void);
void JC_Motor_SetZero(void);

#endif /* __JC_H */