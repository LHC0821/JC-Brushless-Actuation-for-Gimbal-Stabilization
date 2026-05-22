#include "gimbal_track.h"

// 实例化X轴(Yaw)和Y轴(Pitch)的PID控制器
Gimbal_PID_t pid_yaw;   // 控制X轴 (如：电机2)
Gimbal_PID_t pid_pitch; // 控制Y轴 (如：电机3)

// 云台当前的绝对角度目标 (单位: 度)
float target_yaw_deg = 0.0f;
float target_pitch_deg = 0.0f;

/**
 * @brief PID 参数初始化
 */
void Gimbal_PID_Init(void)
{
    // Yaw轴 (X轴) PID 参数
    pid_yaw.Kp = 0.02f;       
    pid_yaw.Ki = 0.000f;      // 初始调试建议先将Ki给0
    pid_yaw.Kd = 0.05f;       
    pid_yaw.max_integral = 5.0f;  
    pid_yaw.max_output = 5.0f;    // 单次最大转动角度限制在5度以内，保证平滑

    // Pitch轴 (Y轴) PID 参数
    pid_pitch.Kp = 0.02f; 
    pid_pitch.Ki = 0.000f;
    pid_pitch.Kd = 0.05f;
    pid_pitch.max_integral = 5.0f;
    pid_pitch.max_output = 5.0f;
    
    // 初始化云台到0度位置 (调用你JC.c里的函数)
    JC_Motor_SetZero(); 
}

/**
 * @brief PID 计算函数 (内部调用)
 */
static float PID_Calc(Gimbal_PID_t *pid, float error)
{
    float output;
    
    pid->error = error;
    pid->integral += error;
    
    // 积分抗饱和限幅
    if (pid->integral > pid->max_integral) pid->integral = pid->max_integral;
    else if (pid->integral < -pid->max_integral) pid->integral = -pid->max_integral;
    
    // 位置式PID计算 (此处输出的是角度的增量)
    output = (pid->Kp * pid->error) + 
             (pid->Ki * pid->integral) + 
             (pid->Kd * (pid->error - pid->last_error));
             
    pid->last_error = pid->error;
    
    // 输出限幅 (限制单次运动的最大步长)
    if (output > pid->max_output) output = pid->max_output;
    else if (output < -pid->max_output) output = -pid->max_output;
    
    return output;
}

/**
 * @brief 视觉云台追踪核心任务 
 * @note  请在主循环(while 1)中持续调用此函数，它会利用你定义的 vision_data 自动运行
 */
void Gimbal_Track_Task(void)
{
    // 1. 检查是否接收到新的视觉数据
    if (vision_data.is_new_data == 1)
    {
        // 立即清除新数据标志位，等待下一帧
        vision_data.is_new_data = 0;

        // 2. 检查目标是否被锁定 (根据你的定义: 0x01=找到并锁定)
        if (vision_data.status == 0x01)
        {
            // 获取你解算出的偏差值
            int16_t offset_x = vision_data.target_x;
            int16_t offset_y = vision_data.target_y;

            // 3. 死区控制：如果误差在极小范围内(如±3个像素)，认为是噪声，停止调节防抖动
            if(offset_x > -3 && offset_x < 3) offset_x = 0;
            if(offset_y > -3 && offset_y < 3) offset_y = 0;

            // 4. 通过PID计算出需要补偿的角度差值 (增量)
            float delta_yaw = PID_Calc(&pid_yaw, (float)offset_x);
            float delta_pitch = PID_Calc(&pid_pitch, (float)offset_y);
            
            // 5. 累加到当前的绝对角度中
            // 【重点】：这里的 += 还是 -= 取决于电机的安装方向！
            // 根据你的定义：X正数偏右。如果目标偏右，云台需要向右转以使目标居中。
            target_yaw_deg += delta_yaw;     
            
            // 根据你的定义：Y正数偏下。如果目标偏下，云台需要向下低头以使目标居中。
            target_pitch_deg += delta_pitch; 
            
            // 6. 云台机械限幅保护 (防止线缆扯断，假设左右45度，上下30度)
            if(target_yaw_deg > 45.0f) target_yaw_deg = 45.0f;
            if(target_yaw_deg < -45.0f) target_yaw_deg = -45.0f;
            
            if(target_pitch_deg > 30.0f) target_pitch_deg = 30.0f;
            if(target_pitch_deg < -30.0f) target_pitch_deg = -30.0f;

            // 7. 换算为 JC 电机通信协议需要的整型数据 (放大100倍)
            int32_t send_yaw = (int32_t)(target_yaw_deg * 100.0f);
            int32_t send_pitch = (int32_t)(target_pitch_deg * 100.0f);
            
            // 8. 调用你底层定义的函数，下发绝对位置指令
            JC_Motor2_SetAbsAngle(send_yaw);
            JC_Motor3_SetAbsAngle(send_pitch);
        }
        else 
        {
            // 目标未找到 (status == 0x00)
            // 可选：在此处将积分器清零，防止丢失目标期间积分器意外累加
            pid_yaw.integral = 0;
            pid_pitch.integral = 0;
            
            // 可选：目标丢失时让云台回到正前方 0 度位置
            target_yaw_deg = 0;
            target_pitch_deg = 0;
            JC_Motor2_SetAbsAngle(0);
            JC_Motor3_SetAbsAngle(0);
        }
    }
}