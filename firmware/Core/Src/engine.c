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

	// set time
	RTC->TR =
	    (2U << RTC_TR_HT_Pos)  |
	    (1U << RTC_TR_HU_Pos)  |
	    (3U << RTC_TR_MNT_Pos) |
	    (4U << RTC_TR_MNU_Pos) |
	    (5U << RTC_TR_ST_Pos)  |
	    (6U << RTC_TR_SU_Pos);

	//set datum
	RTC->DR =
	    (2U << RTC_DR_YT_Pos)  |
	    (6U << RTC_DR_YU_Pos)  |
	    (5U << RTC_DR_WDU_Pos) |
	    (1U << RTC_DR_MT_Pos)  |
	    (1U << RTC_DR_MU_Pos)  |
	    (1U << RTC_DR_DT_Pos)  |
	    (3U << RTC_DR_DU_Pos);

	//exit from initialisation
	RTC->ICSR &= ~RTC_ICSR_INIT;

	// disable RTC write
	RTC->WPR = 0xFF;

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
