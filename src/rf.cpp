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

 * Softwarename: StringNet-IoT-Gateway - RF-433Mhz-related
 * Version: 2.0.0, 16.04.2022
 * Author: Emil Sedlacek (U2Firestar, Firestar)
 * Supported by UAS Technikum Vienna and 3S-Sedlak
 * Note: See Documentation, commandtable in Excle and Firmware-Source-Files for more info!
 */

#include "rf.hpp"

////////// SEMI-GLOBAL VARS /////////

extern GPIO_Object tmp_gpio; // see common.cpp
extern RF_Object tmp_rf;

RCSwitch RF_IO;			  // Object that handles RF-Actions based on given parameters
uint32_t RFbitBuffer = 0; // Memory to save last writen States of all binary RF-Objects - enables toggles

////////// RF FUNCTIONS ////////

// Init of RF
void initRF(void)
{
	// empty as globally or event-ually initialized
}

/** @brief Enables RF_Receiver by pin, leading to interruptnum. Needs preparation of tmp_gpio!
 * @returns EXIT_SUCCESS or EXIT_FAIL - dependend on pre-arming-check
 * @note TODO: Even tho its implemented, it wont be registered and saved!
 */
uint8_t reinitRF_RX(void)
{
	if (verifyDevtypeToIO(tmp_gpio.pin, DEVTYPE_DI_RF_REC, false))
	{
		RF_IO.enableReceive(verifyDevtypeToIO(tmp_gpio.pin, DEVTYPE_DI_RF_REC, true)); // returns index of array and therefor interruptnumber
		return EXIT_SUCCESS;
	}
	else
		Serial.println(F("reinitRF_RX(): Init failed. Wrong interrupt num!"));
	return EXIT_FAIL;
}

// Returns bit corresponding (to nr) from of Statebuffer for RF-Objects
bool getCachedRemoteBit(uint8_t RFbitBufferBitNr)
{
	return (RFbitBuffer & (1 << RFbitBufferBitNr)) ? true : false;
}

// Updates bit corresponding (to nr) in Statebuffer for RF-Objects with given bool
void setCachedRemoteBit(uint8_t RFbitBufferBitNr, bool state)
{
	if (getCachedRemoteBit(RFbitBufferBitNr)) // if 1 --> 0
		RFbitBuffer &= ~(1UL << RFbitBufferBitNr);

	else // 0 --> 1
		RFbitBuffer |= 1UL << RFbitBufferBitNr;
}

// Toggles bit corresponding (to nr) in Statebuffer for RF-Objects
void toggleCachedRemoteBit(uint8_t RFbitBufferBitNr)
{
	RFbitBuffer ^= 1UL << RFbitBufferBitNr;
}

/** @brief Needs preparation of GPIO_Object AND RF_Object!; Inits always at use for flexibility
 * @param mode 0=off, 1=on, 2=toggle;
 * @param objectNr Number of RF-Object to request and update state
 * @returns EXIT_SUCCESS or EXIT_FAIL - depending operation-success (no gurantee for actual switching)
 * @note Leave Values, besides Sequences, to 0 to work with standard-settings
 */
uint8_t transmitRF(uint8_t mode, uint8_t objectNr)
{

	// Serial.print(F("transmitRF()! mode: ")); // DEBUG
	// Serial.print(mode);						 // DEBUG
	// Serial.print(F(" tmp_gpio.name: "));	 // DEBUG
	// Serial.print(tmp_gpio.name);			 // DEBUG
	// Serial.print(F(" tmp_gpio.pin: "));		 // DEBUG
	// Serial.print(tmp_gpio.pin);				 // DEBUG
	// Serial.print(F(" tmp_rf.name: "));		 // DEBUG
	// Serial.print(tmp_rf.name);				 // DEBUG
	// Serial.print(F(" tmp_rf.rfoffSeq: "));	 // DEBUG
	// Serial.print(tmp_rf.rfoffSeq);			 // DEBUG
	// Serial.print(F(" tmp_rf.rfonSeq: "));	 // DEBUG
	// Serial.print(tmp_rf.rfonSeq);			 // DEBUG
	// Serial.print(F(" tmp_rf.rfproto: "));	 // DEBUG
	// Serial.print(tmp_rf.rfproto);			 // DEBUG
	// Serial.print(F(" tmp_rf.pulseln: "));	 // DEBUG
	// Serial.print(tmp_rf.pulseln);			 // DEBUG
	// Serial.print(F(" tmp_rf.repeatTX: "));	 // DEBUG
	// Serial.println(tmp_rf.repeatTX); // DEBUG

	// Arm Pin
	RF_IO.enableTransmit(tmp_gpio.pin);
	if (tmp_rf.rfproto > 0)
		RF_IO.setProtocol(tmp_rf.rfproto);
	if (tmp_rf.repeatTX > 0)
		RF_IO.setRepeatTransmit(tmp_rf.repeatTX);
	if (tmp_rf.pulseln > 0)
		RF_IO.setPulseLength(tmp_rf.pulseln);

	if (mode == SEND_TOGGLE)
	{
		if (getCachedRemoteBit(objectNr))
			mode = SEND_OFF;
		else
			mode = SEND_ON;
	}

	switch (mode)
	{
	case SEND_OFF:
		if (tmp_rf.rfproto == MUMBI_RF_PROTO)
			RF_IO.send(tmp_rf.rfoffSeq, MUMBI_RF_BITLEN);
		else
			RF_IO.send(tmp_rf.rfoffSeq, STOCK_RF_BITLEN);
		setCachedRemoteBit(objectNr, false);
		break;
	case SEND_ON:
		if (tmp_rf.rfproto == MUMBI_RF_PROTO)
			RF_IO.send(tmp_rf.rfonSeq, MUMBI_RF_BITLEN);
		else
			RF_IO.send(tmp_rf.rfonSeq, STOCK_RF_BITLEN);
		setCachedRemoteBit(objectNr, true);
		break;
	default:
		Serial.println(F("Defaulterror in transmitRF()!")); // DEBUG
		return EXIT_FAIL;
		break;
	}

	// Disarm Pin
	RF_IO.disableTransmit();

	return EXIT_SUCCESS;
}