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

