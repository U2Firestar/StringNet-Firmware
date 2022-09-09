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

 * Softwarename: StringNet-IoT-Gateway - EEPROM-Datamanagment
 * Version: 2.0.0, 16.04.2022
 * Author: Emil Sedlacek (U2Firestar, Firestar)
 * Supported by UAS Technikum Vienna and 3S-Sedlak
 * Note: See Documentation, commandtable in Excle and Firmware-Source-Files for more info!
 * 
 * Special thanks to:
 * RC-Switch from sui77 @ Github https://github.com/sui77/rc-switch
 * OSFS from charlesbaynham @ Github https://github.com/charlesbaynham/OSFS
 */

#include "dataManagment.hpp"

////////// SEMI-GLOBAL VARS /////////

extern GPIO_Object tmp_gpio; // see common.cpp
extern RF_Object tmp_rf;

extern stringNetObject Input;
extern stringNetObject Output;

extern bool NetworkMode;
extern uint32_t lifesign_interval_ms;

////////// OSFS & EEPROM-Helper ////////

uint16_t OSFS::startOfEEPROM = 1 + DEV_NAME_OFFSET + SIZE_MAX_NAME; // starts with index 1
uint16_t OSFS::endOfEEPROM = EEPROM_SIZE;							// starts with index 1
const OSFS::result noerr = OSFS::result::NO_ERROR;
const OSFS::result notfound = OSFS::result::FILE_NOT_FOUND;
OSFS::result OSFS_result;

// Neccessary definition for OSFS
void OSFS::readNBytes(uint16_t address, unsigned int num, byte *output)
{
	for (uint16_t i = address; i < address + num; i++)
	{
		*output = eeprom_read_byte((uint8_t *)i);
		output++;
	}
}

// Neccessary definition for OSFS
void OSFS::writeNBytes(uint16_t address, unsigned int num, const byte *input)
{
	for (uint16_t i = address; i < address + num; i++)
	{
		updateByteAndCheckHealth((uint8_t *)i, *input);
		input++;
	}
}

/** @brief Intern function to print out unusual errors and interpret whether EEPROM-Operation from OSFS was a success
 * @param OSFS_result Intern OSFS-Result of Action
 * @param nr Object-Nr
 * @param objecttype Objecttype within FW
 * @returns EXIT_FAIL or EXIT_SUCCESS
 */
uint8_t printOSFSError(OSFS::result OSFS_result, uint8_t nr, uint8_t objecttype)
{
	if (OSFS_result == notfound)
	{
		/*
		Serial.print(F("File Not found. Its")); // DEBUG
		switch (objecttype)
		{
		case DEVTYPE_GPIO:
			Serial.print(F("GPIO#"));
			break;
		case DEVTYPE_RF:
			Serial.print(F("RF#"));
			break;

		default:
			Serial.println(F("printOSFSError(): Default-Error! 1"));
			break;
		}
		Serial.println(nr);
		*/

		return EXIT_FAIL;
	}
	else if (OSFS_result == noerr) // SUCCESS
	{
		/*
		Serial.print(F("File found. Its ")); //DEBUG
		switch (objecttype)
		{
		case DEVTYPE_GPIO:
			Serial.print(F("GPIO#"));
			break;
		case DEVTYPE_RF:
			Serial.print(F("RF#"));
			break;

		default:
			Serial.println(F("printOSFSError(): Default-Error! 2"));
			return EXIT_FAIL;
			break;
		}
		Serial.println(nr);
		*/

		return EXIT_SUCCESS;
	}
	else
	{
		Serial.print(F("printOSFSError - OSFS_result: "));
		Serial.println((int)OSFS_result);
		return EXIT_FAIL;
	}
}

////////// EEPROM FUNCTIONS ////////

// Initial EEPROM-Checks and Actions like Check for MagicByte (if not: Autoformat) and loading systemsettings
void initEEPROM(void)
{
	//  Check for MagicByte; clear EEPROM and reset device if not present
	if (eeprom_read_byte((uint8_t *)MAGICBYTE_INDEX) != MAGICBYTE)
		fullStorageReset();

	loadSystemBits();

	// printWholeEEPROM(); // DEBUG
}

// Useful DEBUG-Function to print full EEPROM to Serial
void printWholeEEPROM(void)
{
	Serial.println(F("Printing whole EEPROM in HEX!"));
	for (uint16_t i = 0; i < EEPROM_SIZE; i = i + 2)
	{
		Serial.print(eeprom_read_byte((uint8_t *)i), 16);
		Serial.print('|');
	}
	Serial.println();
}

// ## READ FUNCTIONS ##

// Load systembits from EEPROM to MEM
void loadSystemBits(void)
{
	NetworkMode = (bool)eeprom_read_byte((uint8_t *)(SIZE_MAGICBYTE));
	lifesign_interval_ms = eeprom_read_dword((uint32_t *)(SIZE_MAGICBYTE + SIZE_NETWORKMODE));
	//Serial.println(NetworkMode);		  // DEBUG
	//aSerial.println(lifesign_interval_ms); // DEBUG
}

/** @brief Loads Name of Device from EEPROM and stores it in Target
 * @param buffer Target-Buffer
 * @param len Length of Target-Buffer to safeguarded
 */
void DevNameToBuffer(char *buffer, uint8_t len)
{
	if (memset(buffer, '\0', len) != NULL)
	{
		if (len > SIZE_MAX_NAME)
			len = SIZE_MAX_NAME; // minimize to array
		for (uint8_t i = 0; i < len; i++)
			buffer[i] = (char)eeprom_read_byte((uint8_t *)(DEV_NAME_OFFSET + i));
	}
}

/** @brief Accesses GPIO-Object through OSFS
 * @param nr Number of GPIO-Object
 * @returns EXIT_FAIL or EXIT_SUCCESS
 */
uint8_t accessGPIOObject(uint8_t nr)
{
	String fileNameString = STANDARD_FILENAME_GPIO + String(nr);
	char fileName[OSFS::FILE_NAME_LENGTH] = {'\0'};
	fileNameString.toCharArray(fileName, OSFS::FILE_NAME_LENGTH);
	OSFS_result = OSFS::getFile(fileName, tmp_gpio);

	return printOSFSError(OSFS_result, nr, DEVTYPE_GPIO);
}

/** @brief Accesses RF-Object through OSFS
 * @param nr Number of RF-Object
 * @returns EXIT_FAIL or EXIT_SUCCESS
 */
uint8_t accessRFObject(uint8_t nr)
{
	String fileNameString = STANDARD_FILENAME_RF + String(nr);
	char fileName[OSFS::FILE_NAME_LENGTH] = {'\0'};
	fileNameString.toCharArray(fileName, OSFS::FILE_NAME_LENGTH);
	OSFS_result = OSFS::getFile(fileName, tmp_rf);

	return printOSFSError(OSFS_result, nr, DEVTYPE_RF);
}

/** @brief Searches through OSFS to find Object by Name. Attention: Overwrites corresponding buffer!
 * @param Name Source-string to compare, is safeguarded
 * @param objecttype Object-Type to work with
 * @returns Index number of Object or EXIT_FAIL - if not successfull
 */
uint8_t findObjectByName(char *Name, uint8_t objecttype)
{
	/*
	Serial.print("findObjectByName()! name: "); // DEBUG
	Serial.print(Name);
	Serial.print(space);
	Serial.println(objecttype);
	*/

	if (strlen(Name) > 0)
	{

		switch (objecttype)
		{
		case DEVTYPE_GPIO:
			for (uint8_t i = BEGINN_FILEINDEX; i < MAX_FILEINDEX_ITERATE_COUNT; i++)
			{
				if (accessGPIOObject(i) == EXIT_SUCCESS)  // FOUND!
					if (strcmp(Name, tmp_gpio.name) == 0) // IS the first right Object
					{
						return i;
						break;
					}
			}
			break;

		case DEVTYPE_RF:
			for (uint8_t i = BEGINN_FILEINDEX; i < MAX_FILEINDEX_ITERATE_COUNT; i++)
			{
				if (accessRFObject(i) == EXIT_SUCCESS)	// FOUND!
					if (strcmp(Name, tmp_rf.name) == 0) // IS the first right Object
					{
						return i;
						break;
					}
			}

		default:
			break;
		}
	}

	return EXIT_FAIL;
}

/** @brief Searches through OSFS to find first fitting Object by devtype. Attention: Overwrites corresponding buffer!
 * @param objecttype Object-Type to work with
 * @returns Index number of Object
 */
uint8_t findFirstObjectByDevtype(uint8_t devtype)
{
	for (uint8_t i = BEGINN_FILEINDEX; i < MAX_FILEINDEX_ITERATE_COUNT; i++)
		if (accessGPIOObject(i) == EXIT_SUCCESS && tmp_gpio.devType == devtype)
			return i;
	return EXIT_FAIL;
}

/** @brief Tests through OSFS to find next free index. Attention: Overwrites corresponding buffer!
 * @param Name Source-string to compare, is safeguarded
 * @param objecttype Object-Type to work with
 * @returns Index number of Object
 */
uint8_t findNextFreeIndex(uint8_t objecttype)
{
	switch (objecttype)
	{
	case DEVTYPE_GPIO:
		for (uint8_t i = BEGINN_FILEINDEX; i < MAX_FILEINDEX_ITERATE_COUNT; i++)
			if (accessGPIOObject(i) != EXIT_SUCCESS)
				return i;
		break;

	case DEVTYPE_RF:
		for (uint8_t i = BEGINN_FILEINDEX; i < MAX_FILEINDEX_ITERATE_COUNT; i++)
			if (accessRFObject(i) != EXIT_SUCCESS)
				return i;
		break;

	default:
		return EXIT_FAIL;
		break;
	}
	return EXIT_FAIL;
}

/** @brief Seaches through OSFS to find next free index. Attention: Overwrites corresponding buffer!
 * @param objecttype Object-Type to work with count number or get highest Index of chosen Objecttype (Names, Devtypes, RF_DEVs)
 * @param returnHighestIndex Source-string to compare, is safeguarded
 * @returns count number or highest Index of chosen Objecttype x
 */
uint8_t getCountOf(uint8_t devtype, bool returnHighestIndex)
{
	/*
	Serial.print(F("getCountOf()! "));	// DEBUG
	Serial.print(devtype);				// DEBUG
	Serial.print(F(" "));				// DEBUG
	Serial.println(returnHighestIndex); // DEBUG
	*/

	uint8_t cntr = 0;
	uint8_t hI = 0;

	for (uint8_t i = BEGINN_FILEINDEX; i < MAX_FILEINDEX_ITERATE_COUNT; i++)
	{
		switch (devtype)
		{
		case DEVTYPE_NAME_GPIO:
			if (accessGPIOObject(i) == EXIT_SUCCESS)
			{
				cntr++;
				hI = i;
			}

			break;
		case DEVTYPE_NAME_RF:
			if (accessRFObject(i) == EXIT_SUCCESS)
			{
				cntr++;
				hI = i;
			}
			break;
		case DEVTYPE_NAME_ALL:
			if (accessGPIOObject(i) == EXIT_SUCCESS)
			{
				cntr++;
				hI = i;
			}
			if (accessRFObject(i) == EXIT_SUCCESS)
			{
				cntr++;
				hI = i;
			}
			break;

		// DEVTYPES defines == devtype in EEPROM
		case DEVTYPE_GPIO:
		case DEVTYPE_DI_GENERIC:
		case DEVTYPE_DI_RF_REC:
		case DEVTYPE_DO_GENERIC:
		case DEVTYPE_DO_RF_SEND:
			if (accessRFObject(i) == EXIT_SUCCESS)
				if (tmp_gpio.devType == devtype)
				{
					cntr++;
					hI = i;
				}
			break;
		default:
			return EXIT_FAIL;
			break;
		}
	}

	/*
		Serial.print(F("getCountOf() result! ")); // DEBUG
		Serial.print(hI);						  // DEBUG
		Serial.print(F(" "));					  // DEBUG
		Serial.println(cntr);					  // DEBUG
	 */

	if (returnHighestIndex)
		return hI;
	else
		return cntr;
}

/** @brief Makes sure the given Objecttype and Objectnumber is valid. Also inits it. Attention: Overwrites corresponding buffer!
 * @param objectNr Target dataset - Nr of Object
 * @param objectType Objecttype defined in #define
 * @returns EXIT_FAIL or EXIT_SUCCESS - dependend on outcome of check and arm procedure
 */
uint8_t checkAndArmSingleDATASET(uint8_t objectNr, uint8_t objectType)
{
	/*
	Serial.print(F("checkAndArmSingleDATASET(): objectNr: ")); // DEBUG
	Serial.print(objectNr);
	Serial.print(" objectType: ");
	Serial.println(objectType); // DEBUG
	 */
	// ObjectCheck
	switch (objectType)
	{
	case DEVTYPE_GPIO:
		// A GPIO Object in General means to check the object and init the bound hardware
		if (accessGPIOObject(objectNr) == EXIT_SUCCESS)
		{
			// If found, check that
			if (verifyDevtypeToIO(tmp_gpio.pin, tmp_gpio.devType, false))
			{
				// Serial.println("GPIO-NR: " + String(objectNr) + " is vaild. Activating now!"); // DEBUG
				objectType = tmp_gpio.devType;
			}
			else
			{
				Serial.println(F("checkAndArmSingleDATASET(): Check failed! Resetting Object... "));
				tmp_gpio.devType = EXIT_SUCCESS;
				tmp_gpio.pin = EXIT_SUCCESS;
				if (updateGPIOObject(Input.VAL_NUM) != EXIT_SUCCESS)
					return EXIT_FAIL;
			}
		}
		else
		{
			Serial.println(F("checkAndArmSingleDATASET(): GPIO-Object not found!"));
			return EXIT_FAIL;
		}
		break;

	case DEVTYPE_RF:
		// A RF Object in General means to check the object and find any possible senders
		if (accessRFObject(objectNr) == EXIT_SUCCESS)
		{
			// Serial.println("RF-NR: " + String(objectNr) + " is vaild"); // DEBUG

			// TODO: Check all Ranges of all parameters

			objectType = DEVTYPE_DO_RF_SEND;
		}
		else
		{
			Serial.println(F("checkAndArmSingleDATASET(): RF-Object not found!"));
			return EXIT_FAIL;
		}
		break;
	default:
		// if nothing was given the function assumes the global object is preset
		break;
	}

	// Hardware-Arming
	switch (objectType)
	{
	case DEVTYPE_DI_GENERIC:
		pinMode(tmp_gpio.pin, INPUT);
		break;
	case DEVTYPE_DI_PULLUP:
		pinMode(tmp_gpio.pin, INPUT_PULLUP);
		break;
	case DEVTYPE_DI_RF_REC:
		// Find gpio-object to rec over
		objectNr = findFirstObjectByDevtype(DEVTYPE_DI_RF_REC);
		if (objectNr != EXIT_FAIL)
			reinitRF_RX();  // TODO: Even tho its implemented, it wont be registered and saved!
		else
			Serial.println(F("RF-RX-Init failed. No bound rx-pin found!"));
		break;
	case DEVTYPE_DO_GENERIC:
		pinMode(tmp_gpio.pin, OUTPUT);
		break;
	case DEVTYPE_DO_RF_SEND:
		// Find gpio-object to send over
		if (findFirstObjectByDevtype(DEVTYPE_DO_RF_SEND) == EXIT_FAIL || verifyDevtypeToIO(tmp_gpio.pin, DEVTYPE_DO_RF_SEND, false) == false)
		{
			Serial.println(F("RF-TX-Init failed. No bound tx-pin found!"));
			return EXIT_FAIL;
		}
		break;

	// TODO: Analog
	default:
		return EXIT_FAIL;
		break;
	}

	return EXIT_SUCCESS;
}

// ## WRITE FUNCTIONS ##

/** @brief Checks EEPROM-Health. Sends official message if bad situation.
 * @param address Pointer to EEPROM-Address-Cell
 * @param value Value that shall be written
 * @returns Damaged cell-address on error. Else: EXIT_SUCCESS
 */
uint8_t updateByteAndCheckHealth(uint8_t *address, uint8_t value)
{
	eeprom_update_byte(address, value);
	if (eeprom_read_byte(address) != value)
	{
		stringStorageToBuffer(COMMAND_STATE_CODE, COMMAND_STATE_STATUS, Output.COM, SIZE_SINGLE_INPUTBUFFER);
		stringStorageToBuffer(COMMAND_STATE_CODE, COMMAND_STATE_ERROR, Output.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
		String error = F("Celldamage detected @");
		error += String((uint16_t)address, 16);
		error.toCharArray(Output.VAL_STR, SIZE_SINGLE_INPUTBUFFER);
		sendSerialSTRINGNET();

		return EXIT_FAIL;
	}
	return EXIT_SUCCESS;
}

/** @brief Writes to EEPROM AND to systemmemory
 * @param newLifeSignIntervall New Intervall of LifeSign-Packages to be send. 0 disables it.
 * @param newNetworkMode Turns On/Off From2-Communcation/Networkmode
 * @returns
 * @note
 */
uint8_t writeSystemBits(uint32_t newLifeSignIntervall, bool newNetworkMode)
{
	uint8_t ret = EXIT_SUCCESS;
	if (updateByteAndCheckHealth((uint8_t *)(SIZE_MAGICBYTE), newNetworkMode) != EXIT_SUCCESS)
	{
		Serial.println(F("ERROR: Write of new NetworkMode failed!"));
		ret = EXIT_FAIL;
	}
	eeprom_update_dword((uint32_t *)(SIZE_MAGICBYTE + SIZE_NETWORKMODE), newLifeSignIntervall);
	if (eeprom_read_dword((uint32_t *)(SIZE_MAGICBYTE + SIZE_NETWORKMODE)) != newLifeSignIntervall)
	{
		Serial.println(F("ERROR: Write of LifesignInterval failed!"));
		ret = EXIT_FAIL;
	}

	return ret;
}

// Tells that storage is about to be reset and performs reset of OSFS and Systemsettings without harming EEPROM too much.
void fullStorageReset(void)
{
	// TELL
	stringStorageToBuffer(COMMAND_STATE_CODE, COMMAND_STATE_STATUS, Output.COM, SIZE_SINGLE_INPUTBUFFER);
	stringStorageToBuffer(COMMAND_STATE_CODE, COMMAND_STATE_WARN, Output.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
	String(F("Storage-Reset of Device...")).toCharArray(Output.VAL_STR, SIZE_SINGLE_INPUTBUFFER); // DEBUG
	sendSerialSTRINGNET();

	for (size_t i = 0; i < 4; i++)
	{
		digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
		delay(500); // DEBUG
	}

	// DO
	updateByteAndCheckHealth((uint8_t *)MAGICBYTE_INDEX, MAGICBYTE);
	OSFS::format();
	updateDevName((char *)DEVICE_TYPE, SIZE_SINGLE_INPUTBUFFER);
	writeSystemBits(STANDARD_LIFESIGNINTERVALL_MS, STANDARD_NETWORKMODE);
	loadSystemBits();
}

/** @brief Updates name of device/system in EEPROM
 * @param Name Source-string to be stored
 * @param len Length of source (safeguarded by max length in EEPROM)
 */
void updateDevName(char *Name, uint8_t len)
{
	// Serial.print(F("updateDevName()!: ")); // DEBUG
	// Serial.println(Name);				   // DEBUG

	if (len > SIZE_MAX_NAME)
		len = SIZE_MAX_NAME; // minimize to eeprom-len

	for (uint8_t i = 0; i < len; i++)
		if (i == (len - 1))
			updateByteAndCheckHealth((uint8_t *)(i + DEV_NAME_OFFSET), zeroTerm); // HAS to be zeroterminated
		else
			updateByteAndCheckHealth((uint8_t *)(i + DEV_NAME_OFFSET), (uint8_t)Name[i]);
}

/** @brief Writes/Creates GPIO-Object-Buffer to OSFS and check whether it was succcessfull. Attention: Needs preparation of tmp_gpio!
 * @param nr Number if GPIO-Object that shall be accessed
 * @returns EXIT_FAIL or EXIT_SUCCESS - depending on result
 */
uint8_t updateGPIOObject(uint8_t nr)
{
	/*
		Serial.print(F("updateGPIOObject()! Nr: ")); // DEBUG
		Serial.print(nr);
		Serial.print(F(" name: ")); // DEBUG
		Serial.print(tmp_gpio.name);
		Serial.print(F(" devtype: ")); // DEBUG
		Serial.print(tmp_gpio.devType);
		Serial.print(F(" pin: ")); // DEBUG
		Serial.println(tmp_gpio.pin);
		 */

	// Update
	String fileNameString = STANDARD_FILENAME_GPIO + String(nr);
	char fileName[OSFS::FILE_NAME_LENGTH] = {'\0'};
	fileNameString.toCharArray(fileName, OSFS::FILE_NAME_LENGTH);
	OSFS_result = OSFS::newFile(fileName, tmp_gpio, true);

	GPIO_Object check = tmp_gpio;

	// Check for successfull update
	if (accessGPIOObject(nr) == EXIT_SUCCESS && strcmp(check.name, tmp_gpio.name) == 0 && check.devType == tmp_gpio.devType && check.pin == tmp_gpio.pin)
		return printOSFSError(OSFS_result, nr, DEVTYPE_GPIO);
	else
	{
		Serial.println(F("ERROR: Update of GPIO-Object-Failed!"));
		return EXIT_FAIL;
	}
}

/** @brief Writes/Creates RF-Object-Buffer to OSFS and check whether it was succcessfull. Attention: Needs preparation of tmp_rf!
 * @param nr Number if RF-Object that shall be accessed
 * @returns EXIT_FAIL or EXIT_SUCCESS - depending on result
 */
uint8_t updateRFObject(uint8_t nr)
{
	/*
		Serial.println(F("updateRFObject()! Nr: ")); // DEBUG
		Serial.print(nr);
		Serial.print(F(" name: ")); // DEBUG
		Serial.print(tmp_rf.name);
		Serial.print(F(" rfoffSeq: ")); // DEBUG
		Serial.print(tmp_rf.rfoffSeq);
		Serial.print(F(" rfonSeq: ")); // DEBUG
		Serial.print(tmp_rf.rfonSeq);
		Serial.print(F(" rfproto: ")); // DEBUG
		Serial.print(tmp_rf.rfproto);
		Serial.print(F(" pulseln: ")); // DEBUG
		Serial.print(tmp_rf.pulseln);
		Serial.print(F(" repeatTX: ")); // DEBUG
		Serial.println(tmp_rf.repeatTX);
	 */

	String fileNameString = STANDARD_FILENAME_RF + String(nr);
	char fileName[OSFS::FILE_NAME_LENGTH] = {'\0'};
	fileNameString.toCharArray(fileName, OSFS::FILE_NAME_LENGTH);
	OSFS_result = OSFS::newFile(fileName, tmp_rf, true);

	RF_Object check = tmp_rf;

	// Check for successfull update
	if (accessRFObject(nr) == EXIT_SUCCESS && strcmp(check.name, tmp_rf.name) == 0 && check.rfoffSeq == tmp_rf.rfoffSeq && check.rfonSeq == tmp_rf.rfonSeq && check.rfproto == tmp_rf.rfproto && check.pulseln == tmp_rf.pulseln && check.repeatTX == tmp_rf.repeatTX)
		return printOSFSError(OSFS_result, nr, DEVTYPE_RF);
	else
	{
		Serial.println(F("ERROR: Update of RF-Object-Failed!"));
		return EXIT_FAIL;
	}
}

/** @brief Deletes a Object from OSFS and checks for success.
 * @param objectNr Target dataset - Nr of Object
 * @param objectType Objecttype defined in #define
 * @returns EXIT_FAIL or EXIT_SUCCESS - dependend on outcome of check procedure
 */
uint8_t deleteObject(uint8_t nr, uint8_t objecttype)
{
	// Serial.println(F("deleteObject()!")); // DEBUG

	char fileName[OSFS::FILE_NAME_LENGTH] = {'\0'};
	String fileNameString;

	switch (objecttype)
	{
	case DEVTYPE_GPIO:
		fileNameString = STANDARD_FILENAME_GPIO + String(nr);
		break;

	case DEVTYPE_RF:
		fileNameString = STANDARD_FILENAME_RF + String(nr);
		break;

	default:
		return EXIT_FAIL;
		break;
	}

	fileNameString.toCharArray(fileName, OSFS::FILE_NAME_LENGTH);
	OSFS_result = OSFS::deleteFile(fileName);
	printOSFSError(OSFS_result, nr, DEVTYPE_RF);

	// check whether deleted
	if (accessGPIOObject(nr) == EXIT_FAIL)
		return EXIT_SUCCESS;
	else
		return EXIT_FAIL;
}
