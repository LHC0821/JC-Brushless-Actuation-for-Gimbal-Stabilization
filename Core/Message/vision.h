#ifndef __VISION_H
#define __VISION_H

#include "main.h"
#include "usart.h"
#include "dma.h"

/* ====================================================================
 *                         视觉通信帧结构定义
 * ==================================================================== */
#define FRAME_HEADER    0xAA        // 帧头：标记一帧数据的开始
#define FRAME_TAIL      0x55        // 帧尾：标记一帧数据的结束
#define DATA_LENGTH     5           // 有效数据长度：2个int16(4字节) + 1个状态(1字节) = 5字节

// 接收总帧长计算：帧头(1) + 长度字节(1) + 数据(5) + 校验和(1) + 帧尾(1) = 9字节
#define FRAME_SIZE      (1 + 1 + DATA_LENGTH + 1 + 1)  

/* ====================================================================
 *                         状态与数据结构体
 * ==================================================================== */
/**
 * @brief 接收状态机枚举
 */
typedef enum {
  RX_IDLE = 0,       // 空闲状态 / 等待启动
  RX_RECEIVING,      // DMA 正在接收中
  RX_COMPLETE,       // 一帧数据接收完成（待处理）
  RX_ERROR           // 串口或DMA启动错误
} RxState;

/**
 * @brief 视觉数据解析后的存储结构体
 */
typedef struct {
  int16_t target_x;    // 目标 X 坐标偏差值值（负数代表偏左，正数代表偏右）
  int16_t target_y;    // 目标 Y 坐标偏差值值（负数代表偏上，正数代表偏下）
  uint8_t status;      // 目标状态：0x01=找到目标并锁定, 0x00=未找到目标
  uint8_t is_new_data; // 新数据到达标志位：1=有未读新数据, 0=数据已被读取
} VisionData;

/* ====================================================================
 *                        全局变量声明 (外部可调用)
 * ==================================================================== */
extern UART_HandleTypeDef huart1;       // 声明底层绑定的串口1句柄
extern DMA_HandleTypeDef hdma_usart1_rx; // 声明底层的DMA接收句柄
extern VisionData vision_data;          // 存储最终解析出的视觉结果
extern volatile uint8_t new_frame_received; // 外部主循环查询的帧接收标志
extern volatile RxState rx_state;       // 当前接收状态机状态

/* ====================================================================
 *                          对外接口函数
 * ==================================================================== */
void Vision_Receiver_Init(void);          // 初始化视觉接收模块
void Vision_Start_DMA_Receive(void);      // 开启一次新的 DMA 接收
void Vision_Process_Frame(uint8_t *frame);// 校验并解析原始帧数据
void Vision_Restart_DMA_Receive(void);    // 异常或单次接收完成后重启 DMA
uint8_t* Vision_Get_DMA_Buffer(void);     // 获取底层 DMA 缓冲区的指针

#endif /* __VISION_H */