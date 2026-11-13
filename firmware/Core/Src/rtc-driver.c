/*
 * rtc-driver.c
 *
 *  Created on: Nov 13, 2026
 *      Author: Pavel
 */
#include "rtc-driver.h"

static uint8_t RTC_Decode2Digits(uint32_t reg,
                                 uint32_t tens_pos,
                                 uint32_t tens_mask,
                                 uint32_t units_pos)
{
    uint8_t tens  = (reg >> tens_pos) & tens_mask;
    uint8_t units = (reg >> units_pos) & 0x0FU;

    return tens * 10U + units;
}


bool RTC_ClockInit_LSI(void)
{
    RCC->APBENR1 |= RCC_APBENR1_PWREN; // enable PWR clock
    PWR->CR1 |= PWR_CR1_DBP; // enable access to Backup Domain
    RCC->CSR |= RCC_CSR_LSION; //LSI on
    while ((RCC->CSR & RCC_CSR_LSIRDY) == 0U){} // waiting for LSI ready

    // checking source of RTC clock
    uint32_t rtc_source = RCC->BDCR & RCC_BDCR_RTCSEL;
    if (rtc_source == 0U)
    {
    	// choose LSI
        RCC->BDCR &= ~RCC_BDCR_RTCSEL;
        RCC->BDCR |= (2U << RCC_BDCR_RTCSEL_Pos);
    }
    else if (rtc_source != (2U << RCC_BDCR_RTCSEL_Pos))
    {
    	// another source already have choose
        return false;
    }

    RCC->BDCR |= RCC_BDCR_RTCEN; // enable RTC

    return true;
}

static uint8_t BCD_ToDecimal(uint8_t bcd)
{
    return ((bcd >> 4) * 10U) + (bcd & 0x0FU);
}

void RTC_GetTimeDate(RTC_DateTime *datetime)
{

	while ((RTC->ICSR & RTC_ICSR_RSF) == 0U){} // after exit from enit

    uint32_t tr = RTC->TR;
    uint32_t dr = RTC->DR;

    datetime->hours = BCD_ToDecimal((tr >> RTC_TR_HU_Pos) & 0x3FU); // Hours
    datetime->minutes = BCD_ToDecimal((tr >> RTC_TR_MNU_Pos) & 0x7FU); // minutes
    datetime->seconds = BCD_ToDecimal((tr >> RTC_TR_SU_Pos) & 0x7FU); // seconds

    datetime->day = BCD_ToDecimal((dr >> RTC_DR_DU_Pos) & 0x3FU); // day
    datetime->month = BCD_ToDecimal((dr >> RTC_DR_MU_Pos) & 0x1FU); // month
    datetime->year = BCD_ToDecimal((dr >> RTC_DR_YU_Pos) & 0xFFU); // year
    datetime->weekday = (dr >> RTC_DR_WDU_Pos) & 0x07U; // weekdays - not BCD!
}

