# fun-with-stm32

Capabilities of STM32L152RCT6 with HAL Libraries.
 
Watch out for newer sensor libraries those are integrated with the STM32 

We will divide the code segments is 5 different levels
## Level 0
Basic GPIO Level implentations for understanding what is happening. 

## Level 1
Use the peripherals available in the STM MCu itself like RTC Calendar, RTC Backup Registers, ADC, DAC etc.

## Level 2
Setup of Communication protocols like I2C, SPI, USART etc. 
See commits
* 2f7d9bf Added I2C for EEPROM
* 80a6c2d Added EEPROM Partitioning


## Level 3
Use I2C and SPI to implement communications between different kinds of sensors in real life scenarios e.g. MPU9250 9DOF MPU.

## Level 4
Dive into the Low Power Modes and Touch Sensing available in the STM32L series.
See branch 
* touch-sensing for reference

Bonus - Understand the Debugging Concepts using GDB and STLink.
