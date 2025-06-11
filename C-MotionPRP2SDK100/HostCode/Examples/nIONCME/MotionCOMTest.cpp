#include "MotionCOMTest.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"

int COMFUNC(PMDAxisHandle* phAxis, PMDPeriphHandle* hPeriph);

PMDresult MotionCOMTest(PMDAxisHandle* hAxis1, PMDPeriphHandle* hAnlgPeriph, PMDPeriphHandle* hPeriph2) {
	PMDresult result;
	int dir = 1;

    PMDuint16 status;
    PMDuint16 eventmask;
    int stop, count, res = 0;
    PMDint32 Defvelocity = 20000, vel, tempvel;//
    PMDint16 analogvalue;//
    int sum = 0;
    vel = Defvelocity;//
    tempvel = vel;
    eventmask = PMDEventStatusMotionComplete | PMDEventStatusInPositiveLimit | PMDEventStatusInNegativeLimit;
    PMDprintf("Starting MotionCOMTest.cpp\r\n");
    PMDSetProfileMode(hAxis1, PMDProfileModeVelocity);
    PMDSetVelocity(hAxis1, Defvelocity);      // former 32768
    PMDSetAcceleration(hAxis1, 10000);  // former 256
    PMDSetJerk(hAxis1, 65535);
    PMDUpdate(hAxis1);

    stop = 0;
    count = 0;
    while (!stop) {
        PMDResetEventStatus(hAxis1, 0);
        PMD_RESULT(PMDUpdate(hAxis1));
        PMDGetEventStatus(hAxis1, &status);
        while (!status) {
            PMDGetEventStatus(hAxis1, &status);
            PMD_RESULT(PMDPeriphRead(hAnlgPeriph, &analogvalue, PMDPIO_AICh1, 1));//
            //PMDprintf("AI read %d\r\n", analogvalue);
            if (sum == 0) { tempvel = (PMDint32)((((float)analogvalue / 14000) + .5) * Defvelocity); }
            else tempvel = abs((PMDint32)(Defvelocity - ((float)analogvalue / 14000) * Defvelocity));
            //PMDprintf("Velocity is "); PMDprintf("%d\n", tempvel); PMDprintf("\r\n");
            //PMDprintf("Analog value is "); PMDprintf("%.2f\n", (float)((float)analogvalue)); PMDprintf("\r\n");
            dir = COMFUNC(hAxis1, hPeriph2);
            vel = dir * tempvel;
            PMDSetVelocity(hAxis1, vel);
            PMD_RESULT(PMDUpdate(hAxis1));
            PMDTaskWait(50);
            if (status & eventmask)
            {
            if (status == PMDEventStatusMotionError) { stop = 1; res = 1; }
            else if (status == (PMDEventStatusInPositiveLimit | PMDEventStatusMotionComplete)) {
                PMDprintf("Positive Limit Switch Reached!\r\n");
                PMD_RESULT(PMDResetEventStatus(hAxis1, PMDEventStatusInPositiveLimit));
                PMDSetVelocity(hAxis1, 0);
                PMD_RESULT(PMDUpdate(hAxis1));
                return PMD_ERR_MP_InvalidMoveIntoLimit;
            }
            else if (status == (PMDEventStatusInNegativeLimit | PMDEventStatusMotionComplete)) {
                PMDprintf("Negative Limit Switch Reached!\r\n");
                PMD_RESULT(PMDResetEventStatus(hAxis1, PMDEventStatusInNegativeLimit));
                PMDSetVelocity(hAxis1, 0);
                PMD_RESULT(PMDUpdate(hAxis1));
                return PMD_ERR_MP_InvalidMoveIntoLimit;
            }
            else {
                PMDResetEventStatus(hAxis1, PMDEventStatusMotionComplete);
                PMDResetEventStatus(hAxis1, PMDEventStatusInstructionError);
            }
            }
        }
    }
    if (res == 1) return PMD_ERR_MP_MoveWhileInError;
    else return PMD_ERR_OK;
}