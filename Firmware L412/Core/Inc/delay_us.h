/*
 * delay_us.h
 *
 *  Created on: Nov 10, 2026
 *      Author: Navumchyk
 */

#ifndef INC_DELAY_US_H_
#define INC_DELAY_US_H_

#include "stm32l412xx.h"

void DWT_Delay_Init(void);
void delay_us(uint32_t us);


#endif /* INC_DELAY_US_H_ */
