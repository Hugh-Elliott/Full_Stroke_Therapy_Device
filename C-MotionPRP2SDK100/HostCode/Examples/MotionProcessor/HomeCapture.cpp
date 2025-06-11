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
    PMDint32 home_pos;



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
        if (status & PMDEventStatusMotionError) {
            return PMD_ERR_MP_MoveWhileInError;
        }
    } while (!(status & PMDEventCaptureReceivedMask)); //dot2
    PMDprintf("Capture recieved\r\n");
    PMD_RESULT(PMDResetEventStatus(hAxis1, PMDEventStatusInNegativeLimit));
    PMD_RESULT(PMDSetStopMode(hAxis1, PMDStopModeSmooth));
    PMD_RESULT(PMDUpdate(hAxis1));
    PMD_RESULT(PMDGetCaptureValue(hAxis1, &home_pos)); //dot1
    PMDprintf("Axis stop\r\n");
    
    //Offset
    PMD_RESULT(PMDAdjustActualPosition(hAxis1, -home_pos));

    // Quarter turn forward
    PMDSetProfileMode(hAxis1, PMDProfileModeTrapezoidal);
    PMDSetPosition(hAxis1, 1000);
    PMDSetVelocity(hAxis1, 10000);      // former 32768
    PMDSetAcceleration(hAxis1, 5000);  // former 256
    PMDSetJerk(hAxis1, 65535);
    PMDUpdate(hAxis1);
    PMDTaskWait(2000);
    PMD_RESULT(PMDResetEventStatus(hAxis1, 0xFFBE));
    return PMD_ERR_OK;
}