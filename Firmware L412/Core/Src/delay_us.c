/*
 * delay_us.c
 *
 *  Created on: Nov 10, 2026
 *      Author: Navumchyk
 */
#include "delay_us.h"

/* Включение DWT и задержка в микросекундах */
void DWT_Delay_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t cycles = us * (SystemCoreClock / 1000000U);
    while ((DWT->CYCCNT - start) < cycles);
}
