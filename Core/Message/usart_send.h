#ifndef __USART_SEND_H__
#define __USART_SEND_H__

#include "main.h"

void USART2_SendByte(uint8_t byte);
void USART2_SendArray(uint8_t *data, uint16_t len);
void USART2_SendString(char *str);
void USART2_SendNumber(uint32_t num, uint8_t len);
void USART2_Printf(char *format, ...);

void USART3_SendByte(uint8_t byte);
void USART3_SendArray(uint8_t *data, uint16_t len);
void USART3_SendString(char *str);
void USART3_SendNumber(uint32_t num, uint8_t len);
void USART3_Printf(char *format, ...);

#endif /* __USART_SEND_H__ */
