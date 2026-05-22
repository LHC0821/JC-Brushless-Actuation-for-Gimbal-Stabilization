#include "vision.h"
#include <string.h>
#include "usart.h" // 引入系统 hal 库配置的 huart1, huart2, huart3

// 全局变量 (严格保持您的定义，一点不改)
VisionData vision_data = {0};
volatile uint8_t new_frame_received = 0;
volatile RxState rx_state = RX_IDLE;

// DMA缓冲区
static uint8_t dma_rx_buffer[FRAME_SIZE]; 
static uint8_t frame_buffer[FRAME_SIZE];

// 添加这个函数，让外部可以获取缓冲区指针
uint8_t* Vision_Get_DMA_Buffer(void) {
    return dma_rx_buffer;
}

/**
 * @brief 初始化视觉接收
 */
void Vision_Receiver_Init(void) {
  memset(&vision_data, 0, sizeof(VisionData));
  vision_data.is_new_data = 0;
  rx_state = RX_IDLE;
  
  // 启动DMA接收
  Vision_Start_DMA_Receive();
}

/**
 * @brief 启动DMA接收
 */
void Vision_Start_DMA_Receive(void) {
  // 清空缓冲区
  memset(dma_rx_buffer, 0, FRAME_SIZE);
  
  // 启动DMA接收 (严格按照系统hal库要求，绑定到拥有DMA的huart1)
  if(HAL_UART_Receive_DMA(&huart1, dma_rx_buffer, FRAME_SIZE) == HAL_OK) {
    rx_state = RX_RECEIVING;
  } else {
    rx_state = RX_ERROR;
  }
}

/**
 * @brief 重新启动DMA接收
 */
void Vision_Restart_DMA_Receive(void) {
  // 先停止当前DMA
  HAL_UART_DMAStop(&huart1);
  
  // 重新启动
  Vision_Start_DMA_Receive();
}

/**
 * @brief 验证帧有效性
 * @param frame 帧数据指针
 * @return 1=有效 0=无效
 */
static uint8_t Validate_Frame(uint8_t *frame) {
  // 检查帧头
  if(frame[0] != FRAME_HEADER) {
    return 0;
  }
  
  // 检查数据长度
  if(frame[1] != DATA_LENGTH) {
    return 0;
  }
  
  // 检查帧尾
  if(frame[FRAME_SIZE - 1] != FRAME_TAIL) {
    return 0;
  }
  
  // 计算校验和 (从帧头累加到数据区最后一个字节)
  uint8_t checksum = 0;
  for(int i = 0; i < FRAME_SIZE - 2; i++) { // 减去校验和与帧尾的位置
    checksum += frame[i];
  }
  checksum &= 0xFF;
  
  // 验证校验和
  if(checksum != frame[FRAME_SIZE - 2]) {
    return 0;
  }
  
  return 1;
}

/**
 * @brief 处理接收到的帧
 * @param frame 帧数据指针
 */
void Vision_Process_Frame(uint8_t *frame) {
  // 验证帧有效性
  if(!Validate_Frame(frame)) {
    return;
  }
  
  // 复制帧数据
  memcpy(frame_buffer, frame, FRAME_SIZE);
  
  // 数据区起始位置：帧头(1) + 长度(1) = 第2字节
  uint8_t *data = &frame_buffer[2];
  
  // 解析数据 (大端序，与发送端pack('>hhB')对应)
  vision_data.target_x = (data[0] << 8) | data[1];   // 第一个int16
  vision_data.target_y = (data[2] << 8) | data[3];   // 第二个int16
  vision_data.status   = data[4];                    // 状态字节
  vision_data.is_new_data = 1;                      // 设置新数据标志
  new_frame_received = 1;                           // 触发新帧接收标志
}


