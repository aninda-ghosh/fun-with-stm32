/* USER CODE BEGIN Header */
/*
 ******************************************************************************
  * File Name          : App/tsl_user.h.h
  * Description        : Touch-Sensing user configuration.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
*/
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TSL_USER_H
#define __TSL_USER_H

#include "tsl.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

#include "stm32l1xx_hal.h"

/* Select if you use STMStudio software (0=No, 1=Yes) */
#define USE_STMSTUDIO (0)

#if USE_STMSTUDIO > 0
#include "stmCriticalSection.h"
#define STMSTUDIO_LOCK {enterLock();}
#define STMSTUDIO_UNLOCK {exitLock();}
#else
#define STMSTUDIO_LOCK
#define STMSTUDIO_UNLOCK
#endif

typedef enum
{
  TSL_USER_STATUS_BUSY       = 0, /**< The bank acquisition is on-going */
  TSL_USER_STATUS_OK_NO_ECS  = 1, /**< The bank acquisition is ok, no time for ECS */
  TSL_USER_STATUS_OK_ECS_ON  = 2, /**< The bank acquisition is ok, ECS finished */
  TSL_USER_STATUS_OK_ECS_OFF = 3  /**< The bank acquisition is ok, ECS not executed */
} tsl_user_status_t;

/* Channel IOs definition */
#define CHANNEL_0_SRC       ((uint32_t) (GR2))
#define CHANNEL_0_DEST      (0)
#define CHANNEL_0_SAMPLE    (TSL_GROUP2_IO2)
#define CHANNEL_0_CHANNEL   (TSL_GROUP2_IO1)

#define CHANNEL_1_SRC       ((uint32_t) (GR9))
#define CHANNEL_1_DEST      (1)
#define CHANNEL_1_SAMPLE    (TSL_GROUP9_IO2)
#define CHANNEL_1_CHANNEL   (TSL_GROUP9_IO1)

#define CHANNEL_2_SRC       ((uint32_t) (GR3))
#define CHANNEL_2_DEST      (2)
#define CHANNEL_2_SAMPLE    (TSL_GROUP3_IO2)
#define CHANNEL_2_CHANNEL   (TSL_GROUP3_IO1)

/* Bank(s) definition */
/* 3CH_LIN_H_NBR1 bank definition*/
// Banks definition
#define BANK_0_NBCHANNELS          (3)
#define BANK_0_INDEX               (0) // Index of 1st channel used
#define BANK_0_SHIELD_SAMPLE       (0)
#define BANK_0_SHIELD_CHANNEL      (0)

/* User Parameters */
extern CONST TSL_Bank_T MyBanks[];
extern CONST TSL_Object_T MyObjects[];
extern TSL_ObjectGroup_T MyObjGroup;
extern CONST TSL_LinRot_T MyLinRots[];

void tsl_user_Init(void);
tsl_user_status_t tsl_user_Exec(void);
void tsl_user_SetThresholds(void);

#endif /* __TSL_USER_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
