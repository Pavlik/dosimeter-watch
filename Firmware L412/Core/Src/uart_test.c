/*
 * uart_test.c
 *
 *  Created on: Nov 21, 2026
 *      Author: Pavel
 */

#include "uart_test.h"

UART_Handle_t uart1 =
{
	.Instance = USART1,
	.baudrate = 115200,
	.pclk = 24000000U,
};
static uint8_t ch;


void USART_Init_Test()
{
	USART_Init(&uart1);
    /* ============================================================
     * EXAMPLE 1: Send one byte
     * ============================================================ */
	UART_SendByte(&uart1, 'A', UART_TIMEOUT_MS(10));
	UART_SendByte(&uart1, '\r', UART_TIMEOUT_MS(10));
	UART_SendByte(&uart1, '\n', UART_TIMEOUT_MS(10));

    /* ============================================================
     * EXAMPLE 2: Send buffer of bytes (string)
     * ============================================================ */
	const char *msg = "Hello World!\r\n";
	UART_SendBuf(&uart1, (uint8_t*)msg, strlen(msg)-1, UART_TIMEOUT_MS(10));

    /* ============================================================
     * ПРИМЕР 3: Отправка бинарного массива
     * ============================================================ */
    uint8_t binary[] = {0x01, 0x02, 0x03, 0xFF, 0x00};
    UART_SendBuf(&uart1, binary, sizeof(binary), UART_TIMEOUT_MS(10));

    /* ============================================================
     * ПРИМЕР 4: Приём одного байта (блокирующий, ждём до 5 секунд)
     * ============================================================ */

    if (UART_RecvByte(&uart1, &ch, UART_TIMEOUT_MS(5000))) {
        // Эхо: отправляем принятый байт обратно
        UART_SendByte(&uart1, ch, UART_TIMEOUT_MS(10));
    } else {
        // Таймаут — байт не пришёл
        const char *err = "TIMEOUT\r\n";
        UART_SendBuf(&uart1, (const uint8_t *)err, strlen(err), UART_TIMEOUT_MS(10));
    }

    /* ============================================================
     * ПРИМЕР 5: Приём массива (например, 4 байта команды)
     * ============================================================ */
    uint8_t cmd[4];
    if (UART_RecvBuf(&uart1, cmd, sizeof(cmd), UART_TIMEOUT_MS(2000))) {
        // Приняли 4 байта — отправляем обратно
        UART_SendBuf(&uart1, cmd, sizeof(cmd), UART_TIMEOUT_MS(10));
    }

}
void USART_While_Test()
{
    if (UART_RecvByte(&uart1, &ch, UART_TIMEOUT_MS(100)))
    {
        UART_SendByte(&uart1, ch, UART_TIMEOUT_MS(10));
    }
    // Таймаут каждые 100 мс — просто продолжаем цикл
}

void USART_Init_Test_IT()
{
	USART_Init(&uart1);
	USART_EnableIRQ(&uart1, 1);

	USART_Send_Byte_IT(&uart1, 'X');
	USART_Send_Byte_IT(&uart1, '\r');
	USART_Send_Byte_IT(&uart1, '\n');

	//char msg[16] = "Hello World\r\n";
	USART_Send_String_IT(&uart1, "Ready. Type something:\r\n");
}
uint8_t buf[32];

// calls from while
static void Echo_Test()
{
	uint8_t ch;
    while (USART_Read_Byte_IT(&uart1, &ch))
    {
        USART_Send_Byte_IT(&uart1, ch);   // эхо
        GPIOA->ODR ^= GPIO_ODR_OD3;
    }
}

static void Control_Led()
{
	uint8_t byte;
	while((USART_Read_Byte_IT(&uart1, &byte)))
	{
		switch(byte)
		{
		case '1':
			GPIOA->BSRR =  GPIO_BSRR_BS3;
			USART_Send_String_IT(&uart1, "LED is ON \r\n");
			break;
		case '0':
			GPIOA->BSRR =  GPIO_BSRR_BR3;
			USART_Send_String_IT(&uart1, "LED is OFF \r\n");
			break;
		case '?':
			if(GPIOA->IDR & GPIO_IDR_ID3)
				USART_Send_String_IT(&uart1, "State: ON\r\n");
			else
				USART_Send_String_IT(&uart1, "State: OFF\r\n");
			break;
		default:
			/* Игнорируем \r, \n и прочее */
			break;
		}
	}
}

//uint8_t c = 48;

static void process_command(const char *cmd, uint16_t len)
{
    /* cmd не нуль-терминирован! len — реальная длина. */

    /* Уберём \r\n в конце */
	while(len > 0 && (cmd[len-1] == '\r' || cmd[len-1] == '\n'))
		len--;
	/* Сравнение без нуль-терминатора: используем strncmp с len */
	if(len == 6 && strncmp(cmd, "LED:ON", 6) == 0)
	{
		GPIOA->BSRR =  GPIO_BSRR_BR3;
		USART_Send_String_IT(&uart1, "LED is ON \r\n");
	}
	else if(len == 7 && strncmp(cmd, "LED:OFF", 7) == 0)
	{
		GPIOA->BSRR =  GPIO_BSRR_BS3;
		USART_Send_String_IT(&uart1, "LED is OFF \r\n");
	}
	else if(len == 9 && strncmp(cmd, "LED:STATE", 9) == 0)
	{
		if(GPIOA->IDR & GPIO_IDR_ID3)
			USART_Send_String_IT(&uart1, "State: OFF\r\n");
		else
			USART_Send_String_IT(&uart1, "State: ON\r\n");
	}
	else
		USART_Send_String_IT(&uart1, "ERR: unknown command\r\n");
}

uint8_t buf[32];
uint16_t pos = 0;   /* сколько байт накоплено в buf */

void control_led_by_command()
{
	 uint8_t ch;
	 while (USART_Read_Byte_IT(&uart1, &ch))
	 {
		 if (ch == '\n' || ch == '\r')
		 {
			 if(pos > 0)
			 {
				 process_command((const char *)buf, pos);
				 pos = 0; // reset
			 }
			 /* несколько \r\n подряд — просто игнорируем пустые строки */
		 }
		 else if(pos < sizeof(buf))
		 {
			 buf[pos++] = ch;
		 }
		 else
		 {
			 //overflow of string - reset and error
			 pos = 0;
			 USART_Send_String_IT(&uart1, "ERR: line too long\r\n");
		 }
	 }


}


char msg[] = "Hello DMA!\r\n";
char msg1[] = "Fuck up!\r\n";
void init_test()
{
	USART_Init(&uart1);
	DMA1_USART1_Init_Tx(&uart1, DMA_MODE_NORMAL);
	DMA1_USART1_Enable_IRQ(&uart1, 2);

	DMA1_USART1_Send(&uart1, (uint8_t*)msg, 12);
	while(uart1.tx_busy) {}
	DMA1_USART1_Send(&uart1, (uint8_t*)msg1, strlen(msg1));
}


void USART_While_Test_IT()
{
	control_led_by_command();
}
