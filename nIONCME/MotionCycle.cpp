#include "MotionCycle.h"
#include "C-Motion.h"
#include "c-motion.h"
#include "PMDdiag.h"
#include "PMDsys.h"
#include "PMDutil.h"
#include "PMDtypes.h"
#include "PMDperiph.h"

PMDresult MotionCycle(PMDAxisHandle* hAxis1, PMDPeriphHandle* hAnlgPeriph) {

    PMDuint16 status = 0;
    PMDuint16 eventmask;
    PMDint32 pos1, pos2, destpos, position;
    int stop, count, res = 0;
    PMDresult result;
    PMDint32 Defvelocity = 20000, vel, maxvel = 0, tempVel = 0;//
    PMDint16 analogvalue, maxanalg = 0;//
    int sum = 0, pass = 1;
    vel = Defvelocity;//
    eventmask = PMDEventStatusMotionComplete | PMDEventStatusMotionError | PMDEventStatusInPositiveLimit | PMDEventStatusInNegativeLimit;
    //PMDGetVelocity(hAxis1, &tempVel); PMDprintf("Sync: %d\r\n", tempVel);
    PMDSetProfileMode(hAxis1, PMDProfileModeTrapezoidal);
    PMD_RESULT(PMDUpdate(hAxis1));
    PMDSetVelocity(hAxis1, Defvelocity);      // former 32768
    PMDSetAcceleration(hAxis1, 66847);  // former 256
    PMDSetJerk(hAxis1, 65535);
    PMD_RESULT(PMDUpdate(hAxis1)); 
    //PMDGetVelocity(hAxis1, &tempVel); PMDprintf("C Velocity: %d\r\n", tempVel);
    pos1 = 0;
    pos2 = 17000;
    destpos = pos2;
    stop = 0;
    count = 0;
    PMDprintf("Start Motion Cycle\r\n");
    while (!stop) {
        if (count == 4) { stop = 1; res = 0; }
        if (destpos == pos1) destpos = pos2, count++;
        else destpos = pos1;
        PMDSetPosition(hAxis1, destpos); PMDprintf("Count: %d\r\n", count);
        PMDResetEventStatus(hAxis1, 0xfffe);
        PMD_RESULT(PMDUpdate(hAxis1)); 
        PMDGetEventStatus(hAxis1, &status); PMDGetEventStatus(hAxis1, &status); PMDprintf("Status: %d\r\n", status); PMDGetVelocity(hAxis1, &tempVel); PMDprintf("Velocity: %d\r\n", tempVel);
        while (!status) {
            PMDGetEventStatus(hAxis1, &status);
            PMD_RESULT(PMDPeriphRead(hAnlgPeriph, &analogvalue, PMDPIO_AICh1, 1));//
            //PMDprintf("AI read %d\r\n", analogvalue);
            if (sum == 0) { vel = (PMDint32)((((float)analogvalue / 14000) + .5) * Defvelocity); }
            else vel = abs((PMDint32)(Defvelocity - ((float)analogvalue / 14000) * Defvelocity));
            if (abs(vel) > maxvel) maxvel = vel;
            if (abs(analogvalue) > maxanalg) maxanalg = analogvalue;
            if (pass) vel = Defvelocity;
            //PMDprintf("Velocity is "); PMDprintf("%d\n", vel); PMDprintf("\r\n");
            //PMDprintf("Analog value is "); PMDprintf("%.2f\n", (float)((float)analogvalue)); PMDprintf("\r\n");
            PMDSetVelocity(hAxis1, vel);
            PMD_RESULT(PMDUpdate(hAxis1));//
            PMDTaskWait(50); //
            if (status & eventmask)
            {
                PMD_RESULT(PMDResetEventStatus(hAxis1, PMDEventStatusMotionComplete));
                if (status == PMDEventStatusMotionError) { stop = 1; res = 1; }
                else if (status == (PMDEventStatusInPositiveLimit | PMDEventStatusMotionComplete)) {
                    PMDprintf("Positive Limit Switch Reached!\r\n");
                    PMD_RESULT(PMDResetEventStatus(hAxis1, PMDEventStatusInPositiveLimit));
                    return PMD_ERR_MP_InvalidMoveIntoLimit;
                }
                else if (status == (PMDEventStatusInNegativeLimit | PMDEventStatusMotionComplete)) {
                    PMDprintf("Negative Limit Switch Reached!\r\n");
                    PMD_RESULT(PMDResetEventStatus(hAxis1, PMDEventStatusInNegativeLimit));
                    return PMD_ERR_MP_InvalidMoveIntoLimit;
                }
                else { 
                    PMD_RESULT(PMDGetActualPosition(hAxis1, &position));
                    PMDprintf("Motion Complete at position: %ld\r\n", position);
                    PMD_RESULT(PMDResetEventStatus(hAxis1, PMDEventStatusMotionComplete));
                    PMDTaskWait(1000); }
            }
        }
    }
    PMDprintf("Max analog is %d\r\n", maxanalg);
    PMDprintf("Max velocity is %d\r\n", maxvel);
    if (res == 1) return PMD_ERR_MP_MoveWhileInError;
    else return PMD_ERR_OK;
}
#if 0
PMDHandle hSerial;
hSerial = CreateFile("COM5",
    GENERIC_READ | GENERIC_WRITE,
    0,
    0,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    0);

if (hSerial == INVALID_HANDLE_VALUE) {
    if (GetLastError() == ERROR_FILE_NOT_FOUND) {
        //serial port does not exist. Inform user.
    }
    //some other error occurred. Inform user.
}
DCB dcbSerialParams = { 0 };
PMDdcbSerial.DCBlength = sizeof(dcbSerialParams);
if (!GetCommState(hSerial, &dcbSerialParams)) {
    //error getting state
}
dcbSerialParams.BaudRate = CBR_19200;
dcbSerialParams.ByteSize = 8;
dcbSerialParams.StopBits = ONESTOPBIT;
dcbSerialParams.Parity = NOPARITY;
if (!SetCommState(hSerial, &dcbSerialParams)) {
    //error setting serial port state
}
PMDHostInterface_Serial;
#endif // 0