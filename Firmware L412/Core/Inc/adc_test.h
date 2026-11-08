/*
 * test.h
 *
 *  Created on: Nov 8, 2026
 *      Author: Pavel
 */

#ifndef INC_TEST_H_
#define INC_TEST_H_
#include "uart_transmit.h"
#include "adc_l412.h"
#include "adc_measurment.h"

void ADC_Test_Polling_Init();
void ADC_TEST_DMA_Circular_TRGO_Init();
void ADC_Test_Polling_While();
void ADC_Test_DMA_Circular_TRGO_While();

#endif /* INC_TEST_H_ */
