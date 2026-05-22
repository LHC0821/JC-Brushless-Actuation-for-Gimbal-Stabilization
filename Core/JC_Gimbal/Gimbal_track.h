#ifndef __GIMBAL_TRACK_H
#define __GIMBAL_TRACK_H

#include "main.h"
#include "JC.h"       // 你的电机底层驱动
#include "vision.h"   // 你的视觉通信模块

// PID 结构体定义
typedef struct {
    float Kp;           // 比例系数
    float Ki;           // 积分系数
    float Kd;           // 微分系数
    
    float error;        // 当前误差
    float last_error;   // 上次误差
    float integral;     // 积分累加值
    
    float max_integral; // 积分限幅
    float max_output;   // 输出限幅 (每次最大增量)
} Gimbal_PID_t;

// 外部引用的绝对角度变量
extern float target_yaw_deg;
extern float target_pitch_deg;

// 函数声明
void Gimbal_PID_Init(void);
void Gimbal_Track_Task(void);

#endif

