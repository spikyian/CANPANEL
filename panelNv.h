#ifndef _PANELNV_H_
#define _PANELNV_H_

/*
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
 */

// Global NVs
#define NV_VERSION                      0
#define NV_SOD_DELAY                    1
#define NV_HB_DELAY                     2
#define NV_PANEL_FLAGS                  3   // hello, flash at start, sync toggles,
#define NV_SEG_OUTPUTS                  4 // Boolean[8] block is 7seg
#define NV_BRIGHTNESS                   5
#define NV_RESPONSE_DELAY               6
#define NV_TEST_MODE                    7
// PB flags (NUM_PB) NVs
#define NV_PB_FLAGS                     8 //send on event, send off event, polarity, toggle, include SoD, uninitialised
// free at NV_PB_FLAGS + NUM_PB

#define NV_PB_FLAGS_SEND_ON             0x01
#define NV_PB_FLAGS_SEND_OFF            0x02
#define NV_PB_FLAGS_POLARITY            0x04
#define NV_PB_FLAGS_TOGGLE              0x08
#define NV_PB_FLAGS_ENABLE_SOD         0x10


#endif