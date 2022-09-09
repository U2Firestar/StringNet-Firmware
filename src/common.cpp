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

 * Softwarename: StringNet-IoT-Gateway - Commandcenter and IO
 * Version: 2.0.0, 16.04.2022
 * Author: Emil Sedlacek (U2Firestar, Firestar)
 * Supported by UAS Technikum Vienna and 3S-Sedlak
 * Note: See Documentation, commandtable in Excle and Firmware-Source-Files for more info!
 */

#include "common.hpp"

////////// SEMI-GLOBAL VARS /////////

unsigned long previousMillis[2] = {0};    // Timer variable for regular Lifesign [0] and DEBUG [1]
char buff[SIZE_SINGLE_INPUTBUFFER] = {0}; // Generic Buffer

bool NetworkMode = STANDARD_NETWORKMODE;                       // true: Dev reacts only if it was addressed; false: Increases Perf. at P2P-Connections as no addressing performed; Attention! P2P-mode might create Loop in a Network!
uint32_t lifesign_interval_ms = STANDARD_LIFESIGNINTERVALL_MS; // controlles lifesign: 0 = off; x > 0 = ON;

uint8_t inputBufferCntr = 0;                 // Counter for buffering characters in Input-Buffer
uint8_t bufferState = INPUT_BUFFERMODE_WAIT; // Sets mode and current Input-Buffer to be written to
uint8_t tmp = 0;                             // Buffer for character

GPIO_Object tmp_gpio; // Firmware-Wide Memory-Buffer for EEPROM-GPIO-Objects
RF_Object tmp_rf;     // Firmware-Wide Memory-Buffer for EEPROM-RF-Objects

stringNetObject Input;  // Firmware-Wide Memory-Buffer for StringNet-Package-Inputs and Interpretations
stringNetObject Output; // Firmware-Wide Memory-Buffer for StringNet-Package-Outputs

////////// Value MATRIXes and Objects /////// - Note: Systemwide needed Strings and Values stored in EEPROM or MEMORY (accessed through functions)

#ifdef ARDUINO_AVR_NANO                                           // Blocks if not checked
const uint8_t PINS_DIGITAL_DNUM[] = {2, 3, 4, 5, 6, 7, 8, 9, 13}; // Physical D<x>-Pins of Arduino Nano
const uint8_t PINS_INTERRUPT[] = {2, 3};                          // Physical D<x>-Pins of Arduino Nano - Index of Array determines interruptnumber
// TODO: Analog pins
#else
#error "No valid Pins set!"
#endif

// StringNet-Object/Message-Positions

const char string_MSGPARAM0[] PROGMEM = "FROM";
const char string_MSGPARAM1[] PROGMEM = "TO";
const char string_MSGPARAM2[] PROGMEM = "COM";
const char string_MSGPARAM3[] PROGMEM = "SUBCOM";
const char string_MSGPARAM4[] PROGMEM = "VAL_STR";
const char string_MSGPARAM5[] PROGMEM = "VAL_NUM";
const char *const string_table_MSGPARAM[] PROGMEM = {string_MSGPARAM0, string_MSGPARAM1, string_MSGPARAM2, string_MSGPARAM3, string_MSGPARAM4, string_MSGPARAM5};

// COMMANDs

const char string_COM0[] PROGMEM = "FORMAT"; // "String 0" etc are strings to store - change to suit.
const char string_COM1[] PROGMEM = "NAME";
const char string_COM2[] PROGMEM = "LIFESIGN";
const char string_COM3[] PROGMEM = "NETMODE";
const char string_COM4[] PROGMEM = "DISCOVER";
const char string_COM5[] PROGMEM = "CREATE";
const char string_COM6[] PROGMEM = "DELETE"; // "String 0" etc are strings to store - change to suit.
const char string_COM7[] PROGMEM = "TYPE";
const char string_COM8[] PROGMEM = "PIN";
const char string_COM9[] PROGMEM = "ONSEQUENCE";
const char string_COM10[] PROGMEM = "OFFSEQUENCE";
const char string_COM11[] PROGMEM = "PROTOCOLL";
const char string_COM12[] PROGMEM = "PULSLENGTH"; // "String 0" etc are strings to store - change to suit.
const char string_COM13[] PROGMEM = "RF_TX_REP";
const char *const string_table_COM[] PROGMEM = {string_COM0, string_COM1, string_COM2, string_COM3, string_COM4, string_COM5, string_COM6, string_COM7, string_COM8, string_COM9, string_COM10, string_COM11, string_COM12, string_COM13};

// SUB-COMMANDs

const char string_SUBCOM0[] PROGMEM = "TELLALL";
const char string_SUBCOM1[] PROGMEM = "TELLDEV";
const char string_SUBCOM2[] PROGMEM = "TELLGPIO";
const char string_SUBCOM3[] PROGMEM = "TELLRF";
const char string_SUBCOM4[] PROGMEM = "SETDEV";
const char string_SUBCOM5[] PROGMEM = "SETGPIO";
const char string_SUBCOM6[] PROGMEM = "SETRF";
const char *const string_table_SUBCOM[] PROGMEM = {string_SUBCOM0, string_SUBCOM1, string_SUBCOM2, string_SUBCOM3, string_SUBCOM4, string_SUBCOM5, string_SUBCOM6};

// STATES

const char string_STATE0[] PROGMEM = "ON";
const char string_STATE1[] PROGMEM = "SilentON";
const char string_STATE2[] PROGMEM = "OFF";
const char string_STATE3[] PROGMEM = "SilentOFF";
const char string_STATE4[] PROGMEM = "TOGGLE";
const char string_STATE5[] PROGMEM = "SilentTOGGLE";
const char string_STATE6[] PROGMEM = "STATUS";
const char string_STATE7[] PROGMEM = "ERROR";
const char string_STATE8[] PROGMEM = "WARNING";
const char *const string_table_STATE[] PROGMEM = {string_STATE0, string_STATE1, string_STATE2, string_STATE3, string_STATE4, string_STATE5, string_STATE6, string_STATE7, string_STATE8};

// DEVTYPES

const char string_DEVTYPE0[] PROGMEM = "DI_GENERIC";
const char string_DEVTYPE1[] PROGMEM = "DI_PULLUP";
const char string_DEVTYPE2[] PROGMEM = "DI_RF";
const char string_DEVTYPE3[] PROGMEM = "DO_GENERIC";
const char string_DEVTYPE4[] PROGMEM = "DO_RF";
const char string_DEVTYPE5[] PROGMEM = "AI";
const char string_DEVTYPE6[] PROGMEM = "AO";
const char string_DEVTYPE7[] PROGMEM = "GPIO";
const char string_DEVTYPE8[] PROGMEM = "RF";
const char string_DEVTYPE9[] PROGMEM = "NAME_ALL";
const char string_DEVTYPE10[] PROGMEM = "NAME_GPIO";
const char string_DEVTYPE11[] PROGMEM = "NAME_RF";
const char *const string_table_DEVTYPE[] PROGMEM = {string_DEVTYPE0, string_DEVTYPE1, string_DEVTYPE2, string_DEVTYPE3, string_DEVTYPE4, string_DEVTYPE5, string_DEVTYPE6, string_DEVTYPE7, string_DEVTYPE8, string_DEVTYPE9, string_DEVTYPE10, string_DEVTYPE11};

////////// HELPER /////////

// Converts given String to uint32_t. Attention(!): Cuts off bits over 2^32 and also returns 0 in Errorcase!!!
uint32_t string2Number(String cp)
{
    char buff[LEN_String_32BIT] = {0};
    cp.toCharArray(buff, LEN_String_32BIT);
    return atol(buff);
}

/** @brief Converts a value to String with binary meaning
 * @returns "ON" (> 0) or "OFF" (= 0)
 */
String boolToBinaryString(uint8_t value)
{
    if (value > 0)
        return F("ON");
    else
        return F("OFF");
}

/** @brief CLEAN way to load all tables from Flash
 * @param tablecode Given Value in #define for table (is safeguarded for interation)
 * @param index Index of predefined table (is safeguarded for interation)
 * @param ptr Target to be written to
 * @param buffLen Length of target buffer
 * @return #defined EXIT_SUCCUESS or EXIT_FAIL
 * @note , posssible input: indexes (0..x) corresponding to the <table>_values OR <table>_values themselfs
 */
uint8_t stringStorageToBuffer(uint8_t tablecode, uint8_t index, char *ptr, uint8_t buffLen)
{
    uint8_t retrn = EXIT_SUCCESS;

    if (memset(ptr, zeroTerm, buffLen) != NULL)
        switch (tablecode)
        {
        case COMMAND_MSGPARAM_CODE:
            if (index >= 0 && index < COMMAND_MSGPARAM_COUNT)
            {
                strncpy_P(ptr, (char *)pgm_read_word(&(string_table_MSGPARAM[index])), buffLen);
            }
            break;

        case COMMAND_COMMAND_CODE:
            if (index >= 0 && index < COMMAND_COUNT)
            {
                strncpy_P(ptr, (char *)pgm_read_word(&(string_table_COM[index])), buffLen);
            }
            break;

        case COMMAND_SUB_CODE:
            if (index >= 0 && index < SUBCOMMAND_COUNT)
            {
                strncpy_P(ptr, (char *)pgm_read_word(&(string_table_SUBCOM[index])), buffLen);
            }
            break;

        case COMMAND_STATE_CODE:
            if (index >= 0 && index < STATUS_COUNT)
            {
                strncpy_P(ptr, (char *)pgm_read_word(&(string_table_STATE[index])), buffLen);
            }
            break;

        case COMMAND_DEVTYPE_CODE:
            if (index >= 0 && index < DEVTYPE_COUNT)
            {
                strncpy_P(ptr, (char *)pgm_read_word(&(string_table_DEVTYPE[index])), buffLen);
            }
            break;
        default:
            Serial.println(F("stringStorageToBuffer(): Default-Error!"));
            retrn = EXIT_FAIL;
            break;
        }
    else
        retrn = EXIT_FAIL;

    return retrn;
}

/** @brief  Compares input to table
 * @param pch Pointer to Source-string
 * @param tablecode Given Value in #define for table (is safeguarded for interation)
 * @attention Chararray of *pch NEED to have a String-Terminator, otherwise there could be bad mem-access
 * @returns Corresponding tableindex or EXIT_FAIL on error;
 */
uint8_t tableMatch(char *pch, uint8_t tablecode)
{
    uint8_t COUNT = 0;

    if (strlen(pch) > 0)
    {
        switch (tablecode)
        {
        case COMMAND_MSGPARAM_CODE:
            COUNT = COMMAND_MSGPARAM_COUNT;
            // Serial.print(F("MessageParam-Matching with: ")); // DEBUG
            break;
        case COMMAND_COMMAND_CODE:
            COUNT = COMMAND_COUNT;
            // Serial.print(F("Command-Matching with: ")); // DEBUG
            break;
        case COMMAND_SUB_CODE:
            COUNT = SUBCOMMAND_COUNT;
            // Serial.print(F("Subcommand-Matching with: ")); // DEBUG
            break;
        case COMMAND_STATE_CODE:
            COUNT = STATUS_COUNT;
            // Serial.print(F("Status-Matching with: ")); // DEBUG
            break;
        case COMMAND_DEVTYPE_CODE:
            COUNT = DEVTYPE_COUNT;
            // Serial.print(F("Devtype-Matching with: ")); // DEBUG
            break;
        default:
            return EXIT_FAIL;
            break;
        }

        for (uint8_t i = 0; i < COUNT; i++)
        {
            stringStorageToBuffer(tablecode, i, buff, SIZE_SINGLE_INPUTBUFFER);
            /*
            // DEBUG
            Serial.print(pch);
            Serial.print(F(" =?= "));
            Serial.println(buff);
            // DEBUG
             */
            if (strcasecmp(pch, buff) == 0)
                return i;
        }
    }

    return EXIT_FAIL;
}

/** @brief
 * @param pin Pin to compare to the predefined
 * @param devtype Type of usage of the physical object
 * @param returnIndex Whether it should be a Index (predefined array) or a Boolean value
 * @returns Index or a Boolean value - dependend on success
 */
uint8_t verifyDevtypeToIO(uint8_t pin, uint8_t devtype, bool returnIndex)
{
    switch (devtype)
    {
    case DEVTYPE_DI_GENERIC:
    case DEVTYPE_DI_PULLUP:
    case DEVTYPE_DO_GENERIC:
    case DEVTYPE_DO_RF_SEND:
        for (uint8_t i = 0; i < (sizeof(PINS_DIGITAL_DNUM) / sizeof(*PINS_DIGITAL_DNUM)); i++)
        {
            if (PINS_DIGITAL_DNUM[i] == pin)
            {
                if (returnIndex)
                    return i;
                else
                    return true;
            }
        }

        break;
    case DEVTYPE_DI_RF_REC:
        for (uint8_t i = 0; i < (sizeof(PINS_INTERRUPT) / sizeof(*PINS_INTERRUPT)); i++)
            if (PINS_INTERRUPT[i] == pin)
            {
                if (returnIndex)
                    return i;
                else
                    return true;
            }
        break;
    default:
        Serial.print(F("verifyDevtypeToIO(): Default-Error!")); // DEBUG

        if (returnIndex)
            return EXIT_FAIL;
        else
            return false;
        break;
    }

    // Serial.println(F("verifyDevtypeToIO(): Result=Fail")); // DEBUG

    if (returnIndex)
        return EXIT_FAIL;
    else
        return false;
}

////////// COMMANDCENTER /////////

// Sets up Serial-Connection
void initConnectivity(void)
{
    Serial.begin(DEVICE_BAUDRATE);
    do
    {
        for (size_t i = 0; i < 2; i++)
        {
            digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
            delay(50); // DEBUG
        }
    } while (!Serial); // wait for serial port to connect. Needed for native USB port only
}

// Dependend on the systems intervall-value and the timer, a StringNet-LifeSign-Message will be send periodically
void lifesignHandler(void)
{
    if (lifesign_interval_ms > 0 && ((unsigned)(millis() - previousMillis[0]) >= lifesign_interval_ms))
    {
        previousMillis[0] = millis();
        stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_NAME, Output.COM, SIZE_SINGLE_INPUTBUFFER);
        stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLDEV, Output.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
        DevNameToBuffer(Output.VAL_STR, SIZE_SINGLE_INPUTBUFFER);
        sendSerialSTRINGNET();
    }
}

// DEBUG-Function to test interpretation-System internally
void internalStringNetblinker(void)
{
    if ((unsigned)(millis() - previousMillis[1]) >= 1000)
    {
        previousMillis[1] = millis();
        strcpy(Input.COM, "LED");
        stringStorageToBuffer(COMMAND_STATE_CODE, COMMAND_STATE_TOGGLE_Silent, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
        interpretInput();
        delay(250);
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
}

/** @brief Handles connection input and protocoll, fills global stringnetobject Input and eventually triggers interpretation
 * @note Saving the Number-String and converting to variable is important as only the String-String is saved in FW-Memory
 * @attention Can break mid-package if there is a '{'
 */
void inputHandler(void)
{
    if (Serial.available())
    {
        tmp = (char)Serial.read();
        // Serial.print(F("Got: "));  // DEBUG
        // Serial.println((char)tmp); // DEBUG

        // Errorcase: There should not be too long packets!
        if (inputBufferCntr >= SIZE_SINGLE_INPUTBUFFER)
        {
            Serial.println(F("inputHandler(): Buffer-Overflow! Resetting Input...")); // DEBUG
            bufferState = INPUT_BUFFERMODE_WAIT;
        }
        else
            switch (bufferState)
            {
            case INPUT_BUFFERMODE_WAIT:
            case INPUT_BUFFERMODE_RECORD_FROM:
            case INPUT_BUFFERMODE_RECORD_TO:
            case INPUT_BUFFERMODE_RECORD_COM:
            case INPUT_BUFFERMODE_RECORD_SUBCOM:
            case INPUT_BUFFERMODE_RECORD_VAL_STR:
            case INPUT_BUFFERMODE_RECORD_VAL_NUM:
                // let chars pass - except...
                if (tmp == seqStart)
                { // check for Start-Sequence
                    inputBufferCntr = 0;
                    if (NetworkMode)
                        bufferState = INPUT_BUFFERMODE_RECORD_FROM;
                    else
                        bufferState = INPUT_BUFFERMODE_RECORD_COM;
                    Input = stringNetObject();
                    break;
                }
                else if (tmp == stringDelim || tmp == seqEnd)
                {
                    bufferState++;
                    if (bufferState == INPUT_BUFFERMODE_RECORD_VAL_NUM) // Before second val is recorded we convert buffer to number
                        Input.VAL_NUM = string2Number(Input.VAL_STR);
                    inputBufferCntr = 0;
                }
                else
                    switch (bufferState)
                    {
                    case INPUT_BUFFERMODE_RECORD_FROM:
                        Input.FROM[inputBufferCntr++] = tmp;
                        break;
                    case INPUT_BUFFERMODE_RECORD_TO:
                        Input.TO[inputBufferCntr++] = tmp;
                        break;
                    case INPUT_BUFFERMODE_RECORD_COM:
                        Input.COM[inputBufferCntr++] = tmp;
                        break;
                    case INPUT_BUFFERMODE_RECORD_SUBCOM:
                        Input.SUBCOM[inputBufferCntr++] = tmp;
                        break;
                    case INPUT_BUFFERMODE_RECORD_VAL_NUM:
                    case INPUT_BUFFERMODE_RECORD_VAL_STR:
                        Input.VAL_STR[inputBufferCntr++] = tmp;
                        break;

                    default:
                        bufferState = INPUT_BUFFERMODE_WAIT;
                        break;
                    }

                if (tmp != seqEnd) // and if that char is there we directly go to...
                    break;
            case INPUT_BUFFERMODE_INTERPRET:
                //// ... check addressing, prepair for Interpret
                if (NetworkMode)
                {
                    // Is this device addressed?
                    DevNameToBuffer(Output.FROM, SIZE_SINGLE_INPUTBUFFER);
                    if (strcasecmp(Input.TO, Output.FROM) == 0)
                    {
                        if (strlen(Input.FROM) > 1)
                            strncpy(Output.TO, Input.FROM, SIZE_SINGLE_INPUTBUFFER);
                        else
                            // Not valid, so we will answer to everyone
                            strncpy(Output.TO, DEVICE_STANDARDTARGET_NAME, SIZE_SINGLE_INPUTBUFFER);
                    }
                    // Nope... So, drop package
                    else
                    {
                        bufferState = INPUT_BUFFERMODE_WAIT;
                        break;
                    }
                }

                interpretInput();
                bufferState = INPUT_BUFFERMODE_WAIT;
                break;
            default:
                Serial.print(F("inputHandler(): Default-Error! Bufferstate: ")); // DEBUG
                Serial.print(bufferState);
                Serial.print(F(" | Got char: "));
                Serial.println(tmp);
                bufferState = INPUT_BUFFERMODE_WAIT;
                break;
            }
    }
}

/** @brief Based on the strings in the Input-Buffers the FW stores, deletes, controls and errorhandles according to the given StringNet-Message. Requires the Input-Buffer-Object to be prepaired!
 * @note Can also be used to trigger actions internally.
 */
uint8_t interpretInput(void)
{
    // DEBUG
    // Serial.print(Input.COM);
    // Serial.print(stringDelim);
    // Serial.print(Input.SUBCOM);
    // Serial.print(stringDelim);
    // Serial.print(Input.VAL_NUM);
    // Serial.print(stringDelim);
    // Serial.print(Input.VAL_STR);
    // Serial.print(F("\t\t"));
    // Serial.print(Output.COM);
    // Serial.print(stringDelim);
    // Serial.print(Output.SUBCOM);
    // Serial.print(stringDelim);
    // Serial.print(Output.VAL_NUM);
    // Serial.print(stringDelim);
    // Serial.println(Output.VAL_STR);
    // DEBUG

    uint8_t returnStatus = EXIT_FAIL; // function for interpretation need tell explicit success-report
    String stringer;
    stringer.reserve(SIZE_SINGLE_INPUTBUFFER);

    // Cleans buffer before use
    tmp_gpio = GPIO_Object();
    tmp_rf = RF_Object();

    // In some functions where Index of object is needed via VAL_NUM: check whether VAL_STR is correct & konvert as Number to update value
    switch (tableMatch(Input.COM, COMMAND_COMMAND_CODE))
    {
    //#### System ####
    case COMMAND_FORMAT:
        if (Input.VAL_NUM == MAGICBYTE)
            fullStorageReset();

        returnStatus = EXIT_SUCCESS;
        break;

    case COMMAND_LIFESIGN:
        switch (tableMatch(Input.SUBCOM, COMMAND_SUB_CODE))
        {
        case COMMAND_SUB_TELLDEV:
            stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_LIFESIGN, Output.COM, SIZE_SINGLE_INPUTBUFFER);
            stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLDEV, Output.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
            Output.VAL_NUM = lifesign_interval_ms;
            boolToBinaryString(lifesign_interval_ms).toCharArray(Output.VAL_STR, SIZE_SINGLE_INPUTBUFFER);
            sendSerialSTRINGNET();

            returnStatus = EXIT_SUCCESS;
            break;

        case COMMAND_SUB_SETDEV:
            // extract value validate and update
            if (writeSystemBits(Input.VAL_NUM, NetworkMode) == EXIT_SUCCESS)
            {
                loadSystemBits();

                // show status
                stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_LIFESIGN, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLDEV, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                interpretInput();

                returnStatus = EXIT_SUCCESS;
            }
            break;
        }
        break;

    case COMMAND_STRINGNET_MODE:
        switch (tableMatch(Input.SUBCOM, COMMAND_SUB_CODE))
        {
        case COMMAND_SUB_TELLDEV:
            stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_STRINGNET_MODE, Output.COM, SIZE_SINGLE_INPUTBUFFER);
            stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLDEV, Output.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
            Output.VAL_NUM = 0;
            if (NetworkMode)
                stringStorageToBuffer(COMMAND_STATE_CODE, COMMAND_STATE_ON, Output.VAL_STR, SIZE_SINGLE_INPUTBUFFER);
            else
                stringStorageToBuffer(COMMAND_STATE_CODE, COMMAND_STATE_OFF, Output.VAL_STR, SIZE_SINGLE_INPUTBUFFER);
            sendSerialSTRINGNET();

            returnStatus = EXIT_SUCCESS;
            break;

        case COMMAND_SUB_SETDEV:
            // extract value validate and update
            switch (tableMatch(Input.VAL_STR, COMMAND_STATE_CODE))
            {
            case COMMAND_STATE_ON:
                NetworkMode = true;
                if (writeSystemBits(lifesign_interval_ms, true) != EXIT_SUCCESS)
                    returnStatus = EXIT_FAILURE;
                loadSystemBits();
                break;

            case COMMAND_STATE_OFF:
                if (writeSystemBits(lifesign_interval_ms, false) != EXIT_SUCCESS)
                    returnStatus = EXIT_FAILURE;
                loadSystemBits();
                break;

            default:
                returnStatus = EXIT_FAILURE;
                break;
            }

            // show status
            if (returnStatus != EXIT_FAILURE)
            {
                stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_STRINGNET_MODE, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLDEV, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                interpretInput();

                returnStatus = EXIT_SUCCESS;
            }
            else
                returnStatus = EXIT_FAIL;
            break;

        default:
            break;
        }

        break;

    //#### Object-Config ####
    case COMMAND_CREATE:
        // First: Create = OVerwrite = Update to unit mode
        switch (tableMatch(Input.SUBCOM, COMMAND_SUB_CODE))
        {
        case COMMAND_SUB_SETGPIO:
            // try input, but if there is a object, find next free index
            if (accessGPIOObject(Input.VAL_NUM) != EXIT_SUCCESS)
                Input.VAL_NUM = findNextFreeIndex(DEVTYPE_GPIO);

            if (Input.VAL_NUM != EXIT_FAIL && Input.VAL_NUM >= BEGINN_FILEINDEX)
            {
                if (updateGPIOObject(Input.VAL_NUM) == EXIT_SUCCESS)
                {
                    // Tell
                    stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_NAME, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                    stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLGPIO, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                    interpretInput();

                    returnStatus = EXIT_SUCCESS;
                }
            }

            break;

        case COMMAND_SUB_SETRF:
            // try input, but if there is a object, find next free index
            if (accessRFObject(Input.VAL_NUM) != EXIT_SUCCESS)
                Input.VAL_NUM = findNextFreeIndex(DEVTYPE_RF);

            if (updateRFObject(Input.VAL_NUM) == EXIT_SUCCESS)
            {
                // Tell
                stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_NAME, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLRF, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                interpretInput();

                returnStatus = EXIT_SUCCESS;
            }
            break;

        default:
            break;
        }
        break;

    case COMMAND_DELETE:
        switch (tableMatch(Input.SUBCOM, COMMAND_SUB_CODE))
        {
        case COMMAND_SUB_SETGPIO:
            if (accessGPIOObject(Input.VAL_NUM) == EXIT_SUCCESS && deleteObject(Input.VAL_NUM, DEVTYPE_GPIO) == EXIT_SUCCESS)
            {
                // Acknowledge - if there was something it was deleted
                stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_DELETE, Output.COM, SIZE_SINGLE_INPUTBUFFER);
                stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLGPIO, Output.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                Output.VAL_NUM = Input.VAL_NUM;
                sendSerialSTRINGNET();

                returnStatus = EXIT_SUCCESS;
            }
            break;

        case COMMAND_SUB_SETRF:
            if (accessRFObject(Input.VAL_NUM) == EXIT_SUCCESS && deleteObject(Input.VAL_NUM, DEVTYPE_RF) == EXIT_SUCCESS)
            {
                // Acknowledge - if there was something it was deleted
                stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_DELETE, Output.COM, SIZE_SINGLE_INPUTBUFFER);
                stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLRF, Output.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                Output.VAL_NUM = Input.VAL_NUM;
                sendSerialSTRINGNET();

                returnStatus = EXIT_SUCCESS;
            }
            break;

        default:
            break;
        }
        break;

    case COMMAND_DISCOVER:
        switch (tableMatch(Input.SUBCOM, COMMAND_SUB_CODE))
        {
        case COMMAND_SUB_TELLALL:
            // TELL DEV NAME
            stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_NAME, Input.COM, SIZE_SINGLE_INPUTBUFFER);
            stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLDEV, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
            interpretInput();

            // TELL available GPIO NAMES plus Status
            for (uint8_t i = BEGINN_FILEINDEX; i <= getCountOf(DEVTYPE_NAME_GPIO, true); i++)
            {
                if (accessGPIOObject(i) == EXIT_SUCCESS)
                {
                    // Name
                    stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_NAME, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                    stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLGPIO, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                    Input.VAL_NUM = i;
                    interpretInput();

                    // Status
                    strncpy(Input.COM, tmp_gpio.name, SIZE_SINGLE_INPUTBUFFER);
                    stringStorageToBuffer(COMMAND_STATE_CODE, COMMAND_STATE_STATUS, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                    interpretInput();
                }
            }

            // TELL available RF NAMES plus Status
            for (uint8_t i = BEGINN_FILEINDEX; i <= getCountOf(DEVTYPE_NAME_RF, true); i++)
            {
                if (accessRFObject(i) == EXIT_SUCCESS)
                {
                    // Name
                    stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_NAME, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                    stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLRF, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                    Input.VAL_NUM = i;
                    interpretInput();

                    // Status
                    strncpy(Input.COM, tmp_rf.name, SIZE_SINGLE_INPUTBUFFER);
                    stringStorageToBuffer(COMMAND_STATE_CODE, COMMAND_STATE_STATUS, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                    interpretInput();
                }
            }

            returnStatus = EXIT_SUCCESS;
            break;

        case COMMAND_SUB_TELLGPIO:
            // recursivle list all details
            if (Input.VAL_NUM >= BEGINN_FILEINDEX && accessGPIOObject(Input.VAL_NUM) == EXIT_SUCCESS)
            {
                stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_NAME, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLGPIO, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                interpretInput();

                stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_DEVTYPE, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLGPIO, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                interpretInput();

                stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_PIN, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLGPIO, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                interpretInput();

                // Status
                strncpy(Input.COM, tmp_gpio.name, SIZE_SINGLE_INPUTBUFFER);
                stringStorageToBuffer(COMMAND_STATE_CODE, COMMAND_STATE_STATUS, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                interpretInput();

                returnStatus = EXIT_SUCCESS;
            }
            break;

        // recursivle list all details
        case COMMAND_SUB_TELLRF:
            if (Input.VAL_NUM >= BEGINN_FILEINDEX && accessRFObject(Input.VAL_NUM) == EXIT_SUCCESS)
            {
                stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_NAME, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLRF, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                interpretInput();

                stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_RFONSEQ, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLRF, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                interpretInput();

                stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_RFOFFSEQ, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLRF, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                interpretInput();

                stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_RFPROTO, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLRF, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                interpretInput();

                stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_RFPULSLEN, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLRF, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                interpretInput();

                stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_RFTXREP, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLRF, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                interpretInput();

                // Status
                strncpy(Input.COM, tmp_rf.name, SIZE_SINGLE_INPUTBUFFER);
                stringStorageToBuffer(COMMAND_STATE_CODE, COMMAND_STATE_STATUS, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                interpretInput();

                returnStatus = EXIT_SUCCESS;
                break;
            }
        default:
            break;
        }

        break;

    case COMMAND_NAME:
        switch (tableMatch(Input.SUBCOM, COMMAND_SUB_CODE))
        {
        case COMMAND_SUB_TELLDEV:
            stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_NAME, Output.COM, SIZE_SINGLE_INPUTBUFFER);
            stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLDEV, Output.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
            DevNameToBuffer(Output.VAL_STR, SIZE_SINGLE_INPUTBUFFER);
            Output.VAL_NUM = Input.VAL_NUM;
            sendSerialSTRINGNET();

            returnStatus = EXIT_SUCCESS;
            break;

        case COMMAND_SUB_TELLGPIO:
            if (Input.VAL_NUM >= BEGINN_FILEINDEX && accessGPIOObject(Input.VAL_NUM) == EXIT_SUCCESS)
            {
                stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_NAME, Output.COM, SIZE_SINGLE_INPUTBUFFER);
                stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLGPIO, Output.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                strncpy(Output.VAL_STR, tmp_gpio.name, SIZE_SINGLE_INPUTBUFFER);
                Output.VAL_NUM = Input.VAL_NUM;
                sendSerialSTRINGNET();

                returnStatus = EXIT_SUCCESS;
            }
            break;

        case COMMAND_SUB_TELLRF:
            if (accessRFObject(Input.VAL_NUM) == EXIT_SUCCESS && Input.VAL_NUM >= BEGINN_FILEINDEX)
            {
                stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_NAME, Output.COM, SIZE_SINGLE_INPUTBUFFER);
                stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLRF, Output.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                strncpy(Output.VAL_STR, tmp_rf.name, SIZE_SINGLE_INPUTBUFFER);
                Output.VAL_NUM = Input.VAL_NUM;
                sendSerialSTRINGNET();

                returnStatus = EXIT_SUCCESS;
            }
            break;

        case COMMAND_SUB_SETDEV:
            if (strlen(Input.VAL_STR) >= 3) // Minimum 3 Chars to ensure it was meant
            {
                updateDevName(Input.VAL_STR, SIZE_SINGLE_INPUTBUFFER);

                // continue regardless of errors to show new State
                stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_NAME, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLDEV, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                interpretInput();

                returnStatus = EXIT_SUCCESS;
            }
            break;

        case COMMAND_SUB_SETGPIO:
            if (strlen(Input.VAL_STR) >= 3) // Minimum 3 Chars to ensure it was meant and not in the System
            {
                // Check whether Name is in System
                if (findObjectByName(Input.VAL_STR, DEVTYPE_GPIO) == EXIT_FAIL && findObjectByName(Input.VAL_STR, DEVTYPE_RF) == EXIT_FAIL && tableMatch(Input.VAL_STR, COMMAND_COMMAND_CODE) == EXIT_FAIL)
                {
                    // Read & Update
                    if (Input.VAL_NUM >= BEGINN_FILEINDEX && accessGPIOObject(Input.VAL_NUM) == EXIT_SUCCESS)
                    {
                        Input.VAL_STR[SIZE_MAX_NAME - 1] = zeroTerm; // Just for sure a terminator
                        strncpy(tmp_gpio.name, Input.VAL_STR, SIZE_MAX_NAME);

                        if (updateGPIOObject(Input.VAL_NUM) == EXIT_SUCCESS)
                        {
                            // Check and Tell all about object
                            stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_NAME, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                            stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLGPIO, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                            interpretInput();

                            returnStatus = EXIT_SUCCESS;
                        }
                    }
                }
                else
                {
                    Serial.println(F("interpretInput(): Wanted GPIO-Name already in System!")); // DEBUG
                }
            }
            break;

        case COMMAND_SUB_SETRF:
            if (strlen(Input.VAL_STR) >= 3) // Minimum 3 Chars to ensure it was meant and not in the System
            {
                // Check whether Name is in System
                if (findObjectByName(Input.VAL_STR, DEVTYPE_GPIO) == EXIT_FAIL && findObjectByName(Input.VAL_STR, DEVTYPE_RF) == EXIT_FAIL && tableMatch(Input.VAL_STR, COMMAND_COMMAND_CODE) == EXIT_FAIL)
                {
                    // Read & Update
                    if (Input.VAL_NUM >= BEGINN_FILEINDEX && accessRFObject(Input.VAL_NUM) == EXIT_SUCCESS)
                    {
                        strncpy(tmp_rf.name, Input.VAL_STR, SIZE_MAX_NAME);
                        tmp_rf.name[SIZE_MAX_NAME - 1] = zeroTerm; // Just for sure...

                        if (updateRFObject(Input.VAL_NUM) == EXIT_SUCCESS)
                        {
                            // Check and Tell all about object, regardless of error
                            stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_NAME, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                            stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLRF, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                            interpretInput();

                            returnStatus = EXIT_SUCCESS;
                        }
                    }
                }
                else
                {
                    Serial.println(F("interpretInput(): Wanted RF-Name already in System!")); // DEBUG
                }
            }

        default:
            break;
        }

        break;

    case COMMAND_PIN:
    case COMMAND_DEVTYPE:
        switch (tableMatch(Input.SUBCOM, COMMAND_SUB_CODE))
        {
        case COMMAND_SUB_TELLGPIO:
            if (accessGPIOObject(Input.VAL_NUM) == EXIT_SUCCESS)
            {
                switch (tableMatch(Input.COM, COMMAND_COMMAND_CODE))
                {
                case COMMAND_DEVTYPE:
                    stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_DEVTYPE, Output.COM, SIZE_SINGLE_INPUTBUFFER);
                    stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLGPIO, Output.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                    String(tmp_gpio.devType).toCharArray(Output.VAL_STR, SIZE_SINGLE_INPUTBUFFER);
                    Output.VAL_NUM = Input.VAL_NUM;
                    sendSerialSTRINGNET();

                    break;
                case COMMAND_PIN:
                    stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_PIN, Output.COM, SIZE_SINGLE_INPUTBUFFER);
                    stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLGPIO, Output.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                    String(tmp_gpio.pin).toCharArray(Output.VAL_STR, SIZE_SINGLE_INPUTBUFFER);
                    Output.VAL_NUM = Input.VAL_NUM;
                    sendSerialSTRINGNET();

                    break;
                default:
                    // No need to failcheck as already checked
                    break;
                }

                returnStatus = EXIT_SUCCESS;
            }
            break;

        case COMMAND_SUB_SETGPIO:
            if (strlen(Input.VAL_STR) >= 1) // Minimum 1 Char to ensure it was meant
            {
                if (accessGPIOObject(Input.VAL_NUM) == EXIT_SUCCESS)
                {
                    switch (tableMatch(Input.COM, COMMAND_COMMAND_CODE))
                    {
                    case COMMAND_DEVTYPE:
                        // If it isnt a valid string, maybe it is a valid number?
                        tmp = tableMatch(Input.VAL_STR, COMMAND_DEVTYPE_CODE);
                        if (tmp == EXIT_FAIL)
                        {
                            tmp = (uint8_t)string2Number(Input.VAL_STR);
                            if (tmp > DEVTYPE_COUNT)
                                tmp = EXIT_FAIL;
                        }

                        // If string was valid number or string, update
                        if (tmp != EXIT_FAIL)
                        {
                            tmp_gpio.devType = tmp;

                            if (updateGPIOObject(Input.VAL_NUM) == EXIT_SUCCESS)
                            {
                                // show result
                                stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_DEVTYPE, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                                stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLGPIO, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                                interpretInput();

                                returnStatus = EXIT_SUCCESS;
                                break;
                            }
                        }

                    case COMMAND_PIN:
                        tmp = string2Number(Input.VAL_STR);
                        if (tmp != 0)
                            if (verifyDevtypeToIO(tmp, tmp_gpio.devType, false)) // Evaluation --> Only Update if valid input!
                            {
                                tmp_gpio.pin = tmp;

                                if (updateGPIOObject(Input.VAL_NUM) == EXIT_SUCCESS)
                                {
                                    // show result
                                    stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_PIN, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                                    stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLGPIO, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                                    interpretInput();

                                    returnStatus = EXIT_SUCCESS;
                                }
                            }
                    default:
                        // No need to failcheck as already checked
                        break;
                    }
                }
            }
            break;

        default:
            break;
        }

        break;

    case COMMAND_RFONSEQ:
    case COMMAND_RFOFFSEQ:
    case COMMAND_RFPROTO:
    case COMMAND_RFPULSLEN:
    case COMMAND_RFTXREP:
        switch (tableMatch(Input.SUBCOM, COMMAND_SUB_CODE))
        {
        case COMMAND_SUB_TELLRF:
            if (accessRFObject(Input.VAL_NUM) == EXIT_SUCCESS)
            {
                switch (tableMatch(Input.COM, COMMAND_COMMAND_CODE))
                {
                case COMMAND_RFONSEQ:
                    stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_RFONSEQ, Output.COM, SIZE_SINGLE_INPUTBUFFER);
                    String(tmp_rf.rfonSeq).toCharArray(Output.VAL_STR, SIZE_SINGLE_INPUTBUFFER);
                    break;

                case COMMAND_RFOFFSEQ:
                    stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_RFOFFSEQ, Output.COM, SIZE_SINGLE_INPUTBUFFER);
                    String(tmp_rf.rfoffSeq).toCharArray(Output.VAL_STR, SIZE_SINGLE_INPUTBUFFER);
                    break;

                case COMMAND_RFPROTO:
                    stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_RFPROTO, Output.COM, SIZE_SINGLE_INPUTBUFFER);
                    String(tmp_rf.rfproto).toCharArray(Output.VAL_STR, SIZE_SINGLE_INPUTBUFFER);
                    break;

                case COMMAND_RFPULSLEN:
                    stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_RFPULSLEN, Output.COM, SIZE_SINGLE_INPUTBUFFER);
                    String(tmp_rf.pulseln).toCharArray(Output.VAL_STR, SIZE_SINGLE_INPUTBUFFER);
                    break;

                case COMMAND_RFTXREP:
                    stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_RFTXREP, Output.COM, SIZE_SINGLE_INPUTBUFFER);
                    String(tmp_rf.repeatTX).toCharArray(Output.VAL_STR, SIZE_SINGLE_INPUTBUFFER);
                    break;

                default:
                    // No need to failcheck as already checked
                    break;
                }

                stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLRF, Output.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                Output.VAL_NUM = Input.VAL_NUM;
                sendSerialSTRINGNET();

                returnStatus = EXIT_SUCCESS;
            }
            break;

        case COMMAND_SUB_SETRF:
            if (strlen(Input.VAL_STR) >= 1 && strcasecmp(Input.VAL_STR, "") != 0) // Minimum 1 Char to ensure it was meant
            {
                if (accessRFObject(Input.VAL_NUM) == EXIT_SUCCESS)
                {
                    switch (tableMatch(Input.COM, COMMAND_COMMAND_CODE))
                    {
                    case COMMAND_RFONSEQ:
                        tmp_rf.rfonSeq = string2Number(Input.VAL_STR);
                        stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_RFONSEQ, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                        break;
                    case COMMAND_RFOFFSEQ:
                        tmp_rf.rfoffSeq = string2Number(Input.VAL_STR);
                        stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_RFOFFSEQ, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                        break;
                    case COMMAND_RFPROTO:
                        tmp_rf.rfproto = string2Number(Input.VAL_STR);
                        stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_RFPROTO, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                        break;
                    case COMMAND_RFPULSLEN:
                        tmp_rf.pulseln = string2Number(Input.VAL_STR);
                        stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_RFPULSLEN, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                        break;
                    case COMMAND_RFTXREP:
                        tmp_rf.repeatTX = string2Number(Input.VAL_STR);
                        stringStorageToBuffer(COMMAND_COMMAND_CODE, COMMAND_RFTXREP, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                        break;
                    default:
                        // No need to failcheck as already checked
                        break;
                    }

                    if (updateRFObject(Input.VAL_NUM) == EXIT_SUCCESS)
                    {
                        // show result, regardless of error
                        stringStorageToBuffer(COMMAND_SUB_CODE, COMMAND_SUB_TELLRF, Input.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                        interpretInput();

                        returnStatus = EXIT_SUCCESS;
                    }
                }
            }
        default:
            break;
        }
        break;

    // TODO: Analog

    //#### Object-Interaction ####
    default:
        // Generic Reaction - Try to find object by input
        tmp_gpio.devType = EXIT_FAIL;

        // Name-Scanning -> If correct Name AND NUM + Devtype is given, Name wins
        tmp = findObjectByName(Input.COM, DEVTYPE_GPIO); // stores object index
        if (tmp == EXIT_FAIL)
        {
            tmp = findObjectByName(Input.COM, DEVTYPE_RF);
            if (tmp == EXIT_FAIL)
            {
                // Num/Dev-Typescanning ...
                tmp_gpio.devType = tableMatch(Input.VAL_STR, COMMAND_DEVTYPE_CODE);
                if (tmp_gpio.devType != EXIT_FAIL)
                {
                    //... per GPIO (VAL_NUM = pin, VAL_STR = devtype)
                    tmp = findFirstObjectByDevtype(tmp_gpio.devType);
                    if (accessGPIOObject(tmp) == EXIT_SUCCESS && (uint8_t)Input.VAL_NUM == tmp_gpio.pin)
                    {
                        // Data already in buffer
                    }

                    //... per RF (VAL_NUM = object_index, VAL_STR = devtype)
                    else if (accessRFObject(Input.VAL_NUM) == EXIT_SUCCESS)
                    {
                        tmp_gpio.devType = DEVTYPE_DO_RF_SEND;
                        tmp = Input.VAL_NUM;
                    }
                    else
                        tmp_gpio.devType = EXIT_FAIL;
                }
            }
            else
                tmp_gpio.devType = DEVTYPE_RF;
        }
        else
        {
            tmp_gpio.devType = DEVTYPE_GPIO;
        }

        /*
        Serial.print(F("interpretInput(): Aftersearch: ")); //DEBUG
        Serial.print(tmp_gpio.devType);
        Serial.print(space);
        Serial.println(tmp);
        */

        // if there is a useful reference: Act!
        if (tmp != EXIT_FAIL && tmp_gpio.devType != EXIT_FAIL)
        {
            returnStatus = EXIT_FAILURE; // MUST be EXIT_FAILURE for conditional report

            if (tmp_gpio.devType == DEVTYPE_GPIO)
                checkAndArmSingleDATASET(tmp, DEVTYPE_GPIO);
            else if (tmp_gpio.devType == DEVTYPE_RF)
                checkAndArmSingleDATASET(tmp, DEVTYPE_RF);
            else
            {
                returnStatus = EXIT_FAIL;
                break;
            }

            // Write and Detect Status
            switch (tableMatch(Input.SUBCOM, COMMAND_STATE_CODE))
            {
            case COMMAND_STATE_ON_Silent:
                returnStatus = EXIT_SUCCESS;
            case COMMAND_STATE_ON:
                if (tmp_gpio.devType == DEVTYPE_DO_GENERIC)
                    digitalWrite(tmp_gpio.pin, HIGH);
                else if (tmp_gpio.devType == DEVTYPE_DO_RF_SEND)
                    transmitRF(SEND_ON, tmp);
                break;

            case COMMAND_STATE_OFF_Silent:
                returnStatus = EXIT_SUCCESS;
            case COMMAND_STATE_OFF:
                if (tmp_gpio.devType == DEVTYPE_DO_GENERIC)
                    digitalWrite(tmp_gpio.pin, LOW);
                else if (tmp_gpio.devType == DEVTYPE_DO_RF_SEND)
                    transmitRF(SEND_OFF, tmp);
                break;

            case COMMAND_STATE_TOGGLE_Silent:
                returnStatus = EXIT_SUCCESS;
            case COMMAND_STATE_TOGGLE:
                if (tmp_gpio.devType == DEVTYPE_DO_GENERIC)
                    digitalWrite(tmp_gpio.pin, !digitalRead(tmp_gpio.pin));
                else if (tmp_gpio.devType == DEVTYPE_DO_RF_SEND)
                    transmitRF(SEND_TOGGLE, tmp);
                break;

            case COMMAND_STATE_STATUS:
                // doesnt happen if silent => returnstatus == EXIT_SUCCESS
                break;

            // TODO: Analog
            default:
                // Trigger generic Error-Message
                returnStatus = EXIT_FAIL; //!!
                break;
            }

            // Report back
            if (returnStatus == EXIT_FAILURE) //!!
            {
                returnStatus = EXIT_SUCCESS; //!!

                switch (tmp_gpio.devType)
                {
                case DEVTYPE_DO_RF_SEND:
                    boolToBinaryString(getCachedRemoteBit(tmp)).toCharArray(Output.VAL_STR, SIZE_SINGLE_INPUTBUFFER);
                    strncpy(Output.SUBCOM, tmp_rf.name, SIZE_SINGLE_INPUTBUFFER);
                    break;
                case DEVTYPE_DO_GENERIC:
                case DEVTYPE_DI_GENERIC:
                    boolToBinaryString(digitalRead(tmp_gpio.pin)).toCharArray(Output.VAL_STR, SIZE_SINGLE_INPUTBUFFER);
                    strncpy(Output.SUBCOM, tmp_gpio.name, SIZE_SINGLE_INPUTBUFFER);
                    break;
                case DEVTYPE_DI_PULLUP:
                    boolToBinaryString(!digitalRead(tmp_gpio.pin)).toCharArray(Output.VAL_STR, SIZE_SINGLE_INPUTBUFFER);
                    strncpy(Output.SUBCOM, tmp_gpio.name, SIZE_SINGLE_INPUTBUFFER);
                    break;
                // TODO: Analog
                default:
                    Serial.println(F("interpretInput(): ERROR in Status!"));
                    returnStatus = EXIT_FAIL;
                    break;
                }

                if (returnStatus == EXIT_SUCCESS)
                {
                    stringStorageToBuffer(COMMAND_STATE_CODE, COMMAND_STATE_STATUS, Output.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
                    strncpy(Output.COM, Input.COM, SIZE_SINGLE_INPUTBUFFER);
                    sendSerialSTRINGNET();
                }
            }
        }
        else
        {
            returnStatus = EXIT_FAIL;
        }
        break;
    }

    // Generic Error-Report
    if (returnStatus == EXIT_FAIL)
    {
        stringStorageToBuffer(COMMAND_STATE_CODE, COMMAND_STATE_ERROR, Output.COM, SIZE_SINGLE_INPUTBUFFER);
        String z = Input.COM;
        z += "+";
        z += Input.SUBCOM;
        z.toCharArray(Output.SUBCOM, SIZE_SINGLE_INPUTBUFFER);
        Output.VAL_NUM = Input.VAL_NUM;
        strncpy(Output.VAL_STR, Input.VAL_STR, SIZE_SINGLE_INPUTBUFFER);
        sendSerialSTRINGNET();
    }

    return EXIT_SUCCESS;
}

// Package-Wrapper and sender for serializing Messages. Needs preparation by writing to global StringNetObject Output!
void sendSerialSTRINGNET(void)
{
    Serial.print(seqStart); // STARTSEQ

    if (NetworkMode)
    {
        // Preparation
        DevNameToBuffer(Output.FROM, SIZE_SINGLE_INPUTBUFFER);
        if (strlen(Input.FROM) < 3) // cant be good FROM Address
            strncpy(Output.TO, DEVICE_STANDARDTARGET_NAME, SIZE_SINGLE_INPUTBUFFER);
        else
            strncpy(Output.TO, Input.FROM, SIZE_SINGLE_INPUTBUFFER);

        // Sending Addresses
        Serial.print(Output.FROM);
        Serial.print(stringDelim);
        Serial.print(Output.TO);
        Serial.print(stringDelim);
    }

    // Sending Payload
    Serial.print(Output.COM);
    Serial.print(stringDelim);
    Serial.print(Output.SUBCOM);
    Serial.print(stringDelim);
    Serial.print(Output.VAL_NUM);
    Serial.print(stringDelim);
    Serial.print(Output.VAL_STR);
    Serial.println(seqEnd); // ENDSEQ

    // Resetting Buffer for cleanness
    Output = stringNetObject();
}