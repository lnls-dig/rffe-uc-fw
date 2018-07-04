/* Serial command interface listens to inbound serial interupts
   saves the characters as they arrive and places a pointer to
   the last complete command in the last_cmd array and calls
   user callback when supplied.   Can support simutaneous
   routing of input from multiple SIO ports. It considers
   \n or \r as command terminators The client is responsible
   to parse the command furhter if needed.

   Intended use is for caller to periodically check for
   new commands and execute the last one found.  Or caller
   can setup a callback so when a complete command which
   must be at least 1 character long between \n or \r
   characters.

   ***
   * By Joseph Ellsworth CTO of A2WH
   * Take a look at A2WH.com Producing Water from Air using Solar Energy
   * March-2016 License: https://developer.mbed.org/handbook/MIT-Licence
   * Please contact us http://a2wh.com for help with custom design projects.
   ***
*/

#include "cli.h"

struct SCMD *sc_listeners[SCMD_MAX_LISTENERS]; // set up to support upto 10 listeners

// Add a listener to queue for processing
// so it receives interupts from the Serial
// IO in wrk->sio
int sc_add_listener(struct SCMD *wrk) {
    short ndx;
    for (ndx=0; ndx < SCMD_MAX_LISTENERS; ndx++) {
        if (sc_listeners[ndx] == wrk) {
            return ndx; // listener already present
        }
        if (sc_listeners[ndx] == NULL) {
            sc_listeners[ndx] = wrk; // take available slot
            return ndx;
        }
    }
    return SCMD_TOO_MANY_LISTENER;
}

// Removes listener from queue and frees up it's
// memory.  After calling this the wrk pointer
// will no longer be valid.  Returns the index
// in queue where it found the listener or
// SCMD_LISTENER_NOT_FOUND if it was not found
// in queue.
int sc_delete_listener(struct SCMD *wrk) {
    short ndx;
    for (ndx=0; ndx < SCMD_MAX_LISTENERS; ndx++) {
        if (sc_listeners[ndx] == wrk) {
            sc_listeners[ndx] = NULL;
            free(wrk);
            return ndx; // listener already present
        }
    }
    free(wrk);
    return SCMD_LISTENER_NOT_FOUND;
}


// returns the index in queue where the wrk
// listener is found or SCMD_LISTENER_NOT_FOUND
// if not present in the queue.
int sc_listener_ndx(struct SCMD *wrk) {
    short ndx;
    for (ndx=0; ndx < SCMD_MAX_LISTENERS; ndx++) {
        if (sc_listeners[ndx] == wrk) {
            return ndx; // listener already present
        }
    }
    return SCMD_LISTENER_NOT_FOUND;
}


// constuct a new listener for the specified
// serial IO and add it to the listen queue
// to read inbound data on that port.
// callback extra is a extra pointer to allow recievers
// to rebuild state as needed.  They are expected
// to cast it to something useful
struct SCMD *scMake(RawSerial *sio, void (*callback)(char *, void *), void *callbackExtra) {
    struct SCMD *sc = (struct SCMD *)  malloc(sizeof(struct SCMD));
    sc->sio = sio;
    memset(sc->buff,SCMD_MAX_CMD_LEN+1,0);
    sc->callback = callback;
    sc->callbackExtra = callbackExtra;
    sc_add_listener(sc);
    sc->sio->attach(&sc_rx_interrupt, RawSerial::RxIrq);
    sc->in_ndx = 0;
    return sc;
}


// process any inbound characters available
// in the uart buffer for the Serial device
// associated with wrk->sio.  Also detects
// end of command and call the command
// processor callback.
void sc_rx_process(struct SCMD *wrk) {
    // Loop just in case more than one character is in UART's receive FIFO buffer
    // Stop if buffer full

    while ((wrk->sio->readable())) {
        char cin = wrk->sio->getc();
        wrk->sio->putc(cin);
        if ((cin == 10) || (cin == 13)) { // found CR or LF
            if (wrk->in_ndx > 0) {
                // commands must be at least 1 byte long
                wrk->buff[wrk->in_ndx] = 0;    // add terminating null
                wrk->callback(wrk->buff, wrk->callbackExtra);
            }
            wrk->in_ndx = 0;  // reset for next cycle;
            wrk->buff[0] = 0; // add null terminator
        }
        else {
            // not a CR or LF so must be a valid character
            wrk->buff[wrk->in_ndx] = cin; // add character to the buffer
            wrk->buff[wrk->in_ndx + 1] = 0; // add null terminator just in case
            wrk->in_ndx++;
            //printf("wrk->buff=%s in_ndx=%d\n", wrk->buff, wrk->in_ndx);
            if (wrk->in_ndx >= SCMD_MAX_CMD_LEN) {
                // buffer is full so treat as command
                wrk->callback(wrk->buff, wrk->callbackExtra);
                wrk->buff[0] = 0;
                wrk->in_ndx = 0;
                // add callback here
            }
        }
    }
    return;
}

// Process all characters available in all the
// uart buffers for our listeners
void sc_rx_interrupt() {
    int ndx;
    for (ndx=0; ndx < SCMD_MAX_LISTENERS; ndx++) {
        if (sc_listeners[ndx] != NULL) {
            sc_rx_process(sc_listeners[ndx]);
        }
    }
    return;
}
