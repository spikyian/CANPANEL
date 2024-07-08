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
 * File:   panelEvents.c
 * Author: Ian Hogg
 * 
 * Here we deal with the module specific event handling. This covers:
 * <UL>
 * <LI>Setting of default events.</LI>
 * <LI>Processing of inbound, consumed events</LI>
 * <LI>Processing Start of Day (SOD) action</LI>
 *</UL>
 * 
 * Created on 28 June 2024, 10:26
 *
 */

#include <stddef.h>

#include "vlcb.h"
#include "nvm.h"
#include "module.h"
#include "event_producer.h"
#include "event_consumer_simple.h"
#include "event_teach_large.h"
#include "mns.h"
#include "timedResponse.h"
#include "panelEvents.h"
#include "max6951.h"
#include "nv.h"


// forward declarations
void doSOD(void);
TimedResponseResult sodTRCallback(uint8_t type, uint8_t serviceIndex, uint8_t step);

void panelEventsInit(void) {
}

/**
 * Set Global Events back to factory defaults.
 */
void factoryResetGlobalEvents(void) {
    uint8_t i;
    // we don't create a default SOD event
    
    // Create the push button produced events
    for (i=1; i<=NUM_PB; i++) {
        addEvent(nn.word, i, 0, i, TRUE);
    }
}


/**
 * Panel specific version of add an event/EV.
 * This ensures that if a Happening is being written then this happening does not
 * exist for any other event.
 * 
 * @param nodeNumber
 * @param eventNumber
 * @param evNum the EV index (starts at 0 for the produced action)
 * @param evVal the EV value
 * @param forceOwnNN the value of the flag
 * @return error number or 0 for success
 */
uint8_t APP_addEvent(uint16_t nodeNumber, uint16_t eventNumber, uint8_t evNum, uint8_t evVal, Boolean forceOwnNN) {
    if ((evNum == 0) && (evVal != NO_ACTION))
    {
        // this is a Happening
#ifdef EVENT_HASH_TABLE       // This generates compile errors if hash table not defined, because producer events are not supported if hash table turned off 
        uint8_t tableIndex = happening2Event[evVal];
        if (tableIndex != NO_INDEX) {
            // Happening already exists
            // remove it
            writeEv(tableIndex, 0, NO_ACTION);
            checkRemoveTableEntry(tableIndex);

            rebuildHashtable();         
        }
#endif  
    }
    return addEvent(nodeNumber, eventNumber, evNum, evVal, forceOwnNN);
}



Processed APP_processConsumedEvent(uint8_t tableIndex, Message * m) {
    uint8_t ledNo;
    uint8_t flags;
    uint8_t e;
    uint8_t pol;
    
    if (m->len < 5) return NOT_PROCESSED;

    switch (m->opc) {
        case OPC_ACON:
#ifdef HANDLE_DATA_EVENTS
        case OPC_ACON1:
        case OPC_ACON2:
        case OPC_ACON3:
#endif
        case OPC_ASON:
#ifdef HANDLE_DATA_EVENTS
        case OPC_ASON1:
        case OPC_ASON2:
        case OPC_ASON3:
#endif
        case OPC_ACOF:
#ifdef HANDLE_DATA_EVENTS
        case OPC_ACOF1:
        case OPC_ACOF2:
        case OPC_ACOF3:
#endif
        case OPC_ASOF:
#ifdef HANDLE_DATA_EVENTS
        case OPC_ASOF1:
        case OPC_ASOF2:
        case OPC_ASOF3:
#endif
            break;
        default:
            return NOT_PROCESSED;
    }

    e = getEVs(tableIndex);
#ifdef SAFETY
    if (e != 0) {
        return PROCESSED; // error getting EVs. Can't report the error so just return
    }
#endif
    
    // ON events work up through the EVs
    // EV#0 is for produced event so start at 1
    // TODO would be more efficient to get all the EVs in one go and then work through them. getEV() isn't quick)
    for (e=1; e<EVperEVT ;e+=2) { 
        ledNo = evs[e];
        flags = evs[e+1];

        if (ledNo == NO_ACTION) {
            continue;
        }
        // Check for SOD
        if ((ledNo == ACTION_SPECIALS) && (flags == ACTION_SPECIAL_SOD)) {
            // Do the SOD
            doSOD();
            continue;
        }
        // check for a valid action
        if (ledNo > NUM_ACTIONS) {
            continue;
        }
        // check the OPC if this is an ON or OFF event
        if ( ! ((m->opc)&EVENT_ON_MASK)) {
            // ON event
            if (flags & ACTION_FLAGS_ENABLEON) {
                if (flags & ACTION_FLAGS_INVERT_EVENT) {
                    pol = 0;
                } else {
                    pol = 1;
                }
            }
        } else {
            // OFF event
            if (flags & ACTION_FLAGS_ENABLEOFF) {
                if (flags & ACTION_FLAGS_INVERT_EVENT) {
                    pol = 1;
                } else {
                    pol = 0;
                }
            }
        }
        if ((flags & ACTION_FLAGS_FLASH) && (pol == 1)) {
            if (flags & ACTION_FLAGS_INVERT_FLASH) {
                antiFlashLed(ledNo);
            } else {
                flashLed(ledNo);
            }
        } else if (pol == 1) {
            setOn(ledNo);
        } else {
            setOff(ledNo);
        }
    }
    return PROCESSED;
}

/**
 * Helper function used when sending SOD response. Inverts the polarity of event if required and also
 * checks if we are allowed to send on or off events.
 * @param happening the happening number to send
 * @param state the event polarity
 * @param invert whether the event should be inverted
 * @param can_send_on if we are allowed to send ON events 
 * @param can_send_off is we are allowed to send OFF events
 * @return true if the event was able to be sent
 */
Boolean sendInvertedProducedEvent(Happening happening, EventState state, Boolean invert, Boolean can_send_on, Boolean can_send_off) {
	EventState state_to_send = invert?!state:state;
	if ((state_to_send && can_send_on) || (!state_to_send && can_send_off)) {
		return sendProducedEvent(happening, state_to_send);
	} else {
		return TRUE;
	}
}

/**
 * Helper function used when sending SOD response. Inverts the polarity of event if required.
 * @param happening the happening number to send
 * @param state the event polarity
 * @param invert whether the event should be inverted
 * @return true if the event was able to be sent
 */
Boolean alwaysSendInvertedProducedEvent(Happening action, EventState state, Boolean invert) {
    return sendProducedEvent(action, invert?!state:state);
}

/**
 * Do the consumed SOD. 
 * This sets things up so that timedResponse will call back into APP_doSOD() whenever another response is
 * required.
 */
void doSOD(void) {
    startTimedResponse(TIMED_RESPONSE_SOD, findServiceIndex(SERVICE_ID_PRODUCER), sodTRCallback);
}

/**
 * Send one response message and increment the step counter ready for the next call.
 * 
 * Here I use step 0 to 63 for 64 keys.
 * 
 * This is the callback used by the service discovery responses.
 * @param type always set to TIMED_RESPONSE_NVRD
 * @param serviceIndex indicates the service requesting the responses
 * @param step loops through each service to be discovered
 * @return whether all of the responses have been sent yet.
 */
TimedResponseResult sodTRCallback(uint8_t type, uint8_t serviceIndex, uint8_t step) {
    uint8_t io;
    uint8_t happeningIndex;
	Boolean send_on_ok;
	Boolean send_off_ok;
	Boolean event_inverted;
    uint8_t flags;
    EventState value;

    // The step is used to index through the events and the channels
    if (step >= NUM_PB) {
        return TIMED_RESPONSE_RESULT_FINISHED;
    }
    flags = (uint8_t)getNV(NV_PB_FLAGS + step);
    if (!(flags & NV_PB_FLAGS_ENABLE_SOD)) {
        return TIMED_RESPONSE_RESULT_NEXT;
    }
    
    happeningIndex = PB_2_HAPPENING(step);
    event_inverted = flags & NV_PB_FLAGS_POLARITY;
    send_on_ok  = flags & NV_PB_FLAGS_SEND_ON;
    send_off_ok = flags & NV_PB_FLAGS_SEND_OFF;

    value = APP_GetEventState(happeningIndex);
             
    if (value != EVENT_UNKNOWN) {
        sendInvertedProducedEvent(happeningIndex, value, event_inverted, send_on_ok, send_off_ok);
    }
    return TIMED_RESPONSE_RESULT_NEXT;
}

