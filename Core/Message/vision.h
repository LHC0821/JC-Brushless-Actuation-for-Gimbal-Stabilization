#ifndef __VISION_H
#define __VISION_H

#include "main.h"
#include "usart.h"
#include "dma.h"

// ֡�ṹ���� (��������Python���ʹ���)
#define FRAME_HEADER    0xAA
#define FRAME_TAIL      0x55
#define DATA_LENGTH     5       // ���������ȣ�2��int16(4�ֽ�) + 1��״̬�ֽ� = 5�ֽ�
#define FRAME_SIZE      (1 + 1 + DATA_LENGTH + 1 + 1)  // ֡ͷ+����+����+У���+֡β = 8�ֽ�

// ״̬ö��
typedef enum {
  RX_IDLE = 0,       // ����״̬
  RX_RECEIVING,      // ������
  RX_COMPLETE,       // �������
  RX_ERROR           // ���մ���
} RxState;

// ���ݽṹ�� (��Ϊ�뷢�Ͷ˶�Ӧ)
typedef struct {
  int16_t target_x;  // Ŀ��X���� (���� err_center[0])
  int16_t target_y;  // Ŀ��Y���� (���� err_center[1])
  uint8_t status;    // ״̬ 0x01=�ҵ�, 0x00=δ�ҵ�
  uint8_t is_new_data; // �Ƿ��������ݱ�־
} VisionData;

// ȫ�ֱ�������
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern VisionData vision_data; // �ṹ�������Ҳ��Ӧ����
extern volatile uint8_t new_frame_received;
extern volatile RxState rx_state;

// ��������
void Vision_Receiver_Init(void);
void Vision_Start_DMA_Receive(void);
void Vision_Process_Frame(uint8_t *frame);
void Vision_Restart_DMA_Receive(void);

uint8_t* Vision_Get_DMA_Buffer(void);

#endif /* __VISION_H */