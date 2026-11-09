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

#define ADC_DMA_AMOUNT_TRANSFER

typedef enum
{
    ADC_MODE_POLLING = 0,
    ADC_MODE_DMA_NORMAL,
    ADC_MODE_DMA_CIRCULAR

} ADC_Mode_t;


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
    ADC_CH16,
	ADC_CH17,

	ADC_CH_VREFINT = ADC_CH0,
	ADC_CH_TEMP = ADC_CH17
} Adc_Channel;

typedef enum
{
    ADC_TRIGGER_SOFTWARE = 0,
    ADC_TRIGGER_TIM15_TRGO
	// ADC_TRIGGER_EXTI11 ... other trigger sources
} ADC_Trigger_t;

typedef enum
{
    ADC_READ_POLLING,
    ADC_READ_DMA,
    ADC_READ_DMA_CIRCULAR
} ADC_ReadMode;

typedef enum
{
	DMA_NORMAL_MODE  = 0,
	DMA_CIRCULAR_MODE
}DMA_Mode;

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

void ADC_Init(ADC_Mode_t mode);
void ADC_EnableVrefInt();
void ADC_EnableTempSensor();

void ADC_SetTrigger(ADC_Trigger_t trigger);
void ADC_Enable();
void ADC_Disable();

void ADC_Start(void);
void ADC_Stop(void);

void ADC_SetSampleTime(uint8_t channel, Adc_Sample_cycles sample_time);
void ADC_SelectChannel(uint8_t channel);
void ADC_SetSequenceLength(uint8_t len);
void ADC_SetSequenceChannel(uint8_t rank, uint8_t channel);


bool ADC_IsConversionComplete(void);
uint16_t ADC_GetValue(void);
bool ADC_IsBusy(void);

void ADC_DMA_Enable();
void ADC_DMA_Disable();
void ADC_DMA_EnableIRQ(uint8_t priority);
void ADC_DMA_DisableIRQ();

void ADC_DMA_Config(uint16_t *buffer, uint16_t length, DMA_Mode mode);
uint16_t* ADC_DMA_Read_Sequence();
void ADC_Start_DMA_Conversion(uint16_t length);
uint8_t ADC_DMA_IsReady();

uint16_t ADC_ReadChannel(uint8_t channel);

bool ADC_WaitConversionComplete(uint32_t timeout);



#endif /* INC_ADC_L412_H_ */
