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
#include "stdbool.h"

#define ADC_DMA_AMOUNT_TRANSFER 1

typedef enum
{
    ADC_CH0 = 0,
    ADC_CH1,
    ADC_CH2,
    ADC_CH3,
    ADC_CH4,
    ADC_CH5,
    ADC_CH6,
    ADC_CH7,
    ADC_CH8,
    ADC_CH9,
    ADC_CH10,
    ADC_CH11,
    ADC_CH12,
    ADC_CH13,
    ADC_CH14,
    ADC_CH15,
    ADC_CH16
} Adc_Channel;


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

void ADC_SetSampleTime(uint8_t channel, Adc_Sample_cycles sample_time);
void ADC_SelectChannel(uint8_t channel);
void ADC_SetSequenceLength(uint8_t len);
void ADC_SetSequenceChannel(uint8_t rank, uint8_t channel);

void ADC_Start(void);
void ADC_Stop(void);
bool ADC_IsConversionComplete(void);
uint16_t ADC_GetValue(void);
bool ADC_IsBusy(void);

void ADC_Init(void);

void ADC1_DMA_Enable();
void ADC1_DMA_Disable();
void ADC_DMA_Config_non_circular_mode();
uint16_t ADC_Get_DMA_Value(void);


uint16_t ADC_ReadChannel(uint8_t channel);



#endif /* INC_ADC_L412_H_ */
