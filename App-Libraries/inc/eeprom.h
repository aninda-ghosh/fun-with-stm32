/*
 * eeprom.h
 *
 *  Created on: Sep 8, 2020
 *      Author: anind
 */

#ifndef INC_EEPROM_H_
#define INC_EEPROM_H_

#include "iic.h"

static const uint8_t EEPROM_ADDRESS = 0x57;
// 7 Bit address, the first 4 bits are 1010, the the last 3 bits according to A2, A1 and A0
// On the common ZS-042 board, this corresponds to (where x is jumper open, and 1 is jumper closed)
// A0    A1    A2
//  x    x     x    0x57  (default)
//  1    x     x    0x56
//  x    1     x    0x55
//  1    1     x    0x54
//  0    0     1    0x53
//  1    0     1    0x52
//  1    1     1    0x51

static const uint16_t EEPROM_SIZE_KBIT = 32768;  // EEPROMs are sized in kilobit
static const uint8_t EEPROM_PAGE_SIZE = 32; // And have a number of bytes per page
static const uint16_t EEPROM_BYTES = EEPROM_SIZE_KBIT / 8;
static const uint8_t EEPROM_PAGES = EEPROM_BYTES / EEPROM_PAGE_SIZE;

// EEPROM structure
//  The EEPROM is used to store "log entries" which each consist of a 5 byte header and an additional 0 to 7 data bytes.
//  The Header of each block includes a count of the data bytes and then a binary representation of the timestamp.
//
//  Blocks are recorded in a circular-buffer fashion in order to reduce wear on the EEPROM, that is, each next block is stored
//  after the block preceeding it, when the top of the EEPROM is reached the next block goes to the EEPROM address zero.
//
//  Blocks do NOT cross EEPROM boundary (a block will not start at the top of the EEPROM and loop around to the bottom) before ending
//  if a block doesn't fit in the available space at the top of the blocks in the EEPROM, it will be placed in address zero.
//
//  Blocks are zeroed completely on read or when they are to be overwritten (even partially overwritten the entire block is nuked).
//
//  <Block>     ::= <Header><DataBytes>
//  <Header> ::= 0Bzzzwwwyy yyyyyymm mmdddddh hhhhiiii iissssss binary representation of DateTime, (zzz = number of data bytes following timestamp, www = day-of-week)
//  <DataBytes> ::= DB1..7

uint16_t eepromWriteAddress = EEPROM_BYTES; // Byte address of the "top" of the EEPROM "stack", the next
// "block" stored will be put here, this location may be
// a valid block start byte, or it may be 00000000 in which case
// there are zero bytes until the next block start which will be
// the first of the series.

uint16_t eepromReadAddress = EEPROM_BYTES; // Byte address of the "bottom" of the EEPROM "stack", the next
// "block" to read is found here, this location may be
// a valid block start byte, or it may be 00000000 in which case
// there are zero bytes to read.

/** Searches the EEPROM for the next place to store a block, sets eepromWriteAddress
 *
 *  @return eepromWriteAddress
 */

uint16_t findEEPROMWriteAddress();

/** Delete enough complete blocks to have enough free space for the
 *  given required number of bytes.
 *
 *  @return True/False for success/fail
 */

uint8_t makeEEPROMSpace(uint16_t Address, int8_t BytesRequired);

/** Find the oldest block to read (based on timestamp date), set eepromReadAddress
 *
 *  Note: Has to search entire EEPROM, slow.
 *
 *  @return eepromReadAddress
 */

uint16_t findEEPROMReadAddress();

/** Read log timestamp and data from a given EEPROM address.
 *
 *  @param Address Byte address of log block
 *  @param timestamp DateTime structure to put the timestamp
 *  @param data Memory location to put the data associated with the log
 *  @param size Max size of the data to read (any more is discarded)
 */

uint16_t readLogFrom(uint16_t Address, DateTime &timestamp, uint8_t *data, uint8_t size = 0);

/** Start a "pagewize" write at the eepromWriteAddress.
 *
 *  Follow this call with 1 or more calls to writeBytePagewize()
 *  Finish with a call to writeBytePagewizeEnd()
 *
 *  Writes in the same page (or rather, 16 byte sections of a page)
 *  are performed as a single write, thence a new write is started
 *  for the next section.
 *
 *  This solves 2 problems, first the 32 byte Wire buffer which isn't
 *  large enough for a whole page of bytes plus the address bytes.
 *  Second the fact that the EEPROM loops back to the start of page
 *  when you run off the end of the page while writing (instead of
 *  to the start of the next page).
 *
 *  Note that you must not modify the eepromWriteAddress between
 *  a writeBytePagewizeStart() and a writeBytePagewizeEnd()
 *  or it's all going to get out of kilter.
 *
 *  @return Success (boolean) 1/0, presently this method always reports success.
 *
 */
uint8_t writeBytePagewizeStart();

/** Write a byte during a pagewize operation.
 *
 *  Note that this function increments the eepromWriteAddress.
 *
 *  @param data Byte to write.
 *  @see DS3231::writeBytePagewizeStart()
 *  @return Success (boolean) 1/0, presently this method always reports success.
 */

uint8_t writeBytePagewize(const uint8_t data);

/** End a pagewize write operation.
 *
 *  @see DS3231::writeBytePagewizeStart()
 *  @return Success (boolean) 1/0, this method will return 0 if Wire.endTransmission reports an error.
 */

uint8_t writeBytePagewizeEnd();

/** Read a byte from the EEPROM
 *
 *  @param Address The address of the EEPROM to read from.
 *  @return The data byte read.
 *  @note   There is limited error checking, if you provide an invalid address, or the EEPROM is not responding etc behaviour is undefined (return 0, return 1, might or might not block...).
 */
uint8_t readEEPROMByte(const uint16_t Address);

public:
/** Erase the EEPROM ready for storing log entries. */

uint8_t formatEEPROM();

/** Write a log entry to the EEPROM, having current timestamp, with an attached data of arbitrary datatype (7 bytes max).
 *
 *  This method allows you to record a "log entry" which you can later retrieve.
 *
 *  The full timestamp is recorded along with one piece of data.  The type of data you provide
 *  is up to you, as long as your data is less than 7 bytes (byte, int, float, char,
 *  or a 6 character string would all be fine).
 *
 *  To store more than one piece of data in a log, use a struct, again, in total your structure
 *  needs to occupy 7 bytes or less of memory.
 *
 *  Examples:
 *     Clock.writeLog(analogRead(A0));
 *     Clock.writeLog( MyTimeAndDate, MyFloatVariable );
 *
 *     // Remember, 7 bytes max!
 *     struct MyDataStructure
 *     {
 *        unsigned int AnalogValue1;
 *        unsigned int AnalogValue2;
 *     };
 *     MyDataStructure MyData; MyData.AnalogValue1 = 123; MyData.AnalogValue2 = 456;
 *     Clock.writeLog(MyDataStructure);
 *
 *
 *  @param data  The data to store, any arbitrary scalar or structur datatype consisting not more than 7 bytes.
 *  @note  To store a string or other pointer contents, you probably want to use `DS3231::writeLog(const DateTime, const uint8_t *data, uint8_t size)`
 *
 */

template <typename datatype>
uint8_t writeLog( const datatype &data ) {
	return writeLog(read(), (uint8_t *) &data, (uint8_t)sizeof(datatype));
}

/** Write a log entry to the EEPROM, having supplied timestamp, with an attached data of arbitrary datatype (7 bytes max).
 *
 * @see DS3231::writeLog(const datatype &   data)
 * @param timestamp The timestamp to associate with the log entry.
 * @param data  The data to store, any arbitrary datatype consisting not more than 7 bytes.
 */

template <typename datatype>
uint8_t writeLog( const DateTime &timestamp, const datatype &data ) {
	return writeLog(timestamp, (uint8_t *) &data, (uint8_t)sizeof(datatype));
}

/** Write a log entry to the EEPROM, having supplied timestamp, with an attached data.
 *
 * @param timestamp The timestamp to associate with the log entry.
 * @param data  Pointer to the data to store
 * @param size  Length of data to store - max length is 7 bytes, this is not checked for compliance!
 */

uint8_t writeLog( const DateTime &timestamp, const uint8_t *data, uint8_t size = 1 );

/** Read the oldest log entry and clear it from EEPROM.
 *
 *  @param timestamp Variable to put the timestamp of the log into.
 *  @param data      Variable to put the data.
 *
 *  @note No check is made as to if the type of "data" and the type of
 *    the data which is read is the same, there is nothing to stop you
 *    doing "writeLog( myFloat );" and subsequently "readLog( myInt );"
 *    but if you do so, results are going to be not what you expect!
 *
 */

template <typename datatype>
uint8_t readLog( DateTime &timestamp, datatype &data ) {
	return readLog(timestamp, (uint8_t *) &data, (uint8_t)sizeof(datatype));
}

/** Read the oldest log entry and clear it from EEPROM.
 *
 *  @param timestamp Variable to put the timestamp of the log into.
 *  @param data      Pointer to buffer to put data associated with the log.
 *  @param size      Size of the data buffer.  Maximum 7 bytes.
 *
 *  @note If the data in the log entry is larger than the buffer, it will be truncated.
 *
 */

uint8_t readLog( DateTime &timestamp, uint8_t *data, uint8_t size = 1 );

#endif /* INC_EEPROM_H_ */
