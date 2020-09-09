/*
 * rtc.h
 *
 *  Created on: 21-Aug-2020
 *      Author: aninda
 */

#ifndef __RTC_H__
#define __RTC_H__

#include <time.h>
#include "stm32l1xx_hal.h"

RTC_HandleTypeDef hrtc;
RTC_DateTypeDef systemDate;
RTC_TimeTypeDef systemTime;

void rtc_Init(void);
void rtc_SetDateTime(uint8_t hours, uint8_t minutes, uint8_t seconds, uint8_t weekDay, uint8_t date, uint8_t month, uint8_t year, uint8_t weekday);
void rtc_GetDateTime(void);
unsigned int rtc_GetEpochTime(RTC_DateTypeDef *date, RTC_TimeTypeDef* time);

void rtc_EnableWakeUp(uint8_t countervalue);
void rtc_DisableWakeUp(void);

#endif /* INC_RTC_H_ */
