/*
 * engine.c
 *
 *  Created on: Oct 27, 2026
 *      Author: Pavel
 */
#include "engine.h"

	RTC_DateTime initial_time =
	{
	    .hours   = 21,
	    .minutes = 30,
	    .seconds = 0,

	    .day     = 14,
	    .month   = 11,
	    .year    = 2026,

	    .weekday = 6
	};

	RTC_DateTime current_time;

void configEngine()
{
	/* GPIO CONFIG START */
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	GPIOA->MODER &= ~GPIO_MODER_MODE4;
	GPIOA->MODER |= GPIO_MODER_MODE4_0;
	/* GPIO CONFIG END */

	/*RTC CONFIG START*/
    if (!RTC_Init_LSI(&initial_time))
    {
        // Ошибка инициализации RTC
        while (1) {}
    }


	/*RTC CONFIG END*/

}

uint8_t set_time_flag = 1;
RTC_DateTime new_time =
{
    .hours   = 1,
    .minutes = 30,
    .seconds = 0,

    .day     = 15,
    .month   = 11,
    .year    = 2026,

    .weekday = 7
};

void mainEngine()
{
    if (RTC_GetDateTime(&current_time))
    {
        // Здесь current_time содержит актуальные дату и время
    }
	GPIOA->ODR ^= GPIO_ODR_OD4;
	HAL_Delay(1000);
	if(set_time_flag == 1)
	{
		set_time_flag = 0;
		RTC_SetDateTime(&new_time);
	}
}



void TIM16_IRQHandler()
{
	 if (TIM16->SR & TIM_SR_UIF)
	 {
		 TIM16->SR &= ~TIM_SR_UIF; // reset UIF flag
	 }

	 // code after tim UIF
}
