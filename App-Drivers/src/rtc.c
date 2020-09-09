/*
 * rtc.c
 *
 *  Created on: 21-Aug-2020
 *      Author: aninda
 */

#include "rtc.h"

static unsigned int UNIXTime(int date, int month, int year, int hour, int minute, int second)
{
	struct tm t;
	time_t t_of_day;

	t.tm_year = (2000+year)-1900;  	// Year - 1900
	t.tm_mon = month-1;           	// Month, where 0 = jan
	t.tm_mday = date;          		// Day of the month
	t.tm_hour = hour;
	t.tm_min = minute;
	t.tm_sec = second;
	t.tm_isdst = 0;        			// Is DST on? 1 = yes, 0 = no, -1 = unknown
	t_of_day = mktime(&t);

	return t_of_day;
}

/**
 * @brief RTC Initialization Function
 * @param None
 * @retval None
 */
void rtc_Init(void)
{
	/** Initialize RTC Only **/
	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = 127;
	hrtc.Init.SynchPrediv = 255;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	if (HAL_RTC_Init(&hrtc) != HAL_OK)
	{
		return;
	}
}

void rtc_SetDateTime(uint8_t hours, uint8_t minutes, uint8_t seconds, uint8_t weekDay, uint8_t date, uint8_t month, uint8_t year, uint8_t weekday)
{
	RTC_TimeTypeDef sTime =
	{ 0 };
	RTC_DateTypeDef sDate =
	{ 0 };
	/** Set the Time and Date **/
	sTime.Hours = hours;
	sTime.Minutes = minutes;
	sTime.Seconds = seconds;
	sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation = RTC_STOREOPERATION_RESET;
	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		return;
	}
	sDate.WeekDay = weekday;
	sDate.Month = month;
	sDate.Date = date;
	sDate.Year = year;

	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
	{
		return;
	}
}

void rtc_GetDateTime(void)
{
	if (HAL_RTC_GetTime(&hrtc, &systemTime, RTC_FORMAT_BIN) != HAL_OK)
	{
		return;
	}
	if (HAL_RTC_GetDate(&hrtc, &systemDate, RTC_FORMAT_BIN) != HAL_OK)
	{
		return;
	}
}

unsigned int rtc_GetEpochTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time)
{
	return UNIXTime(date->Date, date->Month, date->Year, time->Hours, time->Minutes, time->Seconds);
}

void rtc_EnableWakeUp(uint8_t countervalue)
{
	/** Enable the WakeUp **/
	if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, countervalue, RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK)
	{
//		Error_Handler();
	}
}

void rtc_DisableWakeUp(void)
{
	/** Disable the WakeUp **/
	if (HAL_RTCEx_DeactivateWakeUpTimer(&hrtc) != HAL_OK)
	{
//		Error_Handler();
	}
}
