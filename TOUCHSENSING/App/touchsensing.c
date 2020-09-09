/* Includes ------------------------------------------------------------------*/
#include "touchsensing.h"


/* TOUCHSENSING init function */
void TouchSensing_Init(void) {
	tsl_user_Init();
}

uint8_t GetTouchSensing_Status(void) {
	return tsl_user_Exec();
}

/**
 * @brief  Manage the activity on sensors when touched/released (example)
 * @param  None
 * @retval None
 */

void ProcessSwipe() {
	if (MyLinRots[0].p_Data->StateId == TSL_STATEID_DETECT) {
		uint8_t status = tsl_user_Exec();
		while (status != TSL_STATUS_OK) {
		}	//Be double sure of the reading
		if (MyLinRots[0].p_Data->StateId == TSL_STATEID_DETECT) {
			if (touch_event == 0) {
				position = MyLinRots[0].p_Data->Position;
				old_position = position;
				touch_event = 1;
			}

			printf("Touch Event\r\n");
			position = MyLinRots[0].p_Data->Position;

			gradient += (position - old_position);

			if (gradient > 60) {
				swipe_event = 1;
				HAL_Delay(1000);
			} else if (gradient < -60) {
				swipe_event = 2;
				HAL_Delay(1000);
			}
			old_position = position;
		}
	} else if (MyLinRots[0].p_Data->StateId == TSL_STATEID_RELEASE) {
		// Reset all the variables
		touch_event = 0;
		gradient = 0;
		old_position = position = 0;
		MyLinRots[0].p_Data->Position = 0;
	}
}
