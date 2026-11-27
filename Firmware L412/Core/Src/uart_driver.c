/*
 * uart_transmit.c
 *
 *  Created on: Sep 24, 2026
 *      Author: Pavel
 */


#include <uart_driver.h>

/* ====== Глобальные указатели для ISR ====== */

static UART_Handle_t *g_usart1 = NULL;
static UART_Handle_t *g_usart2 = NULL;
static UART_Handle_t *g_lpuart1 = NULL;

/*
 * @brief Initializes USART according to the fields of the structure
 *
 * Enables the peripheral clock, resets CR1/CR2/CR3, calculates the BRR
 * based on the bus frequency and the desired baud rate, and enables the transmitter and receiver.
 * Once this function has been called, the port is ready to perform transmit and receive
 * operations in blocking mode (default format: 8N1).
 *
* @param  huart  Pointer to a UART handle. The Instance, pclk, and baudrate fields
*                must be populated prior to the call. The pclk field represents the bus frequency:
*                APB2 for USART1, and APB1 for USART2.
* @note   GPIO configuration (pins, AF) must be performed BEFORE calling this
*         function — the driver configures only the peripheral itself.
*
* @note   The USART clock source is selected in RCC->CCIPR. By default
*         (CCIPR = 00), PCLK is used, which is what this function expects.
*
* @retval None
 */
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

static void USART_DMA_Enable_TR(UART_Handle_t *huart)
{
	if (!huart || !huart->Instance) return;
	huart->Instance->CR3 |= USART_CR3_DMAT;
}

static void USART_DMA_Disable_TR(UART_Handle_t *huart)
{
	if (!huart || !huart->Instance) return;
	huart->Instance->CR3 &= ~USART_CR3_DMAT;
}

// only for USART1 and DMA1
void DMA1_USART1_Init_Tx(UART_Handle_t *huart, DMA_Mode mode)
{
	if (!huart || !huart->Instance) return;
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

	DMA1_CSELR->CSELR |= (0b0010 << DMA_CSELR_C4S_Pos);

	DMA1_Channel4->CCR &= ~DMA_CCR_EN;

	DMA1_Channel4->CPAR  = (uint32_t)&huart->Instance->TDR;

	if(mode == DMA_MODE_CIRCULAR)
		DMA1_Channel4->CCR |= DMA_CCR_CIRC;

	DMA1_Channel4->CCR |= DMA_CCR_MINC | DMA_CCR_DIR;
}

bool DMA1_USART1_Send(UART_Handle_t *huart, uint8_t *data, uint16_t len)
{
    if (huart->tx_busy) return false;

    huart->tx_busy = true;
	DMA1_Channel4->CCR &= ~DMA_CCR_EN;

	DMA1_Channel4->CMAR  = (uint32_t)data;
	DMA1_Channel4->CNDTR = len;

	USART_DMA_Enable_TR(huart);
	DMA1_Channel4->CCR |= DMA_CCR_EN;

	return true;

}

void DMA1_USART1_Enable_IRQ(UART_Handle_t *huart, uint8_t irqn_priority)
{
	DMA1_Channel4->CCR &= ~DMA_CCR_EN;

	DMA1_Channel4->CCR |= DMA_CCR_HTIE | DMA_CCR_TCIE; //enable half-transmit and transfer complete interruptions

	NVIC_SetPriority(DMA1_Channel4_IRQn, irqn_priority);
	NVIC_EnableIRQ(DMA1_Channel4_IRQn);
	g_usart1 = huart;

	DMA1_Channel4->CCR |= DMA_CCR_EN;


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
//returns true if success/ returns false if timeout
bool UART_SendBuf(UART_Handle_t *huart, const uint8_t *buf, uint16_t len, uint32_t timeout_per_byte)
{
    if (!huart || !huart->Instance || !buf || len == 0) return false;

    for (uint16_t i = 0; i < len; i++)
    {
        if (!UART_SendByte(huart, buf[i], timeout_per_byte))
            return false;  // timeout at a certain byte
    }

    // Дождаться TC: гарантирует, что последний байт физически ушёл из сдвигового регистра
    uint32_t t = timeout_per_byte;
    while (!(huart->Instance->ISR & USART_ISR_TC)) {
        if (t == 0)
            return false;
        t--;
    }
    return true;
}
// returns true if success (buf received), returns false if timeout
bool UART_RecvBuf(UART_Handle_t *huart, uint8_t *buf, uint16_t len, uint32_t timeout_per_byte)
{
    if (!huart || !huart->Instance || !buf || len == 0) return false;

    for (uint16_t i = 0; i < len; i++)
    {
        if (!UART_RecvByte(huart, &buf[i], timeout_per_byte))
            return false;
    }

    return true;
}

void USART_EnableIRQ(UART_Handle_t *huart, uint8_t irqn_priority)
{
	if (!huart || !huart->Instance) return;

	huart->rx_buffer.head = 0;
	huart->rx_buffer.tail = 0;
	huart->tx_buffer.head = 0;
	huart->tx_buffer.tail = 0;

	IRQn_Type irqn;
	if(huart->Instance == USART1)
	{
		irqn = USART1_IRQn;
		g_usart1 = huart;
	}
	else if(huart->Instance == USART2)
	{
		irqn = USART2_IRQn;
		g_usart2 = huart;
	}
	else if(huart->Instance == LPUART1)
	{
		irqn = LPUART1_IRQn;
		g_lpuart1 = huart;
	}

	else return;

    NVIC_SetPriority(irqn, irqn_priority);
    NVIC_EnableIRQ(irqn);

	huart->Instance->CR1 |= USART_CR1_RXNEIE;
}

bool USART_Send_Byte_IT(UART_Handle_t *huart, uint8_t byte)
{
	if (!huart || !huart->Instance) return false;

    if (!ring_buffer_put(&huart->tx_buffer, byte))
        return false;

    huart->Instance->CR1 |= USART_CR1_TXEIE;

    return true;
}

uint16_t USART_Send_Data_IT(UART_Handle_t *huart, const uint8_t *data, uint16_t len)
{
    if (!huart || !huart->Instance || !data) return 0;
    uint16_t n = 0;
    while (len--) {
        if (!USART_Send_Byte_IT(huart, *data++)) break;
        n++;
    }
    return n;
}

void USART_Send_String_IT(UART_Handle_t *huart,const char *str)
{
	if (!huart || !huart->Instance || str == NULL) return;
    while (*str)
    {
    	if(!USART_Send_Byte_IT(huart, *str++))
    		break;
    }
}

/*
 * @retval: true - byte was read, false - buffer is empty
 */
bool USART_Read_Byte_IT(UART_Handle_t *huart, uint8_t *data)
{
    if (!huart || !huart->Instance || !data) return false;
    return ring_buffer_get(&huart->rx_buffer, data);
}


/*
 * @retval: uint16_t - number of bytes read. If les than len - the buffer became empty before end of data array
 */
uint16_t USART_Read_Buf_IT(UART_Handle_t * huart, uint8_t *buf, uint16_t len)
{
	if (!huart || !huart->Instance || !buf || len == 0) return 0;
	uint16_t n = 0;
	while(n < len)
	{
		uint8_t data;
		if(!ring_buffer_get(&huart->rx_buffer, &data))
			break;
		buf[n++] = data;
	}
	return n;
}

static void USART_IRQ_Handler(UART_Handle_t *huart)
{
	if (!huart) return;

	USART_TypeDef *u = huart->Instance;
	uint32_t isr = u->ISR;

	//overrun
    if (isr & USART_ISR_ORE)
    {
        u->ICR |= USART_ICR_ORECF;
        huart->rx_overflow++;
    }

    if(isr & USART_ISR_RXNE)
    {
    	uint8_t data = (uint8_t)(u->RDR & 0xFFU);
        (void)ring_buffer_put(&huart->rx_buffer, data);
    }
    if (isr & USART_ISR_TXE)
    {
        uint8_t data;
        if (ring_buffer_get(&huart->tx_buffer, &data))
            u->TDR = data;
        else
            u->CR1 &= ~USART_CR1_TXEIE;
    }


}

void USART1_IRQHandler(void) { USART_IRQ_Handler(g_usart1); }

void USART2_IRQHandler(void) { USART_IRQ_Handler(g_usart2); }

void DMA1_Channel4_IRQHandler(void)
{
	if(DMA1->ISR & DMA_ISR_TCIF4)
	{
		DMA1->IFCR = DMA_IFCR_CTCIF4;

		DMA1_Channel4->CCR &= ~DMA_CCR_EN;

		USART1->CR3 &= ~USART_CR3_DMAT;
		g_usart1->tx_busy = false;
	}

	if(DMA1->ISR & DMA_ISR_HTIF4)
	{
		DMA1->IFCR = DMA_IFCR_CHTIF4;
		GPIOA->ODR ^= GPIO_ODR_OD3;
	}


}
