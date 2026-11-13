/*
 * engine.c
 *
 *  Created on: Oct 27, 2026
 *      Author: Pavel
 */
#include "engine.h"

void configEngine()
{
	/* GPIO CONFIG START */
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	GPIOA->MODER &= ~GPIO_MODER_MODE4;
	GPIOA->MODER |= GPIO_MODER_MODE4_0;
	/* GPIO CONFIG END */

	/*RTC CONFIG START*/

	if (!RTC_ClockInit_LSI())
	{
	    while (1)
	    {
	    }
	}

	// enable write in RTC
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;

	RTC->ICSR |= RTC_ICSR_INIT; // enter to initialisation
	while ((RTC->ICSR & RTC_ICSR_INITF) == 0U){}

	//predivider config
	RTC->PRER =
	    (127U << RTC_PRER_PREDIV_A_Pos) |
	    (249U << RTC_PRER_PREDIV_S_Pos);

	/*RTC CONFIG END*/

}

void mainEngine()
{
	GPIOA->ODR ^= GPIO_ODR_OD4;
	HAL_Delay(500);
}



void TIM16_IRQHandler()
{
	 if (TIM16->SR & TIM_SR_UIF)
	 {
		 TIM16->SR &= ~TIM_SR_UIF; // reset UIF flag
	 }

	 // code after tim UIF
}
