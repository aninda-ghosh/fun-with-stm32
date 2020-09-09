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


// Format the EEPROM by writing it PageWize
int8_t formatEEPROM(void){
	int8_t ret;
	uint16_t eepromWriteAddress = 0;
	for(uint16_t x = 0; x < EEPROM_BYTES; x++){
		ret = writeEEPROMByte(eepromWriteAddress++, 0);
		if(ret<0)
			return ret;
		HAL_Delay(1);
	}
	return 0;
}

int8_t formatCalibPartition(void){
	int8_t ret;
	uint16_t eepromWriteAddress = CALIB_PARTITION_START;
	for(; eepromWriteAddress < CALIB_PARTITION_END; eepromWriteAddress++){
		ret = writeEEPROMByte(eepromWriteAddress, 0);
		if(ret<0)
			return ret;
		HAL_Delay(1);
	}
	return 0;
}

int8_t formatUserPartition(void){
	int8_t ret;
	uint16_t eepromWriteAddress = USER_PARTITION_START;
	for(; eepromWriteAddress < USER_PARTITION_END; eepromWriteAddress++){
		ret = writeEEPROMByte(eepromWriteAddress, 0);
		if(ret<0)
			return ret;
		HAL_Delay(1);
	}
	return 0;
}

int8_t formatLogsPartition(void){
	int8_t ret;
	uint16_t eepromWriteAddress = LOGS_PARTITION_START;
	for(; eepromWriteAddress < LOGS_PARTITION_END; eepromWriteAddress++){
		ret = writeEEPROMByte(eepromWriteAddress, 0);
		if(ret<0)
			return ret;
		HAL_Delay(1);
	}
	return 0;
}

int8_t writeCalibPartition(uint8_t* dataptr, uint16_t size, uint8_t typeofoperation){
	//Get the total stored data size
	uint16_t total_datasize = 0;
	total_datasize = readEEPROMByte(CALIB_PARTITION_TID_ADDR+1);
	total_datasize = (total_datasize << 8);
	total_datasize += (uint16_t)readEEPROMByte(CALIB_PARTITION_TID_ADDR);

	//Get the last stored data size
	uint16_t last_datasize = 0;
	last_datasize = readEEPROMByte(CALIB_PARTITION_LID_ADDR+1);
	last_datasize = (last_datasize << 8);
	last_datasize += (uint16_t)readEEPROMByte(CALIB_PARTITION_LID_ADDR);

	//We have to load the calibpartition_pointer from the EEPROM location
	calibpartition_pointer = readEEPROMByte(CALIB_PARTITION_POINTER_ADDR+1);
	calibpartition_pointer = (calibpartition_pointer << 8);
	calibpartition_pointer += (uint16_t)readEEPROMByte(CALIB_PARTITION_POINTER_ADDR);

	switch(typeofoperation){
		case CREATE: {
			if((total_datasize + size) > CALIB_PARTITION_END)
					return -1;

			//Next write the data to the respective location
			calibpartition_pointer += last_datasize;	//First update the location to the new head location
			int8_t ret = 0;
			for(uint8_t i=0; i<size; i++){
				ret = writeEEPROMByte(calibpartition_pointer, *dataptr);
				if(ret<0)	//If the write is not a success then return
					return -1;
				dataptr++;
				calibpartition_pointer++;
			}

			//If the data write is success we have to update three things
			//Pointer Location
			ret = writeEEPROMByte(CALIB_PARTITION_POINTER_ADDR, (uint8_t) ((calibpartition_pointer) & 0xFF));
			ret = writeEEPROMByte(CALIB_PARTITION_POINTER_ADDR+1, (uint8_t) ((calibpartition_pointer>>8) & 0xFF));

			//Last stored data size
			ret = writeEEPROMByte(CALIB_PARTITION_LID_ADDR, (uint8_t) ((size) & 0xFF));
			ret = writeEEPROMByte(CALIB_PARTITION_LID_ADDR+1, (uint8_t) ((size>>8) & 0xFF));

			//Total Stored data size
			ret = writeEEPROMByte(CALIB_PARTITION_TID_ADDR, (uint8_t) ((total_datasize+size) & 0xFF));
			ret = writeEEPROMByte(CALIB_PARTITION_TID_ADDR+1, (uint8_t) (((total_datasize+size)>>8) & 0xFF));

			break;
		}
		case UPDATE: {
			//Next write the data to the respective location
			calibpartition_pointer += last_datasize;	//First update the location to the new head location
			int8_t ret = 0;
			for(uint8_t i=0; i<size; i++){
				ret = writeEEPROMByte(calibpartition_pointer, *dataptr);
				if(ret<0)	//If the write is not a success then return
					return -1;
				dataptr++;
				calibpartition_pointer++;
			}

			//If the data write is success we have to update only
			//Last stored data size
			ret = writeEEPROMByte(CALIB_PARTITION_LID_ADDR, (uint8_t) ((size) & 0xFF));
			ret = writeEEPROMByte(CALIB_PARTITION_LID_ADDR+1, (uint8_t) ((size>>8) & 0xFF));

			break;
		}
	}
	return 0;
}

int8_t readCalibPartition(uint8_t* dataptr, uint16_t size){


	for(uint8_t i=0; i<size; i++){
		__NOP();
		dataptr++;
	}
	return 0;
}

