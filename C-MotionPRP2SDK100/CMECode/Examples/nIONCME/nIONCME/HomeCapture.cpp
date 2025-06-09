#include "HomeCapture.h"
#include "C-Motion.h"
#include "c-motion.h"
#include "PMDdiag.h"
#include "PMDsys.h"
#include "PMDutil.h"
#include "PMDtypes.h"

void ReverseMove(PMDAxisHandle* phAxis);


PMDresult HomeCapture(PMDAxisHandle* hAxis1) {

    PMDresult result = PMD_NOERROR;
    PMDuint16 RegValue;
    PMDuint16 EventRegValue;
    PMDuint16 PMDSignalEncoderHomeMask;
    PMDuint16 status;
    PMDuint16 PMDEventCaptureReceivedMask;
    PMDint32 value;
    PMDint32 home_pos = 0, currPos = 0;



    PMDprintf("Stating Home task\r\n");// I added
    PMD_RESULT(PMDGetEventStatus(hAxis1, &EventRegValue));
#if 0
    PMD_RESULT(PMDGetSignalSense(hAxis1, &RegValue));
   
    if (1==1) {
        PMDSignalEncoderHomeMask = PMDSignalStatusEncoderHome;
    }
    else {
        PMDSignalEncoderHomeMask = ((RegValue & 0xFFF7) | PMDSignalStatusEncoderHome);
    }
#endif // 0
    if (1==2) {
        PMDEventCaptureReceivedMask = PMDEventStatusCaptureReceived;
    }
    else {
        PMDEventCaptureReceivedMask = ((EventRegValue & 0xFFF7) | PMDEventStatusCaptureReceived);
    }

    PMDprintf("Check Capture\r\n");
#if 0
    PMD_RESULT(PMDSetSignalSense(hAxis1, PMDSignalEncoderHomeMask));
#endif // 0
    PMD_RESULT(PMDGetSignalStatus(hAxis1, &status));
    //PMD_RESULT(PMDGetSignalSense(hAxis1, &RegValue));

    if (!(status & PMDSignalStatusEncoderHome))
    {
        PMDprintf("Home is pressed\r\n");
        PMD_RESULT(PMDResetEventStatus(hAxis1, (0xFFBF & 0xFFF7)));
        PMDGetActualPosition(hAxis1, &currPos); 
        PMDprintf("currPos %d", currPos);
        PMD_RESULT(PMDAdjustActualPosition(hAxis1, -(1000 + currPos)));
        PMDGetActualPosition(hAxis1, &currPos);
        PMDprintf("currPos %d", currPos);
        PMD_RESULT(PMDSetProfileMode(hAxis1, PMDProfileModeTrapezoidal));
        PMD_RESULT(PMDUpdate(hAxis1));
        PMD_RESULT(PMDSetVelocity(hAxis1, 10000));
        PMD_RESULT(PMDSetPosition(hAxis1, 0));
        PMD_RESULT(PMDUpdate(hAxis1));
        PMDTaskWait(500);
        return PMD_ERR_OK;

               
    }

    // now prepare for the capture 
    PMD_ABORTONERROR(PMDSetCaptureSource(hAxis1, PMDCaptureSourceHome));
    // this clears the latched capture bit 
    PMD_RESULT(PMDResetEventStatus(hAxis1, ~PMDEventCaptureReceivedMask)); //dot1
    // this clears the capture hardware 
    PMD_RESULT(PMDGetCaptureValue(hAxis1, &value));
    // start the move into the home switch 
    ReverseMove(hAxis1);

    // wait for the capture 
    PMDprintf("Waiting for capture\r\n");
    do // add timeout
    {
        PMD_RESULT(PMDGetEventStatus(hAxis1, &status));
        if(status & PMDEventStatusMotionError){
            return PMD_ERR_MP_MoveWhileInError;
        }
    } while (!(status & PMDEventCaptureReceivedMask)); //dot2
    PMDprintf("Capture recieved\r\n");
    PMD_RESULT(PMDResetEventStatus(hAxis1, (0xFFBF & 0xFFF7))); // 0xFFBF resets negative limit and 0xFFF7 resets capture recieved
    PMD_RESULT(PMDSetVelocity(hAxis1, 0));
    PMD_RESULT(PMDUpdate(hAxis1));
    PMD_RESULT(PMDSetStopMode(hAxis1, PMDStopModeSmooth));
    PMD_RESULT(PMDUpdate(hAxis1));
    PMD_RESULT(PMDGetCaptureValue(hAxis1, &home_pos)); //dot1
    PMDprintf("Axis stop\r\n");
    
    //Offset
    PMD_RESULT(PMDAdjustActualPosition(hAxis1, -home_pos));

    // Quarter turn forward
    PMD_RESULT(PMDSetProfileMode(hAxis1, PMDProfileModeTrapezoidal));
    PMD_RESULT(PMDUpdate(hAxis1));
    PMD_RESULT(PMDSetPosition(hAxis1, 1000));
    PMD_RESULT(PMDSetVelocity(hAxis1, 10000));      // former 32768
    PMD_RESULT(PMDSetAcceleration(hAxis1, 5000));  // former 256
    PMD_RESULT(PMDSetJerk(hAxis1, 65535));
    PMD_RESULT(PMDUpdate(hAxis1));
    PMDTaskWait(500); 
    PMDGetActualPosition(hAxis1, &currPos); PMD_RESULT(PMDAdjustActualPosition(hAxis1, -currPos));
    return PMD_ERR_OK;
}