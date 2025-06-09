#include "ResistMode.h"
#include "C-Motion.h"
#include "c-motion.h"
#include "PMDdiag.h"
#include "PMDsys.h"
#include "PMDutil.h"
#include "PMDtypes.h"
#include "PMDperiph.h"

PMDresult ResistMode(PMDAxisHandle* hAxis1, PMDPeriphHandle* hAnlgPeriph) {
	PMDint16 maxanalog = max(14162, 14209, 14226, 14192, 14212);
	//PMDint32 maxvel = max(30231, 30298, 30322, 30274, 30302);

	PMDresult result = PMD_ERR_OK;
	PMDint16 DesForce = maxanalog*0.5;
	PMDint32 pos1 = 0, pos2 = 17000, destpos = pos1, position = 0;
	PMDint32 Defvelocity = 20000, vel = 0, maxvel = (PMDint32)((((float)DesForce/ 14000) + .5) * Defvelocity), tempVel = 0;//
	PMDint16 analogvalue, maxanalg = 0;
	int stop = 0, count = 0;
	PMDuint16 status = 0;
	PMDuint16 eventmask, actOpMode = 0, opMode = 0;

	eventmask = PMDEventStatusMotionComplete | PMDEventStatusMotionError;

	PMD_RESULT(PMDUpdate(hAxis1));
	//PMDGetVelocity(hAxis1, &tempVel); PMDprintf("Velocity: %d\r\n", tempVel);
	PMD_RESULT(PMDSetVelocity(hAxis1, Defvelocity));
	PMD_RESULT(PMDUpdate(hAxis1)); 
	//PMDGetVelocity(hAxis1, &tempVel); PMDprintf("Velocity: %d\r\n", tempVel);

	while (!stop) {
		if (count == 4) { stop = 1;}
		if (destpos == pos1) destpos = pos2, PMDprintf("Moving Positive\r\n");
		else destpos = pos1, count++, PMDprintf("Moving Negative\r\n");
		PMDSetPosition(hAxis1, destpos);
		PMDResetEventStatus(hAxis1, 0xfffe);
		PMD_RESULT(PMDUpdate(hAxis1)); PMDGetVelocity(hAxis1, &tempVel); PMDprintf("Velocity: %d\r\n", tempVel);
		PMDGetEventStatus(hAxis1, &status);
		while (!status) {
			PMDGetEventStatus(hAxis1, &status);
			PMD_RESULT(PMDPeriphRead(hAnlgPeriph, &analogvalue, PMDPIO_AICh1, 1));//
			//PMDprintf("AI read %d\r\n", analogvalue);
			vel = (PMDint32)((((float)analogvalue / 4660) + .5) * Defvelocity);
			if (analogvalue < DesForce) { PMD_RESULT(PMDSetVelocity(hAxis1, vel)); }
			else PMD_RESULT(PMDSetVelocity(hAxis1, maxvel));
			PMD_RESULT(PMDUpdate(hAxis1));
			if (status & eventmask)
			{
				if (status == PMDEventStatusMotionError) { stop = 1;}
				else {
					PMD_RESULT(PMDGetActualPosition(hAxis1, &position));
					PMDprintf("Motion Complete at position: %ld\r\n", position);
					PMDTaskWait(1000);
				}
			}
		}

	}

	return result;
}
