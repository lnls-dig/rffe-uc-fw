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

   Sample Use Example: https://developer.mbed.org/users/joeata2wh/code/xj-multi-serial-command-listener-example/

   #include "mbed.h"
   #include "multi-serial-command-listener.h"

   Serial pc(USBTX, USBRX);
   char myCommand[SCMD_MAX_CMD_LEN+1];


   void commandCallback(char *cmdIn, void *extraContext) {
   strcpy(myCommand, cmdIn);
   // all our commands will be recieved async in commandCallback
   // we don't want to do time consuming things since it could
   // block the reader and allow the uart to overflow so we simply
   // copy it out in the callback and then process it latter.

   // See data_log one of dependants of this library for example
   // of using *extraContext
   }

   int main() {
   pc.baud(9600);
   pc.printf("Demo multi-serial-command-listener\r\n");

   // Instantiate our command processor for the
   // USB serial line.
   struct SCMD *cmdProc = scMake(&pc, commandCallback, NULL)  ;

   while(1) {
   if (myCommand[0] != 0) {
   pc.printf("Command Recieved =%s\r\n", myCommand);
   myCommand[0] = 0; // clear until we recieve the next one
   }
   wait(0.05);
   }
   }


   ** Basic Therory of Operation **
   The operation is the system will append new characters
   to the buffer until it hits the end.  Whenever it sees
   a \r or \n will insert a \000 (null) so the previous
   characters can be used safely in strcpy.

   It will then set up a null terminator and call
   the user specified callback.

   To minimize risk of invalid data it is recomended the
   caller copies the last_cmd to local buffer using the
   sc_last_cmd_copy(char *dest) which disables interupts
   performs the copy and then re-enables the interupts.
   Otherwise a new character inbound could cause the
   command data to change out from under the user.

   If the buffer fills up with more than SC_MAX_CMD_LEN
   characters what is already present will be treated
   as if it had encountered a \r or \n.

   A cmd must contain at least 1 charcter or it will
   be ignored.

   ** Multiple Listeners **
   The system will allow  upto 10 serial listeners
   to each be processing new character.   It will
   automatically multi-plex between these listeners.
   as needed.  Each listner can have it's own cmd
   callback.

   ** Known limitations **
   # If data arrives fast enough we could have sufficient
   data in uart buffer to contain multiple commands. In
   that instnace it is possible that commands before
   the last command could be over-written before
   calling code can process them.

   # Can have SCMD_MAX_LISTENERS and no current report
   if new listener overflow is provied but it can be
   checked with sc_listener_ndx() which will return the
   index of the listener in queue or SCMD_LISTENER_NOT_FOUND

   # it takes some time to multi-plex across the listeners
   and check each one for a new character. Under very fast
   connection speeds it is possible that arriving data
   could overflow the uart buffer before we can copy the
   data out.

   # There is no provision to detect the same Serial
   connection being used by more than one command
   listener.   This is a problem because the first
   one in the listener queue will get all the data.

   # No current detach processed for listener in
   sc_delete_listener which could increase interupt
   callback overhead.  Need to research what happens
   with multiple attach calls.

   # Consumes at least SCMD_MAX_CMD_LEN memory one for
   inbound buffer.

   ** References **
   https://developer.mbed.org/cookbook/Serial-Interrupts

   NOTE:  I am using a struct instead of a class here
   because I may need to port to PSoC in near future
   and it is unclear when they will get full C++.

   ** TODO **
   Modify the callbacks so they are queued for a timer
   that sends the actual callback to the command processor
   rather than blocking the uart read loop.

   Separate into .h, .c files.
*/

#ifndef serial_command_H
#define serial_command_H

#include "mbed.h"

#define SCMD_MAX_LISTENERS 1
#define SCMD_MAX_CMD_LEN 30
#define SCMD_TOO_MANY_LISTENER -2
#define SCMD_LISTENER_NOT_FOUND -1

struct SCMD {
    RawSerial *sio;
    char buff[SCMD_MAX_CMD_LEN+1];
    short in_ndx;
    char last_char;
    void *callbackExtra;
    void (*callback)(char *, void *);

};

// Add a listener to queue for processing
// so it receives interupts from the Serial
// IO in wrk->sio
int sc_add_listener(struct SCMD *wrk);

// Removes listener from queue and frees up it's
// memory.  After calling this the wrk pointer
// will no longer be valid.  Returns the index
// in queue where it found the listener or
// SCMD_LISTENER_NOT_FOUND if it was not found
// in queue.
int sc_delete_listener(struct SCMD *wrk);

// returns the index in queue where the wrk
// listener is found or SCMD_LISTENER_NOT_FOUND
// if not present in the queue.
int sc_listener_ndx(struct SCMD *wrk);


// constuct a new listener for the specified
// serial IO and add it to the listen queue
// to read inbound data on that port.
// callback extra is a extra pointer to allow recievers
// to rebuild state as needed.  They are expected
// to cast it to something useful
struct SCMD *scMake(RawSerial *sio, void (*callback)(char *, void *), void *callbackExtra);

// process any inbound characters available
// in the uart buffer for the Serial device
// associated with wrk->sio.  Also detects
// end of command and call the command
// processor callback.
void sc_rx_process(struct SCMD *wrk);

// Process all characters available in all the
// uart buffers for our listeners
void sc_rx_interrupt();

#endif
