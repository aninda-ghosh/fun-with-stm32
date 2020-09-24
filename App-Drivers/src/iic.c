/*
 * iic.c
 *
 *  Created on: Aug 31, 2020
 *      Author: aninda
 */

#include "iic.h"

int8_t IIC_Init(void){
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 100000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 210;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		return -1;
	}
	return 0;
}

int8_t IIC_ReadRegisters(uint8_t devAddress, uint8_t subAddress, uint8_t count, uint8_t* data){
	int8_t ret;
	uint8_t memaddr[1];
	memaddr[0] = subAddress;
	ret = HAL_I2C_Master_Transmit(&hi2c1, devAddress, memaddr, sizeof(memaddr), 100);
    if(ret != HAL_OK){
    	return -1;
    }else{
    	ret = HAL_I2C_Master_Receive(&hi2c1, devAddress, data, count, 100);
		if(ret != HAL_OK){
			return -1;
		}
    }
    return 0;
}

int8_t IIC_WriteRegister(uint8_t devAddress, uint8_t subAddress, uint8_t count, uint8_t data){
    int8_t ret;
    uint8_t memaddr_data[2];
    memaddr_data[0] = subAddress;
    memaddr_data[1] = data;
    ret = HAL_I2C_Master_Transmit(&hi2c1, devAddress, memaddr_data, sizeof(memaddr_data), 100);
    if(ret != HAL_OK){
		return -1;
	}
    return 0;
}

int8_t IIC_IsDevReady(uint8_t devAddress){
	if(HAL_I2C_IsDeviceReady(&hi2c1, devAddress, 1, 1000) != HAL_OK){
		return -1;
	}
	return 0;
}


int8_t IIC_Read16bitaddress(uint8_t devAddress, uint16_t subAddress, uint8_t count, uint8_t *data){
	int8_t ret;
	uint8_t memaddr[2];
	memaddr[0] = (uint8_t) ((subAddress>>8) & 0xFF);
	memaddr[1] = (uint8_t) ((subAddress) & 0xFF);
	ret = HAL_I2C_Master_Transmit(&hi2c1, devAddress, memaddr, sizeof(memaddr), 100);
    if(ret != HAL_OK){
    	return -1;
    }else{
    	ret = HAL_I2C_Master_Receive(&hi2c1, devAddress, data, count, 1000);
		if(ret != HAL_OK){
			return -1;
		}
    }
    return 0;
}

int8_t IIC_Write16bitaddress(uint8_t devAddress, uint16_t subAddress, uint8_t count, uint8_t *data){
    int8_t ret;
    uint8_t *memaddr_data = (uint8_t*)malloc((sizeof(uint8_t)*count) + 2);
    memaddr_data[0] = (uint8_t) ((subAddress>>8) & 0xFF);
    memaddr_data[1] = (uint8_t) ((subAddress) & 0xFF);
    memcpy(memaddr_data+2, data, sizeof(uint8_t)*count);
    ret = HAL_I2C_Master_Transmit(&hi2c1, devAddress, memaddr_data, sizeof(memaddr_data), 1000);
    if(ret != HAL_OK){
		return -1;
	}
    return 0;
}
