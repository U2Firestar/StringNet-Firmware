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

#ifndef RR_H
#define RF_H

#include <RCSwitch.h>
#include "common.hpp"

////////// RF FUNCTIONS ////////

//Init of RF - nothing to do here...
void initRF(void);

/** @brief Enables RF_Receiver by pin, leading to interruptnum. Needs preparation of tmp_gpio!
 * @returns EXIT_SUCCESS or EXIT_FAIL - dependend on pre-arming-check
 */
uint8_t reinitRF_RX(void);

//Returns bit corresponding (to nr) from of Statebuffer for RF-Objects
bool getCachedRemoteBit(uint8_t RFbitBufferBitNr);

//Updates bit corresponding (to nr) in Statebuffer for RF-Objects with given bool
void setCachedRemoteBit(uint8_t RFbitBufferBitNr, bool state);

//Toggles bit corresponding (to nr) in Statebuffer for RF-Objects
void toggleCachedRemoteBit(uint8_t RFbitBufferBitNr);

/** @brief Needs preparation of GPIO_Object AND RF_Object!; Inits always at use for flexibility
 * @param mode 0=off, 1=on, 2=toggle;
 * @param objectNr Number of RF-Object to request and update state
 * @returns EXIT_SUCCESS or EXIT_FAIL - depending operation-success (no gurantee for actual switching)
 * @Note Leave Values, besides Sequences, to 0 to work with standard-settings
 */
uint8_t transmitRF(uint8_t mode, uint8_t objectNr);

#endif