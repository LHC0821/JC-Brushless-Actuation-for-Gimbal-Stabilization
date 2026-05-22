#include "vision.h"
#include <string.h>
#include <stdio.h>
#include "usart.h"

/* ====================================================================
 *                          静态全局变量
 * ==================================================================== */
// 严格保持您的定义：vision_data 及标志位初始化
VisionData vision_data = {0};
volatile uint8_t new_frame_received = 0;
volatile RxState rx_state = RX_IDLE;

// DMA 直接写入的原始缓冲区（外部不可见，保护数据安全）
static uint8_t dma_rx_buffer[FRAME_SIZE]; 
// 拷贝出来用于解析的二级缓冲区，防止解析过程中 DMA 数据被下一帧冲掉
static uint8_t frame_buffer[FRAME_SIZE];

/**
 * @brief  获取底层 DMA 接收缓冲区的地址
 * @param  无
 * @return uint8_t* 指向 dma_rx_buffer 的指针
 */
uint8_t* Vision_Get_DMA_Buffer(void) {
    return dma_rx_buffer;
}

/**
 * @brief  初始化视觉接收模块
 * @note   在 main.c 的初始化区域调用一次即可
 */
void Vision_Receiver_Init(void) {
  memset(&vision_data, 0, sizeof(VisionData));
  vision_data.is_new_data = 0;
  rx_state = RX_IDLE;
  
  // 启动初次 DMA 接收
  Vision_Start_DMA_Receive();
}

/**
 * @brief  启动串口 DMA 异步接收
 * @note   调用后，底层硬件会自动在后台拼装字节，满 FRAME_SIZE (9) 字节后挂起并触发回调
 */
void Vision_Start_DMA_Receive(void) {
  // 接收前清空缓冲区，规避历史脏数据干扰
  memset(dma_rx_buffer, 0, FRAME_SIZE);
  
  // 启动 DMA 接收 (绑定到拥有接收 DMA 通道的 huart1)
  if(HAL_UART_Receive_DMA(&huart1, dma_rx_buffer, FRAME_SIZE) == HAL_OK) {
    rx_state = RX_RECEIVING; // 成功切入接收状态
  } else {
    rx_state = RX_ERROR;     // 硬件接口调用失败
  }
}

/**
 * @brief  强制停止并重新启动 DMA 接收
 * @note   用于数据校验失败、帧错位或者单次 Normal 模式接收完成后的复位重启
 */
void Vision_Restart_DMA_Receive(void) {
  // 先安全停止当前 DMA，防止指针死锁或数据撕裂
  HAL_UART_DMAStop(&huart1);
  
  // 重新配置并启动
  Vision_Start_DMA_Receive();
}

/**
 * @brief  静态内部函数：严格验证接收帧的合法性（头部、长度、尾部、校验和）
 * @param  frame: 待验证的原始数据包指针
 * @return 1=数据完全合法，0=非法伪包
 */
static uint8_t Validate_Frame(uint8_t *frame) {
  // 1. 验证帧头是否为 0xAA
  if(frame[0] != FRAME_HEADER) {
    return 0;
  }
  
  // 2. 验证视觉端声明的数据内容长度是否为 5
  if(frame[1] != DATA_LENGTH) {
    return 0;
  }
  
  // 3. 验证帧尾是否为 0x55
  if(frame[FRAME_SIZE - 1] != FRAME_TAIL) {
    return 0;
  }
  
  // 4. 计算校验和 (从 frame[0] 累加到数据区最后一个字节 frame[6])
  uint8_t checksum = 0;
  for(int i = 0; i < FRAME_SIZE - 2; i++) { // FRAME_SIZE-2 即扣除了校验字节和帧尾
    checksum += frame[i];
  }
  checksum &= 0xFF; // 显式限制在 8 位范围内
  
  // 5. 将计算出的校验和与接收到的校验位进行比对
  if(checksum != frame[FRAME_SIZE - 2]) {
    return 0; // 校验和不匹配，判定为错码包
  }
  
  return 1; // 通过所有关卡，判定为有效数据帧
}

/**
 * @brief  解析并提取帧中的物理核心数据
 * @param  frame: 指向已接收满的原始缓冲区的指针
 * @note   该函数内部处理了 Python 端 `>hhB` 带来的大端序字节序转换问题
 */
void Vision_Process_Frame(uint8_t *frame) {
  // 验证帧有效性，若通不过则直接丢弃不解析
  if(!Validate_Frame(frame)) {
    rx_state = RX_ERROR;
    return;
  }
  
  // 核心保护：迅速将数据移入二级缓冲区，释放底层 DMA 缓冲以防后续干扰
  memcpy(frame_buffer, frame, FRAME_SIZE);
  
  // 数据区起始物理位置：帧头(1字节) + 长度(1字节) = 索引 2
  uint8_t *data = &frame_buffer[2];
  
  // 解析数据 (大端序解包：高位字节左移8位 OR 低位字节)
  vision_data.target_x = (data[0] << 8) | data[1];   // 解析第一个 int16_t (X坐标)
  vision_data.target_y = (data[2] << 8) | data[3];   // 解析第二个 int16_t (Y坐标)
  vision_data.status   = data[4];                    // 解析状态字节 (0x01/0x00)
  
  vision_data.is_new_data = 1;                       // 标记用户数据区有未读的新数据
  new_frame_received = 1;                            // 触发全局新帧通知标志
  rx_state = RX_COMPLETE;                            // 更新接收状态机
}

/* 在 main.c 的最下方添加此回调函数 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // 判断是否是负责视觉通信的串口触发了中断
    if(huart->Instance == USART1) 
    {
        // 1. 获取底层 DMA 缓冲区的指针，并传入解析函数
        Vision_Process_Frame(Vision_Get_DMA_Buffer());
        // 2. 无论本次解析成功与否，必须重启 DMA 接收，否则串口只会接收这一次
        Vision_Restart_DMA_Receive();
    }
}