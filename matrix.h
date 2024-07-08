/*

 Copyright (C) Pete Brownlow 2014-2017   software@upsys.co.uk

  CBUS CANPanel - Defintion of the keyboard matrix hardware for the CANPanel module

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
	
 For version number and revision history see CANPanel.h


 Created on 17 January 2015, 21:11
*/

#ifndef MATRIX_H
#define	MATRIX_H

#ifdef	__cplusplus
extern "C" {
#endif


#define COLUMN_OUTPUTS      8
#define ROW_INPUTS          8


// Column output definitions

#define COL_LAT             LATC                    // Port for column strobe outputs
#define COLUMN_MASK         0b11111111              // Mask for bits used for column strobe outputs

#define KBD_STROBE0         LATCbits.LATC0
#define KBD_STROBE0_TRIS    TRISCbits.TRISC0
#define KBD_STROBE1         LATCbits.LATC1
#define KBD_STROBE1_TRIS    TRISCbits.TRISC1
#define KBD_STROBE2         LATCbits.LATC2
#define KBD_STROBE2_TRIS    TRISCbits.TRISC2
#define KBD_STROBE3         LATCbits.LATC3          // Note shares with SPI SCK pin
#define KBD_STROBE3_TRIS    TRISCbits.TRISC3
#define KBD_STROBE4         LATCbits.LATC4
#define KBD_STROBE4_TRIS    TRISCbits.TRISC4
#define KBD_STROBE5         LATCbits.LATC5          // Note shares with SPI SDO pin
#define KBD_STROBE5_TRIS    TRISCbits.TRISC5
#define KBD_STROBE6         LATCbits.LATC6          // Note shares with MAX CHIP CS
#define KBD_STROBE6_TRIS    TRISCbits.TRISC6
#define KBD_STROBE7         LATCbits.LATC7
#define KBD_STROBE7_TRIS    TRISCbits.TRISC7


// Row input definitions

#define ROW_MASK            0b11111111

#define KBD_INP0            PORTBbits.RB0
#define KBD_INP0_TRIS       TRISBbits.TRISB0
#define KBD_INP1            PORTBbits.RB1
#define KBD_INP1_TRIS       TRISBbits.TRISB1
#define KBD_INP2            PORTBbits.RB4
#define KBD_INP2_TRIS       TRISBbits.TRISB4
#define KBD_INP3            PORTBbits.RB5
#define KBD_INP3_TRIS       TRISBbits.TRISB5
#define KBD_INP4            PORTAbits.RA5
#define KBD_INP4_TRIS       TRISAbits.TRISA5
#define KBD_INP5            PORTAbits.RA3
#define KBD_INP5_TRIS       TRISAbits.TRISA3
#define KBD_INP6            PORTAbits.RA1
#define KBD_INP6_TRIS       TRISAbits.TRISA1
#define KBD_INP7            PORTAbits.RA0
#define KBD_INP7_TRIS       TRISAbits.TRISA0


#ifdef	__cplusplus
}
#endif

#endif	/* MATRIX_H */

