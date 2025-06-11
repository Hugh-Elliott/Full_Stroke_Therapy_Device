#include "Home.h"
#include "C-Motion.h"
#include "c-motion.h"
#include "PMDdiag.h"
#include "PMDsys.h"
#include "PMDutil.h"
#include "PMDtypes.h"
#include <string>

void Home(PMDAxisHandle* hAxis1) {
    PMDprintf("I entered Home.ccp\r\n");
    PMDresult result = PMD_NOERROR;
    PMDuint16 PMDSignalEncoderHomeMask;
    PMDuint16 status;
    PMDuint16 PMDVelocityContouringProfile;
    PMDuint16 PMDSmoothStopMode;
    PMDuint16 PMDEventCaptureReceivedMask;
    PMDint32 value;
    PMDint32 home_pos;
    PMDSignalEncoderHomeMask = PMDSignalStatusEncoderHome; //3? or PMDSignalStatusBitEncoderHome;
    PMDVelocityContouringProfile = PMDProfileModeVelocity;
    PMDSmoothStopMode = PMDStopModeSmooth;
    PMDEventCaptureReceivedMask = PMDEventStatusCaptureReceived;//3; // FFF7h
    PMDuint16 ping;

    //test if switch is active
    PMD_RESULT(PMDSetSignalSense(hAxis1, PMDSignalEncoderHomeMask)); 
    PMD_RESULT(PMDGetSignalStatus(hAxis1, &status));
    //PMDprintf("%u\n", (unsigned int)PMDSignalEncoderHomeMask);
    //PMDprintf("%u\n", (unsigned int)status);
    //PMDprintf("Status & Mask\r\n");
    //PMDprintf("%u\n", (unsigned int)(status & PMDSignalEncoderHomeMask));
    //PMDprintf(std::to_char(status & PMDSignalEncoderHomeMask));
    if (status & PMDSignalEncoderHomeMask)
    {
        PMDprintf("Moving away from capture\r\n"); //PMD_RESULT(PMDGetActiveOperatingMode(hAxis1, &ping)); PMDprintf("%u\n", (unsigned int)ping);
        PMD_RESULT(PMDSetProfileMode(hAxis1, PMDVelocityContouringProfile));
        PMD_RESULT(PMDSetVelocity(hAxis1, 1000)); // previously 65536
        PMD_RESULT(PMDSetAcceleration(hAxis1, 256));
        PMD_RESULT(PMDUpdate(hAxis1));
        do
        {
            PMD_RESULT(PMDGetSignalStatus(hAxis1, &status));
            //PMDprintf("Encoder Mask Home value\r\n");
            //PMDprintf("%u\n", (unsigned int)PMDSignalEncoderHomeMask);
            //PMDprintf("%u\n", (unsigned int)status);
            //PMDprintf("Status & Mask\r\n");
            //PMDprintf("%u\n", (unsigned int)(status & PMDSignalEncoderHomeMask));
        } while (status & PMDSignalStatusEncoderHome);
        PMD_RESULT(PMDSetStopMode(hAxis1, PMDSmoothStopMode));
        PMD_RESULT(PMDUpdate(hAxis1));
        PMDprintf("Release Home button\r\n");
    }//PMDprintf("Switch deactivated\r\n");
    // Move into switch
    PMD_RESULT(PMDSetProfileMode(hAxis1, PMDVelocityContouringProfile)); //PMD_RESULT(PMDGetActiveOperatingMode(hAxis1, &ping)); PMDprintf("%u\n", (unsigned int)ping);
    PMD_RESULT(PMDSetVelocity(hAxis1, -10000));
    PMD_RESULT(PMDSetAcceleration(hAxis1, 500));
    // now prepare for the capture 
    PMD_RESULT(PMDSetCaptureSource(hAxis1, PMDCaptureSourceHome));
    // this clears the latched capture bit 
    PMD_RESULT(PMDResetEventStatus(hAxis1, ~PMDEventCaptureReceivedMask));
    // this clears the capture hardware 
    PMD_RESULT(PMDGetCaptureValue(hAxis1, &value));
    // start the move into the home switch 
    PMD_RESULT(PMDUpdate(hAxis1)); PMDprintf("Waiting for capture\r\n");

    // wait for the capture 
    do
    {
        PMD_RESULT(PMDGetEventStatus(hAxis1, &status));
        //PMDprintf("%u\n", (unsigned int)PMDEventCaptureReceivedMask);
        //PMDprintf("%u\n", (unsigned int)status);
        PMDprintf("Status & Capture\r\n");
        PMDprintf("%u\n", (unsigned int)!(status & PMDEventCaptureReceivedMask));
    } while (!(status & PMDEventCaptureReceivedMask));
    PMDprintf("Capture recieved\r\n");
    // now that we have the capture, stop the axis 
    PMD_RESULT(PMDSetStopMode(hAxis1, PMDSmoothStopMode));
    PMD_RESULT(PMDUpdate(hAxis1));
    PMD_RESULT(PMDGetCaptureValue(hAxis1, &home_pos));
    PMDprintf("Axis stop\r\n");
    //PMD_RESULT(PMDSetvel); // I added
    // Offset
    PMD_RESULT(PMDAdjustActualPosition(hAxis1, -home_pos)); PMDuint16 pow; //PMD_RESULT(PMDRestoreOperatingMode(hAxis1));
    PMD_RESULT(PMDGetOperatingMode(hAxis1, &pow)); PMDprintf("%u\n", (unsigned int)pow);
    //return result;
}
