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

    uint32_t baudrate;   // speed
    uint32_t pclk;       // bus clock APB2 for USART1, APB1 for USART2/3 и LPUART1
    uint8_t word_length;   // 7/8/9
    uint8_t parity;        // 0=None, 1=Even, 2=Odd
    uint8_t stop_bits;     // 1 or 2

    RingBuffer rx_buffer;
    RingBuffer tx_buffer;

    volatile uint32_t rx_overflow;   // сколько байт потеряно из-за полного rx_buffer

    volatile bool tx_busy;
    DMA_TypeDef *dma;
    DMA_Channel_TypeDef *dma_channel;

} UART_Handle_t;

typedef enum
{
	DMA_MODE_NORMAL = 0,
	DMA_MODE_CIRCULAR
} DMA_Mode;

typedef enum
{
	DMA_USART_TX_ENABLE = 0,
	DMA_USART_TX_DISABLE,
	DAM_USART_RX_ENABLE,
	DMA_USART_RX_DISABLE
}DMA_USART_tx_rx_enable;


void USART_Init(UART_Handle_t *huart);
void DMA1_USART1_Init_Tx(UART_Handle_t *huart, DMA_Mode mode);
bool DMA1_USART1_Send(UART_Handle_t *huart, uint8_t *data, uint16_t len);
void DMA1_USART1_Enable_IRQ(UART_Handle_t *huart, uint8_t irqn_priority);

void LPUART_Init(UART_Handle_t *huart);
void USART_EnableIRQ(UART_Handle_t *huart, uint8_t irqn_priority);

bool UART_SendByte(UART_Handle_t *huart, uint8_t data, uint32_t timeout);
bool UART_RecvByte(UART_Handle_t *huart, uint8_t *data, uint32_t timeout);
bool UART_SendBuf(UART_Handle_t *huart, const uint8_t *buf, uint16_t len, uint32_t timeout_per_byte);
bool UART_RecvBuf(UART_Handle_t *huart, uint8_t *buf, uint16_t len, uint32_t timeout_per_byte);

bool USART_Send_Byte_IT(UART_Handle_t *huart, uint8_t byte);
uint16_t USART_Send_Data_IT(UART_Handle_t *huart, const uint8_t *data, uint16_t len);
void USART_Send_String_IT(UART_Handle_t *huart,const char *str);
bool USART_Read_Byte_IT(UART_Handle_t * huart, uint8_t *data);
uint16_t USART_Read_Buf_IT(UART_Handle_t * huart, uint8_t *buf, uint16_t len);




#endif /* INC_UART_DRIVER_H_ */
