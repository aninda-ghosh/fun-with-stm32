/*
 * iic.h
 *
 *  Created on: Aug 31, 2020
 *      Author: aninda
 */

#ifndef INC_IIC_H_
#define INC_IIC_H_

#include "stm32l1xx_hal.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

I2C_HandleTypeDef hi2c1;

int8_t IIC_Init(void);
int8_t IIC_ReadRegisters(uint8_t devAddress, uint8_t subAddress, uint8_t count, uint8_t* data);
int8_t IIC_WriteRegister(uint8_t devAddress, uint8_t subAddress, uint8_t count, uint8_t data);
int8_t IIC_IsDevReady(uint8_t devAddress);

int8_t IIC_Read16bitaddress(uint8_t devAddress, uint16_t subAddress, uint8_t count, uint8_t* data);
int8_t IIC_Write16bitaddress(uint8_t devAddress, uint16_t subAddress, uint8_t count, uint8_t *data);


#endif /* INC_IIC_H_ */
