/*

 Copyright (C) Pete Brownlow 2014-2017   software@upsys.co.uk
 Updated by Ian Hogg

  CBUS CANPanel - Definitions for Button/Switch matrix scanning

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

*/

#ifndef __BUTTONSCAN_H
#define __BUTTONSCAN_H

#include "vlcb.h"
#include "ticktime.h"
#include "matrix.h"

#define KEY_DEBOUNCE_TIME   TWENTY_MILI_SECOND

#define CR  0x13

typedef union
{
    uint8_t    stateArray[COLUMN_OUTPUTS];
    uint16_t   stateVal[2];
} MatrixState;

#define matrixChanged(a,b)  ((a.stateVal[0] != b.stateVal[0]) || (a.stateVal[1] != b.stateVal[1]))
#define matrixEquals(a,b)   ((a.stateVal[0] == b.stateVal[0]) && (a.stateVal[1] == b.stateVal[1]))

// Function prototypes
void initKeyscan(void);
void keyScan( void );
EventState getKeyState(uint8_t pb);

#endif

