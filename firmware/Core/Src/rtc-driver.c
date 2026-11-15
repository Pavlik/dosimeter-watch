/*
 * rtc-driver.c
 *
 *  Created on: Nov 13, 2026
 *      Author: Pavel
 */
#include "rtc-driver.h"

static bool RTC_WaitForBits(volatile uint32_t *reg, uint32_t mask, uint32_t expected)
{
	uint32_t timeout = RTC_TIMEOUT;
    while ((*reg & mask) != expected)
    {
        if (timeout == 0U)
        {
            return false;
        }

        timeout--;
    }

    return true;
}

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

static RTC_Status RTC_ClockInit(void)
{
    RCC->APBENR1 |= RCC_APBENR1_PWREN; // enable PWR clock
    RCC->APBENR1 |= RCC_APBENR1_RTCAPBEN; // enable RTC clock
    PWR->CR1 |= PWR_CR1_DBP; // enable access to Backup Domain

    //while ((RCC->CSR & RCC_CSR_LSIRDY) == 0U){} // waiting for LSI ready

    // checking source of RTC clock
    uint32_t rtc_source = RCC->BDCR & RCC_BDCR_RTCSEL;

    if(rtc_source != 0 && rtc_source != (2U << RCC_BDCR_RTCSEL_Pos))
    	return RTC_ERROR_CLOCK;

    RCC->CSR |= RCC_CSR_LSION; //LSI on
    if (!RTC_WaitForBits(&RCC->CSR, RCC_CSR_LSIRDY, RCC_CSR_LSIRDY))
        return RTC_ERROR_CLOCK;

    if (rtc_source == 0U)
    {
    	// choose LSI
        RCC->BDCR &= ~RCC_BDCR_RTCSEL;
        RCC->BDCR |= (2U << RCC_BDCR_RTCSEL_Pos);
    }

    RCC->BDCR |= RCC_BDCR_RTCEN; // enable RTC

    return RTC_OK;
}

static RTC_Status RTC_EnterInitMode(void)
{
	RTC_Unlock();

    RTC->ICSR |= RTC_ICSR_INIT;

    if(!RTC_WaitForBits(&RTC->ICSR, RTC_ICSR_INITF, RTC_ICSR_INITF)) // Ожидаем входа в режим инициализации
    {
    	RTC_Lock();
    	return RTC_ERROR_INIT_MODE;
    }
    return RTC_OK;
}

/*Ещё один нюанс: этот подход предполагает, что BYPSHAD = 0,
 * то есть теневые регистры не отключены.
 * При BYPSHAD = 1 ждать установки RSF не нужно — этот флаг аппаратно удерживается в нуле.
 */
//static bool RTC_ExitInitMode(void)
//{
//    RTC->ICSR &= ~RTC_ICSR_INIT;
//
//    RTC_Lock();
//
//    // Ждём обновления теневых регистров
//    if(!RTC_WaitForBits(&RTC->ICSR, RTC_ICSR_RSF, RTC_ICSR_RSF))
//    	return false;
//    return true;
//}
static RTC_Status RTC_WaitForSync(void)
{
	if(!RTC_WaitForBits(&RTC->ICSR,
                           RTC_ICSR_RSF,
                           RTC_ICSR_RSF))
	{
		return RTC_ERROR_SYNC_TIMEOUT;
	}
	return RTC_OK;
}
static void RTC_ExitInitMode(void)
{
    RTC->ICSR &= ~RTC_ICSR_INIT;
    RTC_Lock();
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


RTC_Status RTC_Init_LSI(const RTC_DateTime *initial_time)
{
	RTC_Status status = RTC_ClockInit();

    if (status != RTC_OK)
        return status;

    if ((RTC->ICSR & RTC_ICSR_INITS) == 0U)
    {
        if (!RTC_IsValidDateTime(initial_time))
            return RTC_ERROR_INVALID_ARG;

        status = RTC_EnterInitMode();
        if (status != RTC_OK)
             return status;

        // Асинхронный и синхронный предделители
        RTC->PRER =
            (127U << RTC_PRER_PREDIV_A_Pos) |
            (249U << RTC_PRER_PREDIV_S_Pos);

        // 24-часовой формат
        RTC->CR &= ~RTC_CR_FMT; //!!

        RTC_WriteCalendar(initial_time);

        RTC_ExitInitMode();
        status = RTC_WaitForSync();
        if (status != RTC_OK)
            return status;
    }
    else
    {
        // После системного сброса теневые регистры
        // нужно синхронизировать с календарём
    	status = RTC_WaitForSync();
        if (status != RTC_OK)
            return status;
    }

    return RTC_OK;
}


RTC_Status RTC_SetDateTime(const RTC_DateTime *datetime)
{
    if (!RTC_IsValidDateTime(datetime))
        return RTC_ERROR_INVALID_ARG;

    RTC_Status status = RTC_EnterInitMode();
    if (status != RTC_OK)
        return status;

    RTC_WriteCalendar(datetime);

    RTC_ExitInitMode();
    status = RTC_WaitForSync();

    if (status != RTC_OK)
        return status;

    return RTC_OK;
}


RTC_Status RTC_GetDateTime(RTC_DateTime *datetime)
{
    if (datetime == 0)
        return RTC_ERROR_INVALID_ARG;

    // Убеждаемся, что теневые регистры синхронизированы
    RTC_Status status = RTC_WaitForSync();

    if (status != RTC_OK)
        return status;

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

    return RTC_OK;
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

