/*
 Routines for CBUS FLiM operations - part of CBUS libraries for PIC 18F
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
**************************************************************************************************************
	The FLiM routines have no code or definitions that are specific to any
	module, so they can be used to provide FLiM facilities for any module 
	using these libraries.
	
*/ 
/*
 * File:   panelNv.c
 * Author: Ian Hogg
 *
 * Created on 26 June 2024, 18:14
 */

/**
 * Node variables contain global parameters that need to be persisted to Flash.
 */

#include "module.h"
#include "panelNv.h"
#include "nvm.h"
#include "nv.h"

/**
 * The Application specific NV defaults are defined here.
 */
uint8_t APP_nvDefault(uint8_t index) {
    switch(index) {
        case NV_SOD_DELAY:
            return 0;
        case NV_HB_DELAY:
            return 0;
        case NV_PANEL_FLAGS:
            return 0;
        case NV_RESPONSE_DELAY:
            return 2;
        case NV_SEG_OUTPUTS:
            return 0;
        case NV_BRIGHTNESS:
            return 0;
        default:    // PB_FLAGS
            return 0;
    }
} 

/**
 * We perform the necessary action when an NV changes value.
 */
void APP_nvValueChanged(uint8_t index, uint8_t value, uint8_t oldValue) {

}

/**
 * We validate NV values here.
 *
 */
NvValidation APP_nvValidate(uint8_t index, uint8_t value)  {
    return VALID;
}
