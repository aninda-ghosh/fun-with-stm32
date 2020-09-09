/*
 * eeprom.c
 *
 *  Created on: Sep 8, 2020
 *      Author: anind
 */

#include "eeprom.h"

volatile uint8_t userpartition_pointer;
volatile uint8_t calibpartition_pointer = 0x0006;	//Default Location
volatile uint8_t logspartition_pointer;

int8_t readEEPROMByte(const uint16_t address){
	uint8_t data[1] = {0};
	int8_t ret = IIC_Read16bitaddress(EEPROM_ADDRESS, address, 1, (uint8_t *)&data);
	if(ret < 0)
		return ret;
	else
		return data[0];
}


int8_t writeEEPROMByte(const uint16_t address, uint8_t data){
	return IIC_Write16bitaddress(EEPROM_ADDRESS, address, 1, &data);
}


