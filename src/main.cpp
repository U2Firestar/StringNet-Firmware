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

 * Softwarename: StringNet-IoT-Gateway - Main to launch and loop tasks
 * Version: 2.0.0, 16.04.2022
 * Author: Emil Sedlacek (U2Firestar, Firestar) (U2Firestar, Firestar)
 * Supported by UAS Technikum Vienna and 3S-Sedlak
 * Note: See Documentation, commandtable in Excle and Firmware-Source-Files for more info!
 */

#include "common.hpp"

extern GPIO_Object tmp_gpio;
extern RF_Object tmp_rf;

void setup(void)
{
    // Partsspecific
    initConnectivity();
    initEEPROM();
    initRF();

    // DEBUG
    pinMode(LED_BUILTIN, OUTPUT); // DBEUG
    /*
    //PreCreate LED - usefull for internalStringNetblinker()
    strcpy(tmp_gpio.name, "LED");
    tmp_gpio.devType = DEVTYPE_DO_GENERIC;
    tmp_gpio.pin = 13;
    updateGPIOObject(1);
    checkAndArmSingleDATASET(1, DEVTYPE_GPIO);
    */
}

void loop(void)
{
    ////Regular-Tasks
    inputHandler();
    lifesignHandler();
    //TODO: Regular RCSwitch Receive-availability-check - Not going to happen

    ////TESTs & DEBUG
    // internalStringNetblinker();

    // // ECHO
    // if (Serial.available() > 0)
    // {
    //     Serial.print((char)Serial.read());
    //     digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    // }
}