/* Includes ------------------------------------------------------------------*/
#include "touchsensing.h"


/* TOUCHSENSING init function */
void TouchSensing_Init(void) {
	tsl_user_Init();
}

uint8_t GetTouchSensing_Status(void) {
	return tsl_user_Exec();
}

int8_t processTouchKeys(void){

}
