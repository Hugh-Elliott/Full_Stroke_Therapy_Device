
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "c-motion.h", "C-Motion.h"
#include "PMDperiph.h"
#include "PMDsys.h"
#include "PMDdiag.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0

/*
  Most PMD procedure calls return an error code, zero means success.

  Production programs typically check this if a failure is likely, or
  the consequences are severe, or in order to debug.  For the sake of
  simplicity this program does not check any error codes.
*/

/*
  The USER_CODE_VERSION macro allows you to give your program a
  version number that will be reported by Pro-Motion.

  Even if you don't care about versioning, though, exactly one USER_CODE_VERSION must
  be included.
*/
USER_CODE_VERSION(VERSION_MAJOR, VERSION_MINOR);

/*
  Data structures declared at top level, like these, will be allocated in the
  data area, leaving more room in the stack.
*/
PMDAxisHandle hAxis1;


/*
  The USER_CODE_TASK macro must be used as to declare the main
  function that will be run by your user program.  The argument is
  the name of the function.  "main" is not allowed.
*/
USER_CODE_TASK(HelloWorld)
{
    PMDresult result;           /* The error code, check this if in doubt whether a call succeeded. */
    PMDuint32 lastTime;
    PMDint32 pos, lastPos;


    /*
      First we print a cheery message, which should appear on the console.
    */
    PMDputs("Hello, world\r\n");

    /*
      In order to print formatted data to the console, we use PMDprintf, which
      uses the standard printf format string except for floating point.
    */
    PMDprintf("Program version  %d.%d\r\n", VERSION_MAJOR, VERSION_MINOR);

    /*
      In order to get the version number for the on-card Magellan
      Motion Processor, we first need to open an axis handle for it.
      The arguments are:
         1. a pointer to the uninitialized axis handle
         2. a null pointer for the device, meaning use the local device
         3. a constant indicating which axis we want.
    */
    result = PMDAxisOpen(&hAxis1, NULL, PMDAxis1);

    /*
      Now try a C-Motion call with the axis handle.
    */
    result = PMDGetActualPosition(&hAxis1, &pos);
    PMDprintf("Actual position:  %d\r\n", pos);
              
    /*
      Retrieve the time since startup, in milliseconds.
    */
    lastTime = PMDDeviceGetTickCount();

    /* Retrieve the position indicated by the encoder on axis 1, if there is one. */
    result = PMDSetEncoderSource(&hAxis1, PMDEncoderSourceIncremental);
    result = PMDGetActualPosition(&hAxis1, &pos);

    /* Make lastPos different than pos so that we print something the first time. */
    lastPos = pos + 1;

    PMDprintf("Time:   Position\r\n");

    while (!0) 
    {

        /*
          Print the time in milliseconds, and the encoder reading, on the console.
          The time will be truncated to a multiple of 8.
          If you have an encoder connected, you should be able to verify that 
          its reading changes.
        */
        /* Retrieve position again. */
        PMD_RESULT(PMDGetActualPosition(&hAxis1, &pos));

        if (pos != lastPos) 
	    {
            PMDprintf("%d:  %d\r\n", lastTime, pos);
            lastPos = pos;
        }

        /* Wait a while after lastTime was set. */
        PMDTaskWaitUntil(&lastTime, 10);
    }
}


