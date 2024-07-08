#ifndef _PANELEVENTS_H_
#define _PANELEVENTS_H_

#include "module.h"

#define PB_2_HAPPENING(pb) (pb+1)
#define HAPPENING_2_PB(h)  (h-1)

#define HAPPENING_SOD   (NUM_PB + 1)

//
// The Action is a 2 byte value.
// Byte 1 is the LED number 1..64 (65=Specials ))
// Byte 2 is the flags
#define ACTION_FLAGS_ENABLEON       0x01
#define ACTION_FLAGS_ENABLEOFF      0x02
#define ACTION_FLAGS_INVERT_EVENT   0x04
#define ACTION_FLAGS_FLASH          0x08
#define ACTION_FLAGS_INVERT_FLASH   0x10

#define ACTION_SPECIALS         (NUM_LED + 1)
// Special Actions go into the flags byte
#define ACTION_SPECIAL_SOD      1

#define NUM_ACTIONS             (NUM_LED + 1)

void factoryResetGlobalEvents(void);
void panelEventsInit(void);

#endif