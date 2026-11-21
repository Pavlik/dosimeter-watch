/*
 * uart_tranmit.h
 *
 *  Created on: Sep 24, 2026
 *      Author: Pavel
 */

#ifndef INC_UART_DRIVER_H_
#define INC_UART_DRIVER_H_

#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "ring_buffer.h"

typedef struct
{
    USART_TypeDef *Instance;

//    uint8_t *rx_buf;
//    uint16_t rx_size;
//    uint8_t *tx_buf;
//    uint16_t tx_size;


    uint32_t baudrate;   // speed
    uint32_t pclk;       // bus clock APB2 for USART1, APB1 for USART2/3 и LPUART1
    uint8_t word_length;   // 7/8/9
    uint8_t parity;        // 0=None, 1=Even, 2=Odd
    uint8_t stop_bits;     // 1 or 2

    RingBuffer rx_buffer;
    RingBuffer tx_buffer;
//    volatile uint16_t head;
//    volatile uint16_t tail;
} UART_Handle_t;


void USART_Init(UART_Handle_t *huart);
void LPUART_Init(UART_Handle_t *huart);
void USART_EnableIRQ(UART_Handle_t *huart, uint8_t irqn_priority);

bool UART_SendByte(UART_Handle_t *huart, uint8_t data, uint32_t timeout);
bool UART_RecvByte(UART_Handle_t *huart, uint8_t *data, uint32_t timeout);
bool UART_SendBuf(UART_Handle_t *huart, const uint8_t *buf, uint16_t len, uint32_t timeout_per_byte);
bool UART_RecvBuf(UART_Handle_t *huart, uint8_t *buf, uint16_t len, uint32_t timeout_per_byte);
//void uart_send_init(void);
//bool uart_send_byte_it(uint8_t byte);
//void uart_send_string_it(const char *str);
//void uart_send_data_it(uint8_t *data, uint16_t len);



#endif /* INC_UART_DRIVER_H_ */
