#include "ReverseMove.h"
#include "C-Motion.h"
#include "c-motion.h"
#include "PMDdiag.h"
#include "PMDsys.h"
#include "PMDutil.h"
#include "PMDtypes.h"
//
void ReverseMove(PMDAxisHandle* hAxis1) {

    PMDresult result = PMD_NOERROR;
    PMDuint16 PMDVelocityContouringProfile;
    int vel = -10000;

    PMDVelocityContouringProfile = PMDProfileModeVelocity;

    PMDprintf("Move backwards\r\n");
    PMD_RESULT(PMDSetProfileMode(hAxis1, PMDVelocityContouringProfile));
    PMD_RESULT(PMDUpdate(hAxis1));
    PMD_RESULT(PMDSetVelocity(hAxis1, vel)); // previously 65536
    PMD_RESULT(PMDSetAcceleration(hAxis1, 0.5*abs(vel)));
    PMD_RESULT(PMDUpdate(hAxis1));

    
}