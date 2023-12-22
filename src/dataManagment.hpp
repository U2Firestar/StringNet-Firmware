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

 * Software: StringNet-Firmware for RF-Control - EEPROM-Datamanagment
 * Version: 2.0.0, 16.04.2022
 * Author: Emil Sedlacek (U2Firestar, Firestar)
 * Supported by UAS Technikum Vienna and 3S-Sedlak
 * Note: See README and source-Files of firmware for more info!
 * 
 * Special thanks to:
 * RC-Switch from sui77 @ Github https://github.com/sui77/rc-switch
 * OSFS from charlesbaynham @ Github https://github.com/charlesbaynham/OSFS
 */

#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <avr/eeprom.h>
#include <OSFS.h>
#include "common.hpp"

////////// EEPROM FUNCTIONS ////////

// Initial EEPROM-Checks and Actions like Check for MagicByte (if not: Autoformat) and loading systemsettings
void initEEPROM(void);

// Useful DEBUG-Function to print full EEPROM to Serial
void printWholeEEPROM(void);

// ## READ FUNCTIONS ##

// Load systembits from EEPROM to MEM
void loadSystemBits(void);

/** @brief Loads Name of Device from EEPROM and stores it in Target
 * @param buffer Target-Buffer
 * @param len Length of Target-Buffer to safeguarded
 */
void DevNameToBuffer(char *buffer, uint8_t len);

/** @brief Accesses GPIO-Object through OSFS
 * @param nr Number of GPIO-Object
 * @returns EXIT_FAIL or EXIT_SUCCESS
 */
uint8_t accessGPIOObject(uint8_t nr);

/** @brief Accesses RF-Object through OSFS
 * @param nr Number of RF-Object
 * @returns EXIT_FAIL or EXIT_SUCCESS
 */
uint8_t accessRFObject(uint8_t nr);

/** @brief Searches through OSFS to find Object by Name. Attention: Overwrites corresponding buffer!
 * @param Name Source-string to compare, is safeguarded
 * @param objecttype Object-Type to work with
 * @returns Index number of Object
 */
uint8_t findObjectByName(char *Name, uint8_t devtype);

/** @brief Searches through OSFS to find first fitting Object by devtype. Attention: Overwrites corresponding buffer!
 * @param objecttype Object-Type to work with
 * @returns Index number of Object
 */
uint8_t findFirstObjectByDevtype(uint8_t devtype);

/** @brief Tests through OSFS to find next free index. Attention: Overwrites corresponding buffer!
 * @param Name Source-string to compare, is safeguarded
 * @param objecttype Object-Type to work with
 * @returns Index number of Object
 */
uint8_t findNextFreeIndex(uint8_t devtype);

/** @brief Seaches through OSFS to find next free index. Attention: Overwrites corresponding buffer!
 * @param objecttype Object-Type to work with count number or get highest Index of chosen Objecttype (Names, Devtypes, RF_DEVs)
 * @param returnHighestIndex Source-string to compare, is safeguarded
 * @returns count number or highest Index of chosen Objecttype x
 */
uint8_t getCountOf(uint8_t devtype, bool returnHighestIndex);

/** @brief Makes sure the given Objecttype and Objectnumber is valid. Also inits it. Attention: Overwrites corresponding buffer!
 * @param objectNr Target dataset - Nr of Object
 * @param objectType Objecttype defined in #define
 * @returns EXIT_FAIL or EXIT_SUCCESS - dependend on outcome of check and arm procedure
 */
uint8_t checkAndArmSingleDATASET(uint8_t objectNr, uint8_t objectType);

// ## WRITE FUNCTIONS ##

/** @brief Checks EEPROM-Health. Sends official message if bad situation.
 * @param address Pointer to EEPROM-Address-Cell
 * @param value Value that shall be written
 * @returns Damaged cell-address on error. Else: EXIT_SUCCESS
 */
uint8_t updateByteAndCheckHealth(uint8_t *address, uint8_t value);

/** @brief Writes to EEPROM AND to systemmemory
 * @param newLifeSignIntervall New Intervall of LifeSign-Packages to be send. 0 disables it.
 * @param newNetworkMode Turns On/Off From2-Communcation/Networkmode
 * @returns
 * @note
 */
uint8_t writeSystemBits(uint32_t newLifeSignIntervall, bool newNetworkMode);

// Tells that storage is about to be reset and performs reset of OSFS and Systemsettings without harming EEPROM too much.
void fullStorageReset(void);

/** @brief Updates name of device/system in EEPROM
 * @param Name Source-string to be stored
 * @param len Length of source (safeguarded by max length in EEPROM)
 */
void updateDevName(char *Name, uint8_t len);

/** @brief Writes/Creates GPIO-Object-Buffer to OSFS and check whether it was succcessfull. Attention: Needs preparation of tmp_gpio!
 * @param nr Number if GPIO-Object that shall be accessed
 * @returns EXIT_FAIL or EXIT_SUCCESS - depending on result
 */
uint8_t updateGPIOObject(uint8_t nr);

/** @brief Writes/Creates RF-Object-Buffer to OSFS and check whether it was succcessfull. Attention: Needs preparation of tmp_rf!
 * @param nr Number if RF-Object that shall be accessed
 * @returns EXIT_FAIL or EXIT_SUCCESS - depending on result
 */
uint8_t updateRFObject(uint8_t nr);

/** @brief Deletes a Object from OSFS and checks for success.
 * @param objectNr Target dataset - Nr of Object
 * @param objectType Objecttype defined in #define
 * @returns EXIT_FAIL or EXIT_SUCCESS - dependend on outcome of check procedure
 */
uint8_t deleteObject(uint8_t nr, uint8_t devtype);

#endif