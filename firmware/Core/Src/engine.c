/*
 * engine.c
 *
 *  Created on: Oct 27, 2026
 *      Author: Pavel
 */
#include "engine.h"

static volatile uint8_t  hv_pump_end = 0;

void configEngine()
{
	/* GPIO CONFIG START */
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN
			| RCC_IOPENR_GPIOBEN;

		/*ADC1 CH1 PA7 GPIO CONFIG START*/

		 GPIOA->MODER &= ~GPIO_MODER_MODE7;  // reset
		 GPIOA->MODER |= GPIO_MODER_MODE7;   // 11 = analog

		 GPIOB->AFR[1] &= ~GPIO_AFRH_AFSEL8; 	// Reset bits
		 GPIOB->AFR[1] |= 0x5 << GPIO_AFRH_AFSEL8_Pos;   // AF5 for PB8 //
		 GPIOB->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED8;
		 GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED8;

		 /*ADC1 CH1 PA7 GPIO CONFIG END*/

		/*TIM16 CH1 PB8 GPIO CONFIG START*/

		 GPIOB->MODER &= ~GPIO_MODER_MODE8; 		// Reset bits
		 GPIOB->MODER |= GPIO_MODER_MODE8_1;   	// 10 = AF
		 GPIOB->AFR[1] &= ~GPIO_AFRH_AFSEL8; 	// Reset bits
		 GPIOB->AFR[1] |= 0x5 << GPIO_AFRH_AFSEL8_Pos;   // AF5 for PB8 //
		 GPIOB->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED8;
		 GPIOB->OSPEEDR |= GPIO_OSPEEDR_OSPEED8;

		/*TIM16 CH1 PB8 GPIO CONFIG END*/

	/* GPIO CONFIG END */

	/* ADC1 CONFIG START */
	RCC->APBENR2 |= RCC_APBENR2_ADCEN;

	ADC1->CR &= ~ADC_CR_ADEN; // disable ADC1
	while (ADC1->CR & ADC_CR_ADEN);

	ADC1->CR |= ADC_CR_ADVREGEN; // enable voltage regulator
	HAL_Delay(1);

	ADC1->CR |= ADC_CR_ADCAL; // calibrate ADC1
	while (ADC1->CR & ADC_CR_ADCAL);

	ADC1->SMPR |= ADC_SMPR_SMP1;      // 160.5 cycles = 111
	ADC1->SMPR &= ~ADC_SMPR_SMPSEL13;  // канал 13
	ADC1->SMPR &= ~ADC_SMPR_SMPSEL7;  // канал 7

	ADC1->CFGR1 = 0;

	ADC->CCR |= ADC_CCR_VREFEN; // enable internal vref
	HAL_Delay(1);

	ADC1->ISR |= ADC_ISR_EOC | ADC_ISR_EOS; // clear flags before start adc





    /* ADC1 CONFIG END */

	/* TIM16 CONFIG START */
	RCC->APBENR2 |= RCC_APBENR2_TIM16EN;

	TIM16->PSC = 0; // 24MHz - 41.67 nSec
	TIM16->ARR = 239; // 100kHz - 10 uSec / 10000 nSec
	TIM16->CCR1 = 80; // 50%

	TIM16->CCMR1 &= ~TIM_CCMR1_OC1M;
	TIM16->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // PWM mode 1

	TIM16->CCMR1 |= TIM_CCMR1_OC1PE; // preload enable

	TIM16->RCR = IMPULSE_COUNT - 1; // impulse amount in package

	TIM16->CCER |= TIM_CCER_CC1E; // enable CH1 output
	TIM16->CCER &= ~TIM_CCER_CC1P;

	TIM16->CR1 |= TIM_CR1_OPM; // one pulse mode

	TIM16->EGR = TIM_EGR_UG; // force update
	TIM16->SR &= ~TIM_SR_UIF; // clear flags

	TIM16->DIER |= TIM_DIER_UIE; // enable update interrupts
	NVIC_SetPriority(TIM16_IRQn, 1); // enable interrupts and set priority

	TIM16->CNT = 0;               // reset counter CNT
	//TIM16->CR1 |= TIM_CR1_CEN;    // Start TIM16

	/* TIM16 CONFIG END */

}

static void startPulsePacket()
{
    hv_pump_end = 0;
    TIM16->CNT = 0;
    TIM16->SR &= ~TIM_SR_UIF;
    TIM16->CR1 |= TIM_CR1_CEN;
}

void stopPump()
{
    TIM16->CR1 &= ~TIM_CR1_CEN;
    TIM16->DIER &= ~TIM_DIER_UIE; // отключить прерывание
}

static uint16_t readADC(uint8_t channel)
{
    ADC1->CHSELR = (1 << channel);
    ADC1->CR |= ADC_CR_ADSTART;
    while(!(ADC1->ISR & ADC_ISR_EOC));
    return ADC1->DR;
}

static uint32_t getVDD()
{
	uint16_t adc_vref = readADC(VREFINT_CHANNEL);
	uint16_t vref_cal = *VREFINT_CAL_ADDR;
	uint32_t vdda = 3000UL * vref_cal / adc_vref;
	return vdda; // mV
}

static uint32_t readVoltage(uint8_t channel)
{
    uint32_t vdda = getVDD();
    uint16_t adc = readADC(channel);
    uint32_t voltage = adc * vdda / 4095;
    return voltage; // mV
}




static void measureHV_Pump()
{
	ADC1->CR |= ADC_CR_ADEN;
	while (!(ADC1->ISR & ADC_ISR_ADRDY));
	uint16_t current_bat_voltage = readVoltage(BAT_ADC_CHANNEL);

}

void mainEngine()
{
	startPulsePacket();
	if(hv_pump_end == 1)
	{
		measureHV_Pump();
		hv_pump_end = 0;
	}


}



void TIM16_IRQHandler()
{
	 if (TIM16->SR & TIM_SR_UIF)
	 {
		 TIM16->SR &= ~TIM_SR_UIF; // reset UIF flag
		 hv_pump_end = 1;
	 }

	 // code after tim UIF
}
