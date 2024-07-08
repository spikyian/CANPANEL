/*

 Copyright (C) Pete Brownlow 2014-2017   software@upsys.co.uk

  CBUS CANPanel - MAX6951 LED chip hardware driver

 This code is for a CANPanel CBUS module, to control up to 64 LEDs (or 8 x 7 segment displays)
 and up to 64 push buttons or on/off switches

 This work is licensed under the:
      Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
   To view a copy of this license, visit:
      http://creativecommons.org/licenses/by-nc-sa/4.0/
   or send a letter to Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.

   License summary:
    You are free to:
      Share, copy and redistribute the material in any medium or format
      Adapt, remix, transform, and build upon the material

    The licensor cannot revoke these freedoms as long as you follow the license terms.

    Attribution : You must give appropriate credit, provide a link to the license,
                   and indicate if changes were made. You may do so in any reasonable manner,
                   but not in any way that suggests the licensor endorses you or your use.

    NonCommercial : You may not use the material for commercial purposes. **(see note below)

    ShareAlike : If you remix, transform, or build upon the material, you must distribute
                  your contributions under the same license as the original.

    No additional restrictions : You may not apply legal terms or technological measures that
                                  legally restrict others from doing anything the license permits.

   ** For commercial use, please contact the original copyright holder(s) to agree licensing terms

    This software is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE

**************************************************************************************************************
  Note:   This source code has been written using a tab stop and indentation setting
          of 4 characters. To see everything lined up correctly, please set your
          IDE or text editor to the same settings.
******************************************************************************************************
	
 For full CANPanel version number and revision history see CANPanel.h

*/

/* Revision History
 *
 * 25/01/14     1.0     PNB Coding started
 * 07/07/2024 Ian Hogg updated for PIC18F27Q83 and port to VLCB 
 */
#include <xc.h>
#include "max6951.h"
#include "vlcb.h"
#include "module.h"


// Character generator for ascii characters onto 7 seg display (quite a few compromises!)
// Starts from "0" 0x30
//                              0    1    2    3    4    5    6    7    8    9
const uint8_t  charGen[] = { 0x7E,0x30,0x6d,0x79,0x33,0x5b,0x5F,0x70,0x7F,0x7B,0,0,0,0,0,0,
//                 Upper case   A    B    C    D    E    F    G    H    I    J    K    L    M    N    O
                            0,0x77,0x1F,0x4E,0x3D,0x4F,0x47,0x5E,0x17,0x06,0x3C,0x07,0x0E,0x76,0x15,0x7E,
//                              P    Q    R    S    T    U    V    W    X    Y    Z
                              0x67,0x73,0x05,0x5B,0x0F,0x3E,0x1C,0x3F,0x31,0x3B,0x6D,0,0,0,0,0,
//                 Lower case   A    B    C    D    E    F    G    H    I    J    K    L    M    N    O
                            0,0x77,0x1F,0x4E,0x3D,0x4F,0x47,0x5E,0x17,0x06,0x3C,0x07,0x0E,0x76,0x15,0x7E,
//                              P    Q    R    S    T    U    V    W    X    Y    Z
                              0x67,0x73,0x05,0x5B,0x0F,0x3E,0x1C,0x3F,0x31,0x3B,0x6D,0,0,0,0,0  };

const char HELLO[] = "HELLO";


// In memory status of LEDs
// Copy of MAX chip registers, we need to keep copies in RAM because MAX chip is write only
LedsMap ledsMap;
uint8_t decodeMode;

// Local function prototypes
void sendMxCmd( uint8_t mxRegister, uint8_t mxValue );


/**
 * Set up the SPI(1) to communicate with the MAX6951 LED chip.
 * At this point all the I/O have been set to be digital.
 * @param brightness
 */
void initLedDriver(uint8_t brightness) {
    uint8_t    regadr, regval;

    // RC3 is used as SCK output
    // RC5 used for SDO output
    // RC6 used for MAX6951 chip select
    SCK_TRIS = 0;       // SPI clock is an output
    SDO_TRIS = 0;       // SPI data output
    MX_CS_TRIS = 0;     // MAX chip select is an output
    MX_CS_IO = 1;       // Deselect to start with
    // set up PPS
    RC3PPS = 0x31;      //SPI1_SCK
    RC5PPS = 0x32;      //SPI1_SDO
    RC6PPS = 0x33;      //SPI1_SS

    // SPI1EN=0;    // Disabled
    // SPI1MST=1;   // Master
    // SPI1LSF=0;   // MSB first
    // SPI1BMODE=1  // Count is total
    SPI1CON0 = 0x03 ; 
    // SPI1SMP=0;   // Sample in middle
    // SPI1CKE=1;   // Data change on Active to Idle
    // SPI1CKP=0;   // Active is low
    // SPI1FST=1;   // Delay may be less than 0.5 baud period
    // SPI1SSP=1;   // SS is active low
    // SPI1SDIP=0;  // SPI input is active high (not used)
    // SPI1SDOP=0;  // SDO output is active high
    SPI1CON1 = 0x54;    
    // SPI1SSET = 0;// Drive the CS line active whilst transmitting
    // SPI1TXR=1;   // Require a TX buffer
    // SPI1RXR=0;   // Don't require a RX buffer (N/A))
    SPI1CON2 = 0x02;
    
    OSCFRQbits.HFFRQ = 6; // HFINTOSC at 32MHz
    SPI1CLK = 0;    // SPI set to HFINTOSC
    SPI1BAUD = 0;   // BAUD set to HFINTOSC/2 i.e. 16MHz (ok for MAX chip) with idle clock low. 
    // SSP not enabled yet.
    
    regadr = MX_CONF;
    regval = MX_CONF_FASTBLINK + MX_CONF_BLINKON;

    sendMxCmd( MX_TEST, 0);                                     // Make sure test mode is off
    sendMxCmd( MX_CONF, MX_CONF_CLEAR );                        // Initialise with outputs shut down and all outputs off
    sendMxCmd( MX_SCAN_LIMIT, 0XFF );                           // Show all LEDs/digits
    sendMxCmd( MX_INTENSITY, brightness &0x0F);                 // Brightness passed in as parameter (0-15))
    clearAllLeds();
    decodeMode = 0;
    sendMxCmd( MX_CONF, MX_CONF_FASTBLINK + MX_CONF_BLINKON + MX_CONF_ENABLE );  // Enable outputs with blink feature enabled
  //sendMxCmd( MX_TEST, 1);     // put into test mode to prove initalisation worked
}


// Pass true or false to set test mode on or off
void setLedTestMode(Boolean testMode) {
    // In the Maxim chip, test mode is all segments on at 50% duty cycle (half brightness)
    sendMxCmd( MX_TEST, (uint8_t) testMode);
}


// Run full LED test, cycling each LED on in turn, for the number of passes specified, with software delay

void runLedTest( uint8_t testPasses ) {
    uint8_t passCount, digCount, segCount;
    
    for (passCount = 0; passCount < testPasses; passCount++) {
        for (digCount = 0; digCount < 8; digCount++) {
            segCount = 0x80;
  
            while (segCount != 0) {
                sendMxCmd( MX_DIG_BOTH + digCount, segCount);
                doSwDelay( 500 );   // ?? Convert to heartbeat delay once ISR done
                segCount = (uint8_t)(segCount == 0x80 ? 1 : segCount<<1);
                if (segCount == 0x80)
                    segCount = 0;
            }    
            sendMxCmd( MX_DIG_BOTH + digCount, 0 );
        }    
    }    
}

// Increment LED test to next LED - returns after each LED for other main loop processing
Word ledTestCycle(Word testStatus ) {
    uint8_t    digCount, segCount;
    
    // Digit count is in status MSuint8_t and segment count is in status LSuint8_t
    
    if (testStatus.word == 0xFFFF) {  // Start of test
        digCount = 0;
        segCount = 0;
    } else {   
        digCount = testStatus.bytes.hi;
        segCount = testStatus.bytes.lo;
        if (segCount == 0) {      // start of digit
//            sendMxCmd( MX_DIG_BOTH + digCount++, 0 );   // Clear last digit and move to next
            if (++digCount > 7) {
                digCount = 0;
            }
        }
    }
    
    segCount = (uint8_t)(segCount == 0 ? 1 : segCount<<1);    // Next segment bit in digit byte
   
    sendMxCmd( MX_DIG_BOTH + digCount, segCount);   // Turn on one segment
    
    testStatus.bytes.hi = digCount;
    testStatus.bytes.lo = segCount;
    
    return( testStatus );
}


void showTestX(void) {
    sendMxCmd( MX_DIG_BOTH, 0xC0);
    sendMxCmd( MX_DIG_BOTH + 1, 0x21);
    sendMxCmd( MX_DIG_BOTH + 2, 0x12);
    sendMxCmd( MX_DIG_BOTH + 3, 0x0C);
    sendMxCmd( MX_DIG_BOTH + 4, 0x0c);
    sendMxCmd( MX_DIG_BOTH + 5, 0x12);
    sendMxCmd( MX_DIG_BOTH + 6, 0x21);
    sendMxCmd( MX_DIG_BOTH + 7, 0xC0);
}

/**
 * All Leds OFF.
 */
void clearAllLeds(void) {
    uint8_t    digCount;
    uint8_t    regadr;

    sendMxCmd( MX_DECODE, 0 );  //. turn off character decoding

    for (digCount = 0; digCount < 8; digCount++) {
        regadr = MX_DIG_BOTH + digCount;
        sendMxCmd( regadr, 0);
     }
     memset( (void *) ledsMap, 0, sizeof(ledsMap) );                     // Set in memory map to all zeroes
}

/**
 * Turn an Led ON.
 * @param ledNo
 */
void setOn(uint8_t ledNumber) {
    uint8_t    digNum, segNum;

    digNum = --ledNumber/8;
    segNum = ledNumber % 8;
    
    // update in memory status arrays, then send to chip
    ledsMap[0][digNum] |= segNum;
    ledsMap[1][digNum] |= segNum;
    sendMxCmd( MX_DIG_P0 + digNum, ledsMap[0][digNum]);
    sendMxCmd( MX_DIG_P1 + digNum, ledsMap[1][digNum]);
}

/**
 * Turn an Led OFF.
 * @param ledNo
 */
void setOff(uint8_t ledNumber) {
    uint8_t    digNum, segNum;

    digNum = --ledNumber/8;
    segNum = ledNumber % 8;
    
    // update in memory status arrays, then send to chip
    ledsMap[0][digNum] &= ~segNum;
    ledsMap[1][digNum] &= ~segNum;
    sendMxCmd( MX_DIG_P0 + digNum, ledsMap[0][digNum]);
    sendMxCmd( MX_DIG_P1 + digNum, ledsMap[1][digNum]);
}


/**
 * Set LED to flashing state (use setLed to clear flashing state)
 * @param ledNumber
 */
void flashLed( uint8_t ledNumber ) {
    uint8_t    digNum, segNum;

    digNum = --ledNumber/8;
    segNum = ledNumber % 8;
    
    // update in memory status arrays, then send to chip
    ledsMap[0][digNum] |= segNum;
    ledsMap[1][digNum] &= ~segNum;
    sendMxCmd( MX_DIG_P0 + digNum, ledsMap[0][digNum]);
    sendMxCmd( MX_DIG_P1 + digNum, ledsMap[1][digNum]);
}

/**
 * Set Led to anti-phase flashing state.
 * @param ledNumber
 */
void antiFlashLed( uint8_t ledNumber ) {
    uint8_t    digNum, segNum;

    digNum = --ledNumber/8;
    segNum = ledNumber % 8;
    
    // update in memory status arrays, then send to chip
    ledsMap[0][digNum] &= ~segNum;
    ledsMap[1][digNum] |= segNum;
    sendMxCmd( MX_DIG_P0 + digNum, ledsMap[0][digNum]);
    sendMxCmd( MX_DIG_P1 + digNum, ledsMap[1][digNum]);
}


// Display a 16 bit number on 7 segment displays
void displayNumber( uint16_t toDisplay, uint8_t offset, uint8_t digits, uint8_t format ) {
    uint8_t    byteToDisplay;   //  display value in LS byte of this word

    byteToDisplay = toDisplay >> 8;        // Get the first byte to display, before the value gets truncated to a byte in the parameter
    displayByte(byteToDisplay,offset);
    byteToDisplay = toDisplay & 0xFF;
    displayByte(byteToDisplay, offset+2);
}


// Display LS nibble of toDisplay as a hex digit on the 7 segment display digit given by offset
void displayDigit( uint8_t toDisplay, uint8_t offset ) {
    toDisplay &= 0x0F;
    decodeMode |= (1<<offset);

    // ?? Put in validation check for offset value
    sendMxCmd( MX_DECODE, decodeMode);
    sendMxCmd( MX_DIG_BOTH + offset, toDisplay);
}

// Display byte as 2 hex digits on the 7 segment display starting at the digit given by offset
void displayByte( uint8_t toDisplay, uint8_t offset ) {
    displayDigit( toDisplay>>4, offset );
    displayDigit( toDisplay, offset + 1);
}


void displayChar( unsigned char  toDisplay, uint8_t offset ) {
    unsigned char genChar;

    decodeMode &= ~(1<<offset);
    sendMxCmd( MX_DECODE, decodeMode);   // Turn off decode for alphanumerics

    genChar = (toDisplay == ' ' ? 0 : charGen[toDisplay - 0x30]);
    sendMxCmd( MX_DIG_BOTH + offset, genChar);
}

void displayString( char *toDisplay, uint8_t offset) {
    uint8_t    i;

    for (i = 0; i<strlen(toDisplay); i++) {
        displayChar( toDisplay[i], offset+i);
    }
}


// For first pass test 7 seg display board, fix segment value due to error in board
// Map an LED from row and column to digit number and segment bit map
// Row is 0-8
// Column is 0-7 (for A-H)
Segment mapLED( uint8_t row, uint8_t column ) {
    Segment segment;

    segment.dig = row;
    if (column > row) {
        column--;
    }

    segment.seg = (uint8_t)(column == 0 ? 0x80 : 1<<(column-1) );

    return( segment );
}


void displayMessage( char *message, uint8_t offset, uint8_t digits, Boolean scroll) {
}

void scrollDisplay( Boolean direction, uint8_t limit ) {
}

void sayHello( void ) {
    char    message[9] = {'H','E','L','L','O',' ',' ',' ',0};

    displayString( (char *) &message, 0);
}

void displayVersion( void ) {
    char    message[9] = {'V',' ',' ',' ',' ',' ',' ',' ',0};
    
    message [2] = PARAM_MAJOR_VERSION + 0x30;
    message [3] = PARAM_MINOR_VERSION;
    message[4] = 'B';
    message[5] = 'L';
    message[6] = 'D';
    message[7] = PARAM_BUILD_VERSION + 0x30;   
    displayString( (char *) &message, 0);
}

// ?? This routine will be replaced once the ISR with heartbeat support is done
void doSwDelay( uint16_t milliseconds ) {
    uint16_t  msCounter;
    uint16_t  loopCounter;
    uint16_t  loopsPerMs;

    loopsPerMs = clkMHz/4;
    loopsPerMs  *= 50L;

    for (msCounter = 0; msCounter < milliseconds; msCounter++) {
    // Divide instruction clock by 1000 to get instruction cycles per mS, then divide again for number of instructions per loop
        for (loopCounter = 0; loopCounter < loopsPerMs; loopCounter++)
            ;
    }
}

void sendMxCmd( uint8_t mxRegister, uint8_t mxValue) {
    Boolean    intState;

    intState = INTCON0bits.GIEL;
    INTCON0bits.GIEL = 0;             // Disable low priority interrupts whilst using SPI, as common I/O pins may be used by ISR

    SPI1CON0bits.EN = 1;            // Enable SPI
    MX_CS_IO = 0;                   // Enable MAX chip
    SPI1TXB = mxRegister;            // Send register address
    WaitForDataByte();              // Wait for transfer to complete
    SPI1TXB = mxValue;               // Send data value
    WaitForDataByte();
    MX_CS_IO = 1;                   // Transfers sent data into register
    MX_CS_IO = 0;                   // Next command
    SPI1TXB = MX_NOP;                // Finish with a nop so subsequent transitions on CS cause no problem
    WaitForDataByte();
    SPI1TXB = 0;
    WaitForDataByte();
    MX_CS_IO = 1;                   // Latch NOP command into MAX chip

    SPI1CON0bits.EN = 0;          // Disable SPI so pins can be used for other things

    INTCON0bits.GIEL = intState;
}