/*
 * adc_l412.c
 *
 *  Created on: Nov 3, 2026
 *      Author: Navumchyk
 */

#include "adc_l412.h"

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

	ADC1->SMPR1 |= 3UL << ADC_SMPR1_SMP0_Pos;

	ADC1->CFGR = 0;

	ADC1->SQR1 = 0UL << ADC_SQR1_L_Pos | (5U << ADC_SQR1_SQ1_Pos);

    ADC1->ISR |= ADC_ISR_ADRDY;
    ADC1->CR |= ADC_CR_ADEN;

    while(!(ADC1->ISR & ADC_ISR_ADRDY));


}

