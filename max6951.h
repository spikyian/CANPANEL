/*

 Copyright (C) Pete Brownlow 2014-2017   software@upsys.co.uk

  CBUS CANPanel - MAX6951 LED chip hardware driver, definitions

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

// Revision History
//
// 25/01/14     1.0     PNB Coding started


#ifndef __MAX6951_H
#define	__MAX6951_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <devincs.h>
#include "module.h"
#include "vlcb.h"
#include <string.h>

    
// SPI interface definitions

//#include <spi.h>

#define MX_CS_TRIS              (TRISCbits.TRISC6)
#define MX_CS_IO                (LATCbits.LATC6)
#define SCK_TRIS                (TRISCbits.TRISC3)
#define SDO_TRIS                (TRISCbits.TRISC5)
    
#define WaitForDataByte()       {while (SPI1CON2bits.BUSY);}
   

// MAX6951 definitions - see Maxim data sheet
    
// MAX chip register definitions

#define MX_NOP          0
#define MX_DECODE       1       // Controls hex character set decode
#define MX_INTENSITY    2       // Control display intensity - 16 levels
#define MX_SCAN_LIMIT   3       // 5 or 8 digits
#define MX_CONF         4       // Configuration register
#define MX_TEST         7       // Put into test mode
#define MX_DIG_P0       0X20    // Plane 0 - 8 digits this is base register
#define MX_DIG_P1       0x40    // Plane 1 - 8 digits this is base register
#define MX_DIG_BOTH     0X60    // Write both planes - 8 digits this is base register

// Configuration register bit numbers

#define MX_CONF_ENABLE      1   // Shutdown mode - if zero nothing displayed but can still program registers, set to 1 to enable outputs
#define MX_CONF_FASTBLINK   4   // Set to 1 for double blink rate of 0.5sec on 0.5sec off
#define MX_CONF_BLINKON     8   // Enable blinking
#define MX_CONF_BLINKSYNC   16  // To sync multiple 6950/1 chips - not required for CANPanel
#define MX_CONF_CLEAR       32  // Set to 1 to clear all LEDs/digits




void initLedDriver(uint8_t brightness);
void setLedTestMode(Boolean testMode);
void runLedTest( uint8_t testPasses );
Word ledTestCycle( Word testStatus );
void showTestX(void);
void clearAllLeds(void);
void setOn( uint8_t ledNumber);
void setOff( uint8_t ledNumber);
void flashLed( uint8_t ledNumber );
void antiFlashLed( uint8_t ledNumber );
void displayNumber( uint16_t toDisplay, uint8_t offset, uint8_t digits, uint8_t format );
void displayDigit( uint8_t toDisplay, uint8_t offset );
void displayByte( uint8_t toDisplay, uint8_t offset );
void displayChar( unsigned char  toDisplay, uint8_t offset );
void displayString( char *toDisplay, uint8_t offset);
void displayMessage( char *message, uint8_t offset, uint8_t digits, Boolean scroll );
void scrollDisplay( Boolean direction, uint8_t limit );
void sayHello( void );
void displayVersion( void );

void doSwDelay( uint16_t milliseconds );


// In memory map of LEDs/display status

typedef uint8_t     DigitMap[8];    // One bit per segment/LED (on or off)
typedef DigitMap    LedsMap[2];     // Two planes - for flashing status

typedef struct
{
    uint8_t    dig;
    uint8_t    seg;
} Segment;

#ifdef	__cplusplus
}
#endif

#endif	/* MAX6951_H */

