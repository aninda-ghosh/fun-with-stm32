/*
 * eeprom.h
 *
 *  Created on: Sep 8, 2020
 *      Author: aninda
 */

#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_

#include "iic.h"

#define EEPROM_ADDRESS (0x57<<1)
// 7 Bit address, the first 4 bits are 1010, the the last 3 bits according to A2, A1 and A0
// On the common ZS-042 board, this corresponds to (where x is jumper open, and 1 is jumper closed)
// A0    A1    A2
//  x    x     x    0x57  (default)

#define EEPROM_SIZE_KBIT 32768  // EEPROMs are sized in kilobit
#define EEPROM_PAGE_SIZE 32 // And have a number of bytes per page
#define EEPROM_BYTES EEPROM_SIZE_KBIT/8
#define EEPROM_PAGES EEPROM_BYTES/EEPROM_PAGE_SIZE

/**
 * Memory structure in EEPROM
 * 32 Kb
 * 32/8 = 4KB Space
 *
 * Application storage facility
 *
 * Partitioning
 * 512  Bytes - Calibration and Config Variables (0x0000 - 0x0200)
 * 		2Bytes				2Bytes 							2Bytes 							Rest
 * 		Pointer address		Length of Last inserted Data	Length of total inserted data 	Actual Calibration data
 * 128  Bytes - User Specific Config Variables   (0x0201 - 0x0280)
 * 3456 Bytes - Logs							 (0x0281 - 0x1000)
 */



int8_t readEEPROMByte(const uint16_t address);
int8_t writeEEPROMByte(const uint16_t address, uint8_t data);




#endif /* INC_EEPROM_H_ */
