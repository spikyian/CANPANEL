/*

 Copyright (C) Pete Brownlow 2014-2017   software@upsys.co.uk

  CBUS CANPanel - Button/Switch matrix scanning

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

#include "buttonscan.h"
#include "panelEvents.h"
#include "nv.h"
#include "event_producer.h"

typedef union {
    uint8_t val;   
    struct {
        uint8_t b0:1;
        uint8_t b1:1;
        uint8_t b2:1;
        uint8_t b3:1;
        uint8_t b4:1;
        uint8_t b5:1;
        uint8_t b6:1;
        uint8_t b7:1;
    } bits;
} ByteBits;

typedef union {
    uint8_t val;
    struct {
        uint8_t lower:4;
        uint8_t upper:4;
    } nibbles;
} ByteNibbles;

ByteBits keyInputState[COLUMN_OUTPUTS];  // after debounce
ByteBits keyOutputState[COLUMN_OUTPUTS]; // after toggle
ByteNibbles debounceCounters[COLUMN_OUTPUTS*ROW_INPUTS/2];  // 4 bits per counter
#define PB(c,r) ((c)*ROW_INPUTS + (r))
#define COUNTER_INDEX(c,r)   (PB(c,r)/2)

#define DEBOUNCE_DELAY  4   // 40ms

// forward declarations
void sendPBEvent(uint8_t pb, uint8_t state, uint8_t flags);

/**
 * Initialise the button scanning.
 */
void initKeyscan(void) {
    uint8_t col;
    uint8_t        strobeMask;
    
    // Keypad strobe  output pins - intialise all high
    COL_LAT |= COLUMN_MASK;  
                    
    // Keypad strobe  input pins
    KBD_INP0_TRIS = 1;
    KBD_INP1_TRIS = 1;
    KBD_INP2_TRIS = 1;
    KBD_INP3_TRIS = 1;
#if ROW_INPUTS >3
    KBD_INP4_TRIS = 1;
    KBD_INP5_TRIS = 1;
    KBD_INP6_TRIS = 1;
    KBD_INP7_TRIS = 1;
#endif
    
    // now initialise the matrix data with current matrix state
    for ( col = 0; col < COLUMN_OUTPUTS; col++) {
        strobeMask = ~((0b00000001 << col) & COLUMN_MASK);    // Shifting column from bit 0
        COL_LAT |= COLUMN_MASK;                                 // Set all strobe column bits
        COL_LAT &= strobeMask;                                  // Clear strobe bit to active low for this column

        // Read in the value strobed by the outputs
        keyInputState[col].val = 0;
        keyInputState[col].bits.b0 = KBD_INP0;
        keyInputState[col].bits.b1 = KBD_INP1;
        keyInputState[col].bits.b2 = KBD_INP2;
        keyInputState[col].bits.b3 = KBD_INP3;
        keyInputState[col].bits.b4 = KBD_INP4;
        keyInputState[col].bits.b5 = KBD_INP5;
        keyInputState[col].bits.b6 = KBD_INP6;
        keyInputState[col].bits.b7 = KBD_INP7;
        
        keyOutputState[col].val = keyInputState[col].val;
        
    }  // for each column strobe group
    
    // zero the debounce counters
    for (col = 0; col < COLUMN_OUTPUTS*ROW_INPUTS/2; col++) {
        debounceCounters[col].val = 0;
    }
}

/**
 * This routine does one scan of the keypad, updating any debounce counts. It returns the value
 * of the currently pressed key, when the press has been debounced.
 * IF no valid key (or combination) is  pressed, or there has been no change, then 0xFF is returned.
 *
 * NOTE: this code is optimised by using specific binary patterns for the bit masks.
 * These are defined in matrix.h which is application hardware specific.
 * You need to generate a matrix.h file specific to your hardware
 * If there are particular hardware differences, conditional code changes may also be required
 *
 * The keypad control pins are shared with the LCD output and the SPI clock.
 * The LCD routines must leave LCD CS disabled
 * The SPI routines must leave all SPI device CS signals disabled and the SPI module disabled to enable normal I/O operation
 * Interrupts are disabled during the strobe out and read in sequence in case LCD or SPI operations happen in the ISR
 *
 * This routine does NOT block during the debounce, and should be called repeatedly from the main loop,
 * It will return the new keycode when the debounce period has completed.
 */
void keyScan( void ) {
    uint8_t col;
    uint8_t strobeMask;
    uint8_t flags;
    
    for ( col = 0; col < COLUMN_OUTPUTS; col++) {
        strobeMask = ~((0b00000001 << col) & COLUMN_MASK);    // Shifting column from bit 0
        COL_LAT |= COLUMN_MASK;                                 // Set all strobe column bits
        COL_LAT &= strobeMask;                                  // Clear strobe bit to active low for this column

        // Check each button read in the value strobed by the outputs
        if (keyInputState[col].bits.b0 != KBD_INP0) {
            if (debounceCounters[COUNTER_INDEX(col, 0)].nibbles.lower == DEBOUNCE_DELAY) {
                debounceCounters[COUNTER_INDEX(col, 0)].nibbles.lower = 0;
                keyInputState[col].bits.b0 = KBD_INP0;
                
                flags = (uint8_t)getNV(NV_PB_FLAGS+PB(col,0));
                if (flags & NV_PB_FLAGS_TOGGLE) {
                    keyOutputState[col].bits.b0 = ! keyOutputState[col].bits.b0;
                } else {
                    if (flags & NV_PB_FLAGS_POLARITY) {
                        keyOutputState[col].bits.b0 = KBD_INP0;
                    } else {
                        keyOutputState[col].bits.b0 = ! KBD_INP0;
                    }
                }
                sendPBEvent(PB(col,0), keyOutputState[col].bits.b0, flags);
            } else {
                debounceCounters[COUNTER_INDEX(col, 0)].nibbles.lower++;
            }
        }
        if (keyInputState[col].bits.b1 != KBD_INP1) {
            
        }
        if (keyInputState[col].bits.b2 != KBD_INP2) {
            
        }
        if (keyInputState[col].bits.b3 != KBD_INP3) {
            
        }
        if (keyInputState[col].bits.b4 != KBD_INP4) {
            
        }
        if (keyInputState[col].bits.b5 != KBD_INP5) {
            
        }
        if (keyInputState[col].bits.b6 != KBD_INP6) {
            
        }
        if (keyInputState[col].bits.b7 != KBD_INP7) {
            
        }
    }  // for each column strobe group
}   // keyscan

void sendPBEvent(uint8_t pb, uint8_t state, uint8_t flags) {
    if (state) {
        if (flags & NV_PB_FLAGS_SEND_ON) {
            sendProducedEvent((Happening)(PB_2_HAPPENING(pb)), EVENT_ON);
        }
    } else {
        if (flags & NV_PB_FLAGS_SEND_OFF) {
            sendProducedEvent((Happening) (PB_2_HAPPENING(pb)), EVENT_OFF);
        }
    }
}

EventState getKeyState(uint8_t pb) {
    switch (pb%8) {
        case 0:
            return keyOutputState[pb/8].bits.b0 ? EVENT_ON : EVENT_OFF;
        case 1:
            return keyOutputState[pb/8].bits.b1 ? EVENT_ON : EVENT_OFF;
        case 2:
            return keyOutputState[pb/8].bits.b2 ? EVENT_ON : EVENT_OFF;
        case 3:
            return keyOutputState[pb/8].bits.b3 ? EVENT_ON : EVENT_OFF;
        case 4:
            return keyOutputState[pb/8].bits.b4 ? EVENT_ON : EVENT_OFF;
        case 5:
            return keyOutputState[pb/8].bits.b5 ? EVENT_ON : EVENT_OFF;
        case 6:
            return keyOutputState[pb/8].bits.b6 ? EVENT_ON : EVENT_OFF;
        case 7:
            return keyOutputState[pb/8].bits.b7 ? EVENT_ON : EVENT_OFF;
    }
    return EVENT_UNKNOWN;
}

#ifdef RETURN_LOOKUP
/**
 *  Return lookup keycode value as a uint8_t from the strobe results array
 */
uint8_t keyLookup( uint8_t buttonNum, MatrixState buttonState ) {
    uint8_t keyCode;
    uint8_t i;

    if ((buttonState & 0x8000000) == 0) { // Single or no key pressed
        // Look up single key press
        return( singleKeyLookupTable[buttonNum] );
    } else { // key combination
        for (i = 0; (i< KEY_COMBINATIONS) && (buttonState != keyCombinationLookupTable[i].buttonState) ; i++)
            ;
        return(keyCombinationLookupTable[i].keycode);
    }
}
#endif










