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


typedef enum
{
	ADC_SAMPLE_2_5CYCLES = 0,
	ADC_SAMPLE_6_5CYCLES,
	ADC_SAMPLE_12_5CYCLES,
	ADC_SAMPLE_24_5CYCLES,
	ADC_SAMPLE_47_5CYCLES,
	ADC_SAMPLE_92_5CYCLES,
	ADC_SAMPLE_247_5CYCLES,
	ADC_SAMPLE_640_5CYCLES
}Adc_Sample_cycles;

void configADC(void);

uint16_t ADC_ReadChannel(uint8_t channel);
void ADC_SetSampleTime(uint8_t channel, uint8_t sample_time);



#endif /* INC_ADC_L412_H_ */
