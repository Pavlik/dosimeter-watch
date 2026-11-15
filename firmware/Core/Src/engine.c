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

char buf[32];

void configEngine()
{
	RCC->IOPENR |=
			RCC_IOPENR_GPIOAEN
			| RCC_IOPENR_GPIOBEN;

	/* GPIO CONFIG START */
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
	GPIOA->MODER &= ~GPIO_MODER_MODE4;
	GPIOA->MODER |= GPIO_MODER_MODE4_0;

	//PB6 - UART_TX
	//PB7 - UART_RX
	GPIOB->MODER &= ~GPIO_MODER_MODE6;
	GPIOB->MODER &= ~GPIO_MODER_MODE7;

	GPIOB->MODER |= GPIO_MODER_MODE6_1; // 10 = af
	GPIOB->MODER |= GPIO_MODER_MODE7_1; // 10 = af

	GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL6 | GPIO_AFRL_AFSEL7);

	/* GPIO CONFIG END */
	uart_send_init();

	/*RTC CONFIG START*/
    if (!RTC_Init_LSI(&initial_time))
    {
        // Ошибка инициализации RTC
        while (1) {}
    }

	/*RTC CONFIG END*/
}


void mainEngine()
{
    if (RTC_GetDateTime(&current_time))
    {
        // Здесь current_time содержит актуальные дату и время
    }
	snprintf(buf, sizeof(buf), "Time: %d:%d:%d\n", (uint8_t)current_time.hours, (uint8_t)current_time.minutes, (uint8_t)current_time.seconds);
	uart_send_string_it(buf);
	snprintf(buf, sizeof(buf), "Date: %d-%d-%d\n", (uint8_t)current_time.day, (uint8_t)current_time.month, (uint16_t)current_time.year);
	uart_send_string_it(buf);
	snprintf(buf, sizeof(buf), "Weekday: %d\n", (uint8_t)current_time.weekday);
	uart_send_string_it(buf);
	GPIOA->ODR ^= GPIO_ODR_OD4;
	HAL_Delay(1000);
}



void TIM16_IRQHandler()
{
	 if (TIM16->SR & TIM_SR_UIF)
	 {
		 TIM16->SR &= ~TIM_SR_UIF; // reset UIF flag
	 }

	 // code after tim UIF
}
