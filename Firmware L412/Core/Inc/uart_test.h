/*
 * uart_test.h
 *
 *  Created on: Nov 21, 2026
 *      Author: Pavel
 */

#ifndef INC_UART_TEST_H_
#define INC_UART_TEST_H_

#define UART_TIMEOUT_MS(ms)  ((uint32_t)(ms) * (SystemCoreClock / 1000U))

#include "uart_driver.h"

void USART_Init_Test();
void USART_While_Test();

void USART_Init_Test_IT();


#endif /* INC_UART_TEST_H_ */
