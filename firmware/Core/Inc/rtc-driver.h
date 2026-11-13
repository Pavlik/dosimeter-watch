/*
 * rtc-driver.h
 *
 *  Created on: Nov 13, 2026
 *      Author: Pavel
 */

#ifndef INC_RTC_DRIVER_H_
#define INC_RTC_DRIVER_H_

#include "stm32g0xx.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;

    uint8_t day;
    uint8_t month;
    uint16_t year; // 20xx format
    uint8_t weekday; // 1 = Monday ... 7 = Sunday

} RTC_DateTime;

/*
 * Инициализация RTC от LSI.
 *
 * initial_time используется только при первом запуске RTC,
 * когда календарь ещё не был инициализирован.
 *
 * Если RTC уже работал ранее, сохранённые дата и время
 * не перезаписываются.
 */

bool RTC_ClockInit_LSI(void);
bool RTC_SetDateTime(const RTC_DateTime *datetime);
void RTC_GetTimeDate(RTC_DateTime *datetime);

#endif /* INC_RTC_DRIVER_H_ */
