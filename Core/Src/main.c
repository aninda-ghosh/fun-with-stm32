/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "main.h"
#include "sysclock.h"
#include "iic.h"
#include "mpu.h"
#include "quaternion.h"


float angles[3] = {0};

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	MPU_Init();
	while(MPU_whoAmI() != 0x71){
		__NOP();
	}
	madwickBegin();



	/* Infinite loop */
	while (1) {

		MPU_readSensor();
		madwickUpdate(_gyroscope_val[0], _gyroscope_val[1], _gyroscope_val[2], _accelerometer_val[0], _accelerometer_val[1], _accelerometer_val[2], _magnetometer_val[0], _magnetometer_val[1], _magnetometer_val[2]);
		madwickGetAngles(angles);
		__NOP();

		HAL_Delay(10);
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
