/*
 * adc_l412.h
 *
 *  Created on: Nov 3, 2026
 *      Author: Navumchyk
 */

#ifndef INC_ADC_L412_H_
#define INC_ADC_L412_H_

#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"

void configADC(void);

uint16_t ADC_ReadChannel(uint8_t channel);




#endif /* INC_ADC_L412_H_ */
