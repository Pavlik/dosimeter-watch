/*
 * uart_transmit.c
 *
 *  Created on: Sep 24, 2026
 *      Author: Pavel
 */


#include "uart_transmit.h"

static RingBuffer uart_tx;

void USART_Init(UART_Handle_t *huart)
{
	if (!huart || !huart->Instance || !huart->baudrate || !huart->pclk) return;
	//clock
	if(huart->Instance == USART1)
	{
		RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	}
	else if(huart->Instance == USART2)
		RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;
	else
		return;

    USART_TypeDef *u = huart->Instance;

    u->CR1 = 0;  // reset, UE=0
    u->CR2 = 0;  // 1 stop bit, without CLKEN and other
    u->CR3 = 0;

    u->BRR = (huart->pclk + huart->baudrate / 2) / huart->baudrate;
	/*
	 * RCC->CCIPR &= ~RCC_CCIPR_USART1SEL; // CCIPR - choosing of USART clock (PCLK/SYSCLK/HSI16/LSE)
	 * default CCIPR == 00 means USART clk from PCLK
	 *
	 */

	huart->Instance->CR1 = USART_CR1_TE // ENABLE UART TX
						 | USART_CR1_RE // ENABLE UART RX
						 | USART_CR1_UE; // ENABLE UART
}

void LPUART_Init(UART_Handle_t *huart)
{
	if (!huart || !huart->Instance || !huart->baudrate || !huart->pclk) return;
	//clock
	RCC->APB1ENR2 |= RCC_APB1ENR2_LPUART1EN;

    USART_TypeDef *u = huart->Instance;

    u->CR1 = 0;  // reset, UE=0
    u->CR2 = 0;  // 1 stop bit, without CLKEN and other
    u->CR3 = 0;

    u->BRR = 256 * huart->pclk / huart->baudrate;
	/*
	 * RCC->CCIPR &= ~RCC_CCIPR_USART1SEL; // CCIPR - choosing of USART clock (PCLK/SYSCLK/HSI16/LSE)
	 * default CCIPR == 00 means USART clk from PCLK
	 *
	 */

	huart->Instance->CR1 = USART_CR1_TE // ENABLE UART TX
						 | USART_CR1_RE // ENABLE UART RX
						 | USART_CR1_UE; // ENABLE UART
}

//returns true if success/ returns false if timeout
bool UART_SendByte(UART_Handle_t *huart, uint8_t data, uint32_t timeout)
{
    if (!huart || !huart->Instance) return false;

    // Wait until TXE is set (send register is empty)
    while (!(huart->Instance->ISR & USART_ISR_TXE))
    {
        if (timeout-- == 0) return false;
    }
    // send byte
    huart->Instance->TDR = data;
    return true;
}

// returns true if success (byte received), returns false if timeout
bool UART_RecvByte(UART_Handle_t *huart, uint8_t *data, uint32_t timeout)
{
    if (!huart || !huart->Instance || !data) return false;

    // wait until RXNE is set (receive register is not empty)
    while (!(huart->Instance->ISR & USART_ISR_RXNE))
    {
        if (timeout-- == 0) return false;
    }
    //get byte
    *data = (uint8_t)(huart->Instance->RDR & 0xFF);
    return true;
}

//void uart_send_init(void)
//{
//	uart_tx.head = 0;
//	uart_tx.tail = 0;
//
//    NVIC_SetPriority(USART1_IRQn, 1);
//    NVIC_EnableIRQ(USART1_IRQn);
//}

//bool uart_send_byte_it(uint8_t byte)
//{
//    if (!ring_buffer_put(&uart_tx, byte))
//        return false;
//    USART1->CR1 |= USART_CR1_TXEIE;
//    return true;
//}
//
//void uart_send_data_it(uint8_t *data, uint16_t len)
//{
//    while(len--)
//    {
//        uart_send_byte_it(*data++);
//    }
//}
//
//
//void uart_send_string_it(const char *str)
//{
//    while (*str)
//    {
//    	if(!uart_send_byte_it(*str++))
//    		break;
//    }
//}
//
//void USART1_IRQHandler(void)
//{
//    if (USART1->ISR & USART_ISR_TXE)
//    {
//        uint8_t data;
//        if (ring_buffer_get(&uart_tx, &data))
//            USART1->TDR = data;
//        else
//            USART1->CR1 &= ~USART_CR1_TXEIE;
//    }
//}
