#include "FullMotionRange.h"
#include "C-Motion.h"
#include "c-motion.h"
#include "PMDdiag.h"
#include "PMDsys.h"
#include "PMDutil.h"
#include "PMDtypes.h"

PMDresult FullMotionRange(PMDAxisHandle* hAxis1) {

    PMDuint16 status;
    PMDuint16 eventmask;
    PMDint32 pos1, pos2, destpos;
    int stop, count;
    PMDresult result;

    eventmask = PMDEventStatusMotionComplete | PMDEventStatusMotionError;

    PMDSetProfileMode(hAxis1, PMDProfileModeTrapezoidal);
    PMDSetVelocity(hAxis1, 20000);      // former 32768
    PMDSetAcceleration(hAxis1, 10000);  // former 256
    PMDSetJerk(hAxis1, 65535);
    pos1 = 0;
    pos2 = 20000;
    destpos = pos2;
    stop = 0;
    count = 0;
    while (!stop) {
        if (count == 10) stop = 1;
        if (destpos == pos1) destpos = pos2;
        else destpos = pos1, count++;
        PMDSetPosition(hAxis1, destpos);
        PMDResetEventStatus(hAxis1, 0);
        PMD_RESULT(PMDUpdate(hAxis1));
        PMDGetEventStatus(hAxis1, &status);
        while (!status) {
            PMDGetEventStatus(hAxis1, &status);
            if (status & eventmask)
            {
                if (status == PMDEventStatusMotionError) stop = 1;
                else PMDprintf("Motion Complete\r\n");
            }
        }
    }
    return PMD_ERR_OK;
}