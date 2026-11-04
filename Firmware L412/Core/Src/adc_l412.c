/*
 * adc_l412.c
 *
 *  Created on: Nov 3, 2026
 *      Author: Navumchyk
 */

#include "adc_l412.h"

void ADC_SetSampleTime(uint8_t channel, Adc_Sample_cycles sample_time)
{
	ADC1->SMPR1 |= sample_time << (channel * 3UL);
}

void ADC_SelectChannel(uint8_t channel)
{
	while (ADC1->CR & ADC_CR_ADSTART);

    ADC1->SQR1 &= ~ADC_SQR1_SQ1;
    ADC1->SQR1 |= (0UL << ADC_SQR1_L_Pos) | (channel << ADC_SQR1_SQ1_Pos);
}

void configADC(void)
{
	if((RCC->AHB2ENR & RCC_AHB2ENR_GPIOAEN) != 1)
	{
		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	}

	GPIOA->MODER &= ~GPIO_MODER_MODE0;
	GPIOA->MODER |= 3UL << GPIO_MODER_MODE0_Pos; //PA0 as ADC1 CH5

	RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN; // ADC CLOCK ENABLE

	ADC1->CR &= ~ADC_CR_ADEN; // SWITCH OF ADC1
	while (ADC1->CR & ADC_CR_ADEN);

	ADC1->CR &= ~ADC_CR_DEEPPWD;

	ADC1->CR |= ADC_CR_ADVREGEN;
	for(volatile int i=0;i<1000;i++);

	ADC1->CR &= ~ADC_CR_ADCALDIF;
	ADC1->CR |= ADC_CR_ADCAL;
	while (ADC1->CR & ADC_CR_ADCAL);

	ADC_SetSampleTime(5, ADC_SAMPLE_640_5CYCLES);

	ADC1->CFGR = 0;

	ADC_SelectChannel(5);

    ADC1->ISR |= ADC_ISR_ADRDY;
    ADC1->CR |= ADC_CR_ADEN;

    while(!(ADC1->ISR & ADC_ISR_ADRDY));
}

void ADC_Start(void)
{
    ADC1->ISR |= ADC_ISR_EOC; // end of conversion flag
    ADC1->ISR |= ADC_ISR_EOS; // end of sequence flag
    ADC1->ISR |= ADC_ISR_OVR; // ADC overrun flag 0 - no overrum occurs/ 1 - overrun has occurred

    ADC1->CR |= ADC_CR_ADSTART;
}

bool ADC_IsConversionComplete(void)
{
    return (ADC1->ISR & ADC_ISR_EOC);
}

uint16_t ADC_GetValue(void)
{
    return (uint16_t)ADC1->DR;
}

uint16_t ADC_ReadChannel(uint8_t channel) // blocking method of reading channel
{
    ADC_SelectChannel(channel);

    ADC_Start();

    while(!ADC_IsConversionComplete());

    return ADC_GetValue();
}



