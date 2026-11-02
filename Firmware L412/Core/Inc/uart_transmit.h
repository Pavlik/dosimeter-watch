/*
 * uart_tranmit.h
 *
 *  Created on: Sep 24, 2026
 *      Author: Pavel
 */

#ifndef INC_UART_TRANSMIT_H_
#define INC_UART_TRANSMIT_H_

#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

#include <string.h>
#include <stdio.h>

#include "ring_buffer.h"

void uart_send_init(void);
bool uart_send_byte_it(uint8_t byte);
void uart_send_string_it(const char *str);


#endif /* INC_UART_TRANSMIT_H_ */
