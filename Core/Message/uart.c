#include "uart.h"
#include "usart.h"
#include <stdio.h>
#include <stdarg.h>

extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;

static uint32_t Serial_Pow(uint32_t x, uint32_t y)
{
    uint32_t result = 1;
    while (y--) result *= x;
    return result;
}

/* ---------- USART2 ---------- */



void USART2_SendByte(uint8_t byte)
{
    while (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TXE) == RESET);
    huart2.Instance->DR = byte;
}



void USART2_SendArray(uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        USART2_SendByte(data[i]);
    }
}



void USART2_SendString(char *str)
{
    while (*str) {
        USART2_SendByte((uint8_t)*str++);
    }
}



void USART2_SendNumber(uint32_t num, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {

        USART2_SendByte((num / Serial_Pow(10, len - i - 1) % 10) + '0');
    }
}



void USART2_Printf(char *format, ...)
{
    char buf[100];
    va_list arg;
    va_start(arg, format);
    vsnprintf(buf, sizeof(buf), format, arg);
    va_end(arg);
    USART2_SendString(buf);

}


/* ---------- USART3 ---------- */


void USART3_SendByte(uint8_t byte)
{
    while (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_TXE) == RESET);
    huart3.Instance->DR = byte;
}


void USART3_SendArray(uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        USART3_SendByte(data[i]);
    }
}



void USART3_SendString(char *str)
{
    while (*str) {
        USART3_SendByte((uint8_t)*str++);
    }
}



void USART3_SendNumber(uint32_t num, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++) {
        USART3_SendByte((num / Serial_Pow(10, len - i - 1) % 10) + '0');
    }
}



void USART3_Printf(char *format, ...)
{

    char buf[100];
    va_list arg;
    va_start(arg, format);
    vsnprintf(buf, sizeof(buf), format, arg);
    va_end(arg);
    USART3_SendString(buf);
}