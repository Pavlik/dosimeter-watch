/*
 * uart_transmit.c
 *
 *  Created on: Sep 24, 2026
 *      Author: Pavel
 */


#include "uart_transmit.h"

static RingBuffer uart_tx;

void uart_send_init(void)
{
	uart_tx.head = 0;
	uart_tx.tail = 0;

    NVIC_SetPriority(USART1_IRQn, 0);
    NVIC_EnableIRQ(USART1_IRQn);
}

bool uart_send_byte_it(uint8_t byte)
{
    if (!ring_buffer_put(&uart_tx, byte))
        return false;
    USART1->CR1 |= USART_CR1_TXEIE;
    return true;
}


void uart_send_string_it(const char *str)
{
    while (*str)
    {
    	if(!uart_send_byte_it(*str++))
    		break;
    }
}

void USART1_IRQHandler(void)
{
    if (USART1->ISR & USART_ISR_TXE)
    {
        uint8_t data;
        if (ring_buffer_get(&uart_tx, &data))
            USART1->TDR = data;
        else
            USART1->CR1 &= ~USART_CR1_TXEIE;
    }
}



//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
//{
//	if(huart->Instance != USART1)
//		return;
//	if(ring_buffer_get(&tx_usart, &tx_current))
//	{
//		if(HAL_UART_Transmit_IT(&huart1, &tx_current, 1) != HAL_OK)
//			tx_active = false;
//	}
//	else tx_active = false;
//}
