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

static bool RTC_WaitForLSI(void)
{
    uint32_t timeout = RTC_TIMEOUT;

    while ((RCC->CSR & RCC_CSR_LSIRDY) == 0U)
    {
        if (timeout-- == 0U)
        {
            return false;
        }
    }
    return true;
}

static bool RTC_ClockInit(void)
{
    RCC->APBENR1 |= RCC_APBENR1_PWREN; // enable PWR clock
    RCC->APBENR1 |= RCC_APBENR1_RTCAPBEN; // enable RTC clock
    PWR->CR1 |= PWR_CR1_DBP; // enable access to Backup Domain
    RCC->CSR |= RCC_CSR_LSION; //LSI on
    //while ((RCC->CSR & RCC_CSR_LSIRDY) == 0U){} // waiting for LSI ready
    if (!RTC_WaitForLSI())
    {
        return false;
    }

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

static void RTC_EnterInitMode(void)
{
	RTC_Unlock();

    RTC->ICSR |= RTC_ICSR_INIT;

    while ((RTC->ICSR & RTC_ICSR_INITF) == 0U)
    {
        // Ожидаем входа в режим инициализации
    }
}

static void RTC_ExitInitMode(void)
{
    RTC->ICSR &= ~RTC_ICSR_INIT;

    RTC_Lock();

    // Ждём обновления теневых регистров
    while ((RTC->ICSR & RTC_ICSR_RSF) == 0U)
    {
    }
}

static void RTC_WriteCalendar(const RTC_DateTime *dt)
{
    // Время
    RTC->TR =
        ((dt->hours / 10U)   << RTC_TR_HT_Pos)  |
        ((dt->hours % 10U)   << RTC_TR_HU_Pos)  |
        ((dt->minutes / 10U) << RTC_TR_MNT_Pos) |
        ((dt->minutes % 10U) << RTC_TR_MNU_Pos) |
        ((dt->seconds / 10U) << RTC_TR_ST_Pos)  |
        ((dt->seconds % 10U) << RTC_TR_SU_Pos);

    // Дата
    RTC->DR =
        (((dt->year % 100U) / 10U) << RTC_DR_YT_Pos)  |
        ((dt->year % 10U)          << RTC_DR_YU_Pos)  |
        (dt->weekday              << RTC_DR_WDU_Pos) |
        ((dt->month / 10U)         << RTC_DR_MT_Pos)  |
        ((dt->month % 10U)         << RTC_DR_MU_Pos)  |
        ((dt->day / 10U)           << RTC_DR_DT_Pos)  |
        ((dt->day % 10U)           << RTC_DR_DU_Pos);
}


bool RTC_Init_LSI(const RTC_DateTime *initial_time)
{
    if (!RTC_ClockInit())
        return false;

    if ((RTC->ICSR & RTC_ICSR_INITS) == 0U)
    {
        if (!RTC_IsValidDateTime(initial_time))
            return false;

        RTC_EnterInitMode();

        // Асинхронный и синхронный предделители
        RTC->PRER =
            (127U << RTC_PRER_PREDIV_A_Pos) |
            (249U << RTC_PRER_PREDIV_S_Pos);

        // 24-часовой формат
        RTC->CR &= ~RTC_CR_FMT; //!!

        RTC_WriteCalendar(initial_time);

        RTC_ExitInitMode();
    }
    else
    {
        // После системного сброса теневые регистры
        // нужно синхронизировать с календарём.
        while ((RTC->ICSR & RTC_ICSR_RSF) == 0U)
        {
        }
    }

    return true;
}


bool RTC_SetDateTime(const RTC_DateTime *datetime)
{
    if (!RTC_IsValidDateTime(datetime))
        return false;

    RTC_EnterInitMode();

    RTC_WriteCalendar(datetime);

    RTC_ExitInitMode();

    return true;
}


bool RTC_GetDateTime(RTC_DateTime *datetime)
{
    if (datetime == 0)
        return false;

    // Убеждаемся, что теневые регистры синхронизированы
    if ((RTC->ICSR & RTC_ICSR_RSF) == 0U)
        return false;

    // Важно: сначала TR, затем DR
    uint32_t tr = RTC->TR;
    uint32_t dr = RTC->DR;

    // Время
    datetime->hours =
        RTC_Decode2Digits(tr,
                          RTC_TR_HT_Pos, 0x03U,
                          RTC_TR_HU_Pos);

    datetime->minutes =
        RTC_Decode2Digits(tr,
                          RTC_TR_MNT_Pos, 0x07U,
                          RTC_TR_MNU_Pos);

    datetime->seconds =
        RTC_Decode2Digits(tr,
                          RTC_TR_ST_Pos, 0x07U,
                          RTC_TR_SU_Pos);

    // Дата
    datetime->day =
        RTC_Decode2Digits(dr,
                          RTC_DR_DT_Pos, 0x03U,
                          RTC_DR_DU_Pos);

    datetime->month =
        RTC_Decode2Digits(dr,
                          RTC_DR_MT_Pos, 0x01U,
                          RTC_DR_MU_Pos);

    datetime->year =
        2000U + RTC_Decode2Digits(dr,
                                  RTC_DR_YT_Pos, 0x0FU,
                                  RTC_DR_YU_Pos);

    datetime->weekday =
        (dr >> RTC_DR_WDU_Pos) & 0x07U;

    return true;
}




//static uint8_t BCD_ToDecimal(uint8_t bcd)
//{
//    return ((bcd >> 4) * 10U) + (bcd & 0x0FU);
//}
//
//void RTC_GetTimeDate(RTC_DateTime *datetime)
//{
//
//	while ((RTC->ICSR & RTC_ICSR_RSF) == 0U){} // after exit from enit
//
//    uint32_t tr = RTC->TR;
//    uint32_t dr = RTC->DR;
//
//    datetime->hours = BCD_ToDecimal((tr >> RTC_TR_HU_Pos) & 0x3FU); // Hours
//    datetime->minutes = BCD_ToDecimal((tr >> RTC_TR_MNU_Pos) & 0x7FU); // minutes
//    datetime->seconds = BCD_ToDecimal((tr >> RTC_TR_SU_Pos) & 0x7FU); // seconds
//
//    datetime->day = BCD_ToDecimal((dr >> RTC_DR_DU_Pos) & 0x3FU); // day
//    datetime->month = BCD_ToDecimal((dr >> RTC_DR_MU_Pos) & 0x1FU); // month
//    datetime->year = BCD_ToDecimal((dr >> RTC_DR_YU_Pos) & 0xFFU); // year
//    datetime->weekday = (dr >> RTC_DR_WDU_Pos) & 0x07U; // weekdays - not BCD!
//}

