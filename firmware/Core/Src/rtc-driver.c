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


static bool RTC_IsLeapYear(uint16_t year)
{
    return ((year % 4U) == 0U);
}

static bool RTC_IsValidDateTime(const RTC_DateTime *dt)
{
    static const uint8_t days_in_month[] =
    {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    if (dt == 0)
        return false;

    if (dt->hours > 23U ||
        dt->minutes > 59U ||
        dt->seconds > 59U)
        return false;

    if (dt->year < 2000U || dt->year > 2099U)
        return false;

    if (dt->month < 1U || dt->month > 12U)
        return false;

    if (dt->weekday < 1U || dt->weekday > 7U)
        return false;

    uint8_t max_days = days_in_month[dt->month - 1U];

    if (dt->month == 2U && RTC_IsLeapYear(dt->year))
        max_days = 29U;

    if (dt->day < 1U || dt->day > max_days)
        return false;

    return true;
}


static void RTC_Unlock(void)
{
    RCC->APBENR1 |= RCC_APBENR1_PWREN;

    PWR->CR1 |= PWR_CR1_DBP;

    RTC->WPR = 0xCAU;
    RTC->WPR = 0x53U;
}

static void RTC_Lock(void)
{
    RTC->WPR = 0xFFU;
}

static bool RTC_ClockInit(void)
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





