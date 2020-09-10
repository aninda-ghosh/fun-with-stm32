/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "main.h"
#include "sysclock.h"
#include "rtc.h"
#include "iic.h"
#include "eeprom.h"


typedef struct __attribute__ ((packed)) calibData{
	float a;	//4
	float b; 	//4
	float c; 	//4
}CALIB_DATA;

CALIB_DATA cd = {1.1, 2.2, 3.3};

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	rtc_Init();
	rtc_SetDateTime(17, 03, 30, 1, 24, RTC_MONTH_AUGUST, 20, RTC_WEEKDAY_MONDAY);
	// Initialize the IIC peripheral
	IIC_Init();


	while(IIC_IsDevReady(0x57<<1) != 0){
 		__NOP();
	}

	formatCalibPartition();

//	//Write 2 first 32 bytes of data
//	uint16_t startAddress = 0;
//	uint8_t writedata = 0;
//	for( ;startAddress < 32; startAddress++){
//		if(writedata > 128)
//			writedata = 0;
//		writeEEPROMByte(startAddress, writedata++);
//		HAL_Delay(1);
//	}



	writeCalibPartition((uint8_t*)&cd,sizeof(CALIB_DATA), CREATE);

	peekPartition(CALIB_PARTITION);


	/* Infinite loop */
	while (1) {

//		rtc_GetDateTime();		//Refresh the Time Stamp from RTC
//		unsigned int unixTimeStamp = rtc_GetEpochTime(&systemDate, &systemTime);
//		__NOP();
//		UNUSED(unixTimeStamp);

		HAL_Delay(1000);
	}
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {

}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
