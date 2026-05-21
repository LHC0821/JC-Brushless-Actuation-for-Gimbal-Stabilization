#ifndef __UART_H
#define __UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

/* ---------- USART2 ---------- */

/**
 * @brief  USART2发送一个字节
 * @param  byte: 要发送的数据
 * @retval 无
 */
void USART2_SendByte(uint8_t byte);

/**
 * @brief  USART2发送数组
 * @param  data: 数据数组指针
 * @param  len: 数据长度
 * @retval 无
 */
void USART2_SendArray(uint8_t *data, uint16_t len);

/**
 * @brief  USART2发送字符串
 * @param  str: 字符串指针
 * @retval 无
 */
void USART2_SendString(char *str);

/**
 * @brief  USART2发送数字
 * @param  num: 要发送的数字
 * @param  len: 数字长度
 * @retval 无
 */
void USART2_SendNumber(uint32_t num, uint8_t len);

/**
 * @brief  USART2格式化打印
 * @param  format: 格式化字符串
 * @retval 无
 */
void USART2_Printf(char *format, ...);


/* ---------- USART3 ---------- */

/**
 * @brief  USART3发送一个字节
 * @param  byte: 要发送的数据
 * @retval 无
 */
void USART3_SendByte(uint8_t byte);

/**
 * @brief  USART3发送数组
 * @param  data: 数据数组指针
 * @param  len: 数据长度
 * @retval 无
 */
void USART3_SendArray(uint8_t *data, uint16_t len);

/**
 * @brief  USART3发送字符串
 * @param  str: 字符串指针
 * @retval 无
 */
void USART3_SendString(char *str);

/**
 * @brief  USART3发送数字
 * @param  num: 要发送的数字
 * @param  len: 数字长度
 * @retval 无
 */
void USART3_SendNumber(uint32_t num, uint8_t len);

/**
 * @brief  USART3格式化打印
 * @param  format: 格式化字符串
 * @retval 无
 */
void USART3_Printf(char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* __UART_H */