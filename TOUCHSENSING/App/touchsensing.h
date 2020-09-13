
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __touchsensing_H
#define __touchsensing_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "string.h"
#include "stdio.h"
#include "tsl_conf.h"
#include "tsl_user.h"

/* Global variables ---------------------------------------------------------*/
extern CONST TSL_LinRot_T MyLinRots[];
extern uint8_t swipe_event;

int8_t gradient;	//Can have negative value
uint32_t position;
uint32_t old_position;
//uint32_t timeoutcounter = 0;
uint8_t touch_event;


/* TOUCHSENSING init function */
void TouchSensing_Init(void);
uint8_t GetTouchSensing_Status(void);
int8_t processTouchKeys(void);

#ifdef __cplusplus
}
#endif
#endif /*__touchsensing_H */
