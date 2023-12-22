/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.

 * Software: StringNet-Firmware for RF-Control - Commandcenter and IO
 * Version: 2.0.0, 16.04.2022
 * Author: Emil Sedlacek (U2Firestar, Firestar)
 * Supported by UAS Technikum Vienna and 3S-Sedlak
 * Note: See README and source-Files of firmware for more info!
 * 
 * Special thanks to:
 * RC-Switch from sui77 @ Github https://github.com/sui77/rc-switch
 * OSFS from charlesbaynham @ Github https://github.com/charlesbaynham/OSFS
 */

#ifndef _COMMON_HPP
#define _COMMON_HPP

////////// External LIBRARYs ////////

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <Arduino.h>

////////// DEFINITIONS ////////

// Error Codes & Werte

#define LEN_String_32BIT 12 // +4 294 967 296 --> with sign - 11 + Terminator

#define EXIT_SUCCESS 0 // SAY OR DO NOTHING
#define EXIT_FAIL 0xFF // WORSTCASE - bound to max-val of uint8_t

#define ERROR_INIT 1
#define ERROR_SERIAL 10
#define ERROR_COMMANDCENTER 20
#define ERROR_DATAMANGEMENT 30
#define ERROR_EEPROM 40
#define ERROR_RF_INTERACTION 50

// DEVICE-SPECIFIC-INFO

#ifdef ARDUINO_AVR_NANO
#define EEPROM_SIZE (E2END + 1) // Based on ATMEGA328P
#define MAX_FILEINDEX_ITERATE_COUNT 30 // for Arduino Nano calculated Number of Strings entrys to be tested from BEGINN_FILEINDEX. Warning: does not prevent EEPROM-fragmentation!
#endif

// Serial & StringNet

const char zeroTerm = '\0';
const char space = ' ';
const char seqStart = '{';
const char stringDelim = ';';
const char seqEnd = '}';

#define DEVICE_BAUDRATE 115200
#define SIZE_SINGLE_INPUTBUFFER 31
// Max 60 Chars + Terminator for Input: {XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX;
// Max 30 Chars + Terminator for Input: {XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX;
// Max 15 Chars + Terminator for Input: {XXXXXXXXXXXXXXX;

const char DEVICE_TYPE[] = "ArduinoNano";
const char DEVICE_STANDARDTARGET_NAME[] = "stringNet";

// InputBuffer Codes - NOTE: The bufferstates MUST be incremental!

#define INPUT_BUFFERMODE_WAIT 0
#define INPUT_BUFFERMODE_RECORD_FROM 1
#define INPUT_BUFFERMODE_RECORD_TO 2
#define INPUT_BUFFERMODE_RECORD_COM 3
#define INPUT_BUFFERMODE_RECORD_SUBCOM 4
#define INPUT_BUFFERMODE_RECORD_VAL_NUM 6
#define INPUT_BUFFERMODE_RECORD_VAL_STR 5
#define INPUT_BUFFERMODE_INTERPRET 10

// EEPROM

#define MAGICBYTE 69
#define MAGICBYTE_INDEX 0 // beginning at 0
#define SIZE_MAGICBYTE 1

#define STANDARD_NETWORKMODE false
#define SIZE_NETWORKMODE 1

#define STANDARD_LIFESIGNINTERVALL_MS 60000
#define SIZE_LIFESIGNINTERVALL 4

#define DEV_NAME_OFFSET (SIZE_MAGICBYTE + SIZE_NETWORKMODE + SIZE_LIFESIGNINTERVALL) // beginning at 0, start at this pos
#define SIZE_MAX_NAME 16                                                             // Max 15 Chars for Name + Terminator

#define BEGINN_FILEINDEX 1             // 1 due to functionreturns, which could be 0 in errorcase

const char STANDARD_FILENAME_GPIO[] = "GPIO#"; // plus assigned Number
const char STANDARD_FILENAME_RF[] = "RF#";     // plus assigned Number

// RF

#define STOCK_RF_BITLEN 24
#define MUMBI_RF_BITLEN 32
#define MUMBI_RF_PROTO 13

#define SEND_OFF 0
#define SEND_ON 1
#define SEND_TOGGLE 2

// TABLE Codes - NOTE: String-bound #defines HAVE to be incremental array-indexes (0...x)

#define COMMAND_MSGPARAM_CODE 1
#define COMMAND_MSGPARAM_COUNT 6
#define COMMAND_MSGPARAM_FROM 0
#define COMMAND_MSGPARAM_TO 1
#define COMMAND_MSGPARAM_COM 2
#define COMMAND_MSGPARAM_SUBCOM 3
#define COMMAND_MSGPARAM_VALSTR 4
#define COMMAND_MSGPARAM_VALNUM 5

#define COMMAND_COMMAND_CODE 2
#define COMMAND_COUNT 14
#define COMMAND_FORMAT 0
#define COMMAND_NAME 1
#define COMMAND_LIFESIGN 2
#define COMMAND_STRINGNET_MODE 3
#define COMMAND_DISCOVER 4
#define COMMAND_CREATE 5
#define COMMAND_DELETE 6
#define COMMAND_DEVTYPE 7
#define COMMAND_PIN 8
#define COMMAND_RFONSEQ 9
#define COMMAND_RFOFFSEQ 10
#define COMMAND_RFPROTO 11
#define COMMAND_RFPULSLEN 12
#define COMMAND_RFTXREP 13

#define COMMAND_SUB_CODE 3
#define SUBCOMMAND_COUNT 7
#define COMMAND_SUB_TELLALL 0
#define COMMAND_SUB_TELLDEV 1
#define COMMAND_SUB_TELLGPIO 2
#define COMMAND_SUB_TELLRF 3
#define COMMAND_SUB_SETDEV 4
#define COMMAND_SUB_SETGPIO 5
#define COMMAND_SUB_SETRF 6

#define COMMAND_STATE_CODE 4
#define STATUS_COUNT 9
#define COMMAND_STATE_ON 0
#define COMMAND_STATE_ON_Silent 1
#define COMMAND_STATE_OFF 2
#define COMMAND_STATE_OFF_Silent 3
#define COMMAND_STATE_TOGGLE 4
#define COMMAND_STATE_TOGGLE_Silent 5
#define COMMAND_STATE_STATUS 6
#define COMMAND_STATE_ERROR 7
#define COMMAND_STATE_WARN 8

#define COMMAND_DEVTYPE_CODE 5
#define DEVTYPE_COUNT 12
#define DEVTYPE_DI_GENERIC 0
#define DEVTYPE_DI_PULLUP 1
#define DEVTYPE_DI_RF_REC 2
#define DEVTYPE_DO_GENERIC 3
#define DEVTYPE_DO_RF_SEND 4
#define DEVTYPE_AI 5 // TODO
#define DEVTYPE_AO 6 // TODO
#define DEVTYPE_RF 7
#define DEVTYPE_GPIO 8
#define DEVTYPE_NAME_ALL 9
#define DEVTYPE_NAME_GPIO 10
#define DEVTYPE_NAME_RF 11

// Dataobjects

struct GPIO_Object
{
    char name[SIZE_MAX_NAME];
    uint8_t devType;
    uint8_t pin;
    GPIO_Object()
    {
        memset(name, zeroTerm, SIZE_MAX_NAME);
        devType = 0;
        pin = 0;
    }
};

struct RF_Object
{
    char name[SIZE_MAX_NAME];
    uint32_t rfoffSeq;
    uint32_t rfonSeq;
    uint16_t pulseln;
    uint8_t rfproto;
    uint8_t repeatTX;
    RF_Object()
    {
        memset(name, zeroTerm, SIZE_MAX_NAME);
        rfoffSeq = 0;
        rfonSeq = 0;
        pulseln = 0;
        rfproto = 0;
        repeatTX = 0;
    }
};

struct stringNetObject
{
    char FROM[SIZE_SINGLE_INPUTBUFFER];
    char TO[SIZE_SINGLE_INPUTBUFFER];
    char COM[SIZE_SINGLE_INPUTBUFFER];
    char SUBCOM[SIZE_SINGLE_INPUTBUFFER];
    char VAL_STR[SIZE_SINGLE_INPUTBUFFER];
    uint32_t VAL_NUM;
    stringNetObject()
    {
        memset(FROM, zeroTerm, SIZE_SINGLE_INPUTBUFFER);
        memset(TO, zeroTerm, SIZE_SINGLE_INPUTBUFFER);
        memset(COM, zeroTerm, SIZE_SINGLE_INPUTBUFFER);
        memset(SUBCOM, zeroTerm, SIZE_SINGLE_INPUTBUFFER);
        memset(VAL_STR, zeroTerm, SIZE_SINGLE_INPUTBUFFER);
        VAL_NUM = 0;
    }
};

////////// SELFMADE FUNCTIONS and LIBRARYS ////////

#include "rf.hpp"
#include "dataManagment.hpp"

// Helper

// Converts given String to uint32_t. Attention(!): Cuts off bits over 2^32 and also returns 0 in Errorcase!!!
uint32_t string2Number(String cp, uint8_t len);

/** @brief Converts a value to String with binary meaning
 * @returns "ON" (> 0) or "OFF" (= 0)
 */
String boolToBinaryString(uint8_t value);

/** @brief CLEAN way to load all tables from Flash
 * @param tablecode Given Value in #define for table (is safeguarded for interation)
 * @param index Index of predefined table (is safeguarded for interation)
 * @param ptr Target to be written to
 * @param buffLen Length of target buffer
 * @return #defined EXIT_SUCCUESS or EXIT_FAIL
 * @note , posssible input: indexes (0..x) corresponding to the <table>_values OR <table>_values themselfs
 */
uint8_t stringStorageToBuffer(uint8_t tablecode, uint8_t index, char *ptr, uint8_t buffLen);

/** @brief  Compares input to table
 * @param pch Pointer to Source-string
 * @param tablecode Given Value in #define for table (is safeguarded for interation)
 * @returns Corresponding tableindex or EXIT_FAIL on error;
 * @attention Chararray of *pch NEED to have a String-Terminator, otherwise there could be bad mem-access
 */
uint8_t tableMatch(char *pch, uint8_t tablecode);

/** @brief
 * @param pin Pin to compare to the predefined
 * @param devtype Type of usage of the physical object
 * @param returnIndex Whether it should be a Index (predefined array) or a Boolean value
 * @returns Index or a Boolean value - dependend on success
 */
uint8_t verifyDevtypeToIO(uint8_t pin, uint8_t devtype, bool returnIndex);

// Commandcenter

// Sets up Serial-Connection
void initConnectivity(void);

// Dependend on the systems intervall-value and the timer, a StringNet-LifeSign-Message will be send periodically
void lifesignHandler(void);

// DEBUG-Function to test interpretation-System internally
void internalStringNetblinker(void);

/** @brief Handles connection input and protocoll, fills global stringnetobject Input and eventually triggers interpretation
 * @note Saving the Number-String and converting to variable is important as only the String-String is saved in FW-Memory
 * @attention Can break mid-package if there is a '{'
 */
void inputHandler(void);

/** @brief Based on the strings in the Input-Buffers the FW stores, deletes, controls and errorhandles according to the given StringNet-Message. Requires the Input-Buffer-Object to be prepaired!
 * @note Can also be used to trigger actions internally.
 */
uint8_t interpretInput(void);

// Package-Wrapper and sender for serializing Messages. Needs preparation by writing to global StringNetObject Output!
void sendSerialSTRINGNET(void);

////////// ALL WARNINGS ////////

#ifndef ARDUINO_AVR_NANO
#error "No EEPROM_SIZE set!"
#error "Check functions regarding pins and VIDs!"
#error "No standard FROM or TO is set!"
#endif

#ifndef EEPROM_SIZE
#error "EEPROM-Size is unchecked!"
#endif

#if (MAX_FILEINDEX_ITERATE_COUNT > 32)
#error "Check RFbitBuffer! Does it and its functions need to be bigger?"
#endif

#if MAX_FILEINDEX_ITERATE_COUNT >= 0xFE
#error "Check all functions regarding IDs! Some functions need to return bigger than uin8_t!"
#endif

#if ((DEV_NAME_OFFSET + SIZE_MAX_NAME) > EEPROM_SIZE)
#error "DevName-Length exeeds EEPROM"
#endif

#endif