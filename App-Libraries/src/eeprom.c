/*
 * eeprom.c
 *
 *  Created on: Sep 8, 2020
 *      Author: anind
 */

#include "eeprom.h"

uint8_t DS3231_Simple::formatEEPROM()
{
	eepromWriteAddress = 0;
	writeBytePagewizeStart();
	for(uint16_t x = 0; x < EEPROM_BYTES; x++)
	{
		writeBytePagewize(0);
	}
	writeBytePagewizeEnd();

	eepromWriteAddress = 0;
	eepromReadAddress = 0;
	return 1;
}

uint8_t DS3231_Simple::readEEPROMByte(const uint16_t address)
{
	uint8_t b = 0;
	Wire.beginTransmission(EEPROM_ADDRESS); // DUMMY WRITE
	Wire.write((uint8_t) ((address>>8) & 0xFF));
	Wire.write((uint8_t) ((address) & 0xFF));

	if(Wire.endTransmission(false))// Do not send STOP, just restart
	{
		return 0;
	}

	if(Wire.requestFrom(EEPROM_ADDRESS, (uint8_t) 1))
	{
		b = Wire.read();
	}

	Wire.endTransmission(); // Now send STOP

	return b;
}

// Locate the NEXT place to store a block
uint16_t DS3231_Simple::findEEPROMWriteAddress()
{
	uint8_t t = 0;

	for(eepromWriteAddress = 0; eepromWriteAddress < EEPROM_BYTES; )
	{
		t = readEEPROMByte(eepromWriteAddress);

		// If the byte read is a zero, then this is the top of the stack.
		if(t == 0) break;

		// If not zero, then this must be a start byte for the block (we will assert that
		// blocks are always aligned to byte zero of the EEPROM, there is no "wrapping"
		// of a block starting at the top of the EEPROM and finishing in the bottom
		// so we will add the length of this block to x

		// The upper 3 bits store the number of data bytes
		// plus 5 header bytes
		eepromWriteAddress = eepromWriteAddress + (t >> 5) + 5;
	}

	// If we have filled up as much as we can... reset back to the bottom as the stack top.
	if(eepromWriteAddress >= EEPROM_BYTES-5)
	{
		eepromWriteAddress = 0;
	}

	return eepromWriteAddress;
}

// Locate the NEXT block to read from
uint16_t DS3231_Simple::findEEPROMReadAddress()
{
	// This is going to be really memory hungry :-/
	// Anybody care to think of a better way.
	uint16_t nxtPtr, x = 0;

	DateTime currentOldest;
	DateTime compareWith;
	currentOldest.Year = 255;// An invalid year the highest we can go so that any valid log is older.

	// Find the oldest block, that is the bottom
	for(x = 0; x < EEPROM_BYTES; )
	{
		if(readEEPROMByte(x) == 0) {x++; continue;}

		// readLogFrom will return the address of the next log entry if any
		// or EEPROM_BYTES if not.

		nxtPtr = readLogFrom(x, compareWith, 0, 0);
		if(compareTimestamps(currentOldest,compareWith) > 0)
		{
			currentOldest = compareWith;
			eepromReadAddress = x;
		}

		if(nxtPtr > x)
		{
			x = nxtPtr;
		}
		else
		{
			break; // no more entries
		}
	}

	return eepromReadAddress;
}

// Clear some space int he EEPROM to record BytesRequired bytes, nulls
//  any overlappig blocks.
uint8_t DS3231_Simple::makeEEPROMSpace(uint16_t Address, int8_t BytesRequired)
{
	if((Address+BytesRequired) >= EEPROM_BYTES)
	{
		return 0;  // No can do.
	}
	uint8_t x;
	while(BytesRequired > 0)
	{
		x = readEEPROMByte(Address);
		if(x == 0) // Already blank
		{
			BytesRequired--;
			Address++;
			continue;
		}
		else
		{
			uint16_t oldEepromWriteAddress = eepromWriteAddress;
			eepromWriteAddress = Address;

			writeBytePagewizeStart();
			for(x = ((x>>5) + 5); x > 0; x-- )
			{
				writeBytePagewize(0);
			}
			writeBytePagewizeEnd();
			eepromWriteAddress = oldEepromWriteAddress;
		}
	}

	return 1;
}

uint8_t DS3231_Simple::writeBytePagewizeStart()
{
	Wire.beginTransmission(EEPROM_ADDRESS);
	Wire.write((eepromWriteAddress >> 8) & 0xFF);
	Wire.write(eepromWriteAddress & 0xFF);
	return 1;
}

uint8_t DS3231_Simple::writeBytePagewize(const uint8_t data)
{
	Wire.write(data);

	// Because of the 32 byte buffer limitation in Wire, we are
	//  using 4 bits as the page size for a page of 16 bytes
	//  even though the actual page size is probably higher
	//  (it needs to be a binary multiple for this to work).
	eepromWriteAddress++;

	if(eepromWriteAddress < EEPROM_BYTES && ((eepromWriteAddress >>4) & 0xFF) != (((eepromWriteAddress-1)>>4) & 0xFF))
	{
		// This is a new page, finish the previous write and start a new one
		writeBytePagewizeEnd();
		writeBytePagewizeStart();
	}

	return 1;
}

uint8_t DS3231_Simple::writeBytePagewizeEnd()
{
	if(Wire.endTransmission() > 0)
	{
		// Failure
		return 0;
	}

	// Poll for write to complete
	while(!Wire.requestFrom(EEPROM_ADDRESS,(uint8_t) 1));
	return 1;
}

uint8_t DS3231_Simple::writeLog( const DateTime &timestamp, const uint8_t *data, uint8_t size )
{
	if(size > 7) return 0; // Limit is 7 data bytes.

	if(eepromWriteAddress >= EEPROM_BYTES) findEEPROMWriteAddress();// Uninitialized stack top, find it.
	if((eepromWriteAddress + 5 + size) >= EEPROM_BYTES) eepromWriteAddress = 0;// Would overflow so wrap to start

	if(!makeEEPROMSpace(eepromWriteAddress, 5+size))
	{
		return 0;
	}

	writeBytePagewizeStart();
	writeBytePagewize((size<<5) | (timestamp.Dow<<2) | (timestamp.Year >> 6));
	writeBytePagewize((timestamp.Year<<2) | (timestamp.Month >> 2));
	writeBytePagewize((timestamp.Month<<6) | (timestamp.Day << 1) | (timestamp.Hour >>4));
	writeBytePagewize((timestamp.Hour<<4) | (timestamp.Minute>>2));
	writeBytePagewize(((timestamp.Minute<<6)| (timestamp.Second)) & 0xFF);

	for(; size > 0; size--)
	{
		writeBytePagewize(*data);
		data++;
	}
	writeBytePagewizeEnd();

	// We must also clear any existing block in the next write address
	//  this ensures that if the reader catches up to us that it will only
	//  read a blank block
	makeEEPROMSpace(eepromWriteAddress, 5);

	return 1;
}

uint16_t DS3231_Simple::readLogFrom( uint16_t Address, DateTime &timestamp, uint8_t *data, uint8_t size )
{
	uint8_t b1, b2, datalength;

	b1 = readEEPROMByte(Address++);
	b2 = readEEPROMByte(Address++);

	if(!b1) return EEPROM_BYTES+1;

	datalength = (b1 >> 5);

	// <Timestamp> ::= 0Bzzzwwwyy yyyyyymm mmdddddh hhhhiiii iissssss
	timestamp.Dow = (b1 >> 2) | 0x03;
	timestamp.Year = (b1 << 6) | (b2>>2);// & 0b11111111

	b1 = readEEPROMByte(Address++);
	timestamp.Month = ((b2 << 2) | (b1 >> 6)) & 0b00001111;
	timestamp.Day = (b1 >> 1) & 0b00011111;

	b2 = readEEPROMByte(Address++);
	timestamp.Hour = ((b1 << 4) | (b2 >> 4)) & 0b00011111;

	b1 = readEEPROMByte(Address++);
	timestamp.Minute = ((b2 << 2) | (b1 >> 6)) & 0b00111111;
	timestamp.Second = b1 & 0b00111111;

	while(datalength--)
	{
		// If our supplied buffer has room, copy the data byte into it
		if(size)
		{
			size--;
			*data = readEEPROMByte(Address);
			data++;
		}

		Address++;
	}

	// If we have caught up with the writer, return that as the next read
	if(Address == eepromWriteAddress) return Address;

	while(Address < EEPROM_BYTES && (readEEPROMByte(Address) == 0))
	{
		Address++;
	}

	if(Address == EEPROM_BYTES && eepromWriteAddress < Address)
	{
		// There was nothing ahead of us, and the writer is behind us
		//  which means this is all empty unusable space we just walked
		//  so go to zero position
		Address = 0;
	}

	return Address;
}

uint8_t DS3231_Simple::readLog( DateTime &timestamp, uint8_t *data, uint8_t size )
{
	// Initialize the read address
	if(eepromReadAddress >= EEPROM_BYTES) findEEPROMReadAddress();

	// Is it still empty?
	if(eepromReadAddress >= EEPROM_BYTES)
	{
		// No log block was found.
		return 0;
	}

	uint16_t nextReadAddress = readLogFrom(eepromReadAddress, timestamp, data, size);

	if(nextReadAddress == EEPROM_BYTES+1)
	{
		// Indicates no log entry was read (0 start byte)
		return 0;
	}

	// Was read OK so we need to kill that byte, we won't trust the user to have
	// given the correct size here, instead read the start byte
	makeEEPROMSpace(eepromReadAddress, (readEEPROMByte(eepromReadAddress)>>5)+5);

	eepromReadAddress = nextReadAddress;
	return 1;
}
