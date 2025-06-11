#include "TraceTest.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"

int traceSetup;
PMDuint32 TRACETIMESTART;

PMDresult TraceTest(PMDAxisHandle* hAxis1) {
	PMDresult result;
    PMDuint8 comvel = 0, actvel = 0, actposloc = 0x05;
    PMDint32 actpos = 0; // 0x05
    PMDint16 ActMotCom = 0; // 0x07


    result = PMD_ERR_OK;

    // Trace Setup
    if (!traceSetup)
    {
        PMD_RESULT(PMDSetTracePeriod(hAxis1, 100));
        PMD_RESULT(PMDSetTraceVariable(hAxis1, 0, 0, 0x07)); // Sets trace variable 1 to active motor command
        PMD_RESULT(PMDSetTraceVariable(hAxis1, 1, 0, 0x05)); // Sets trace variable 2 to actual position
        PMD_RESULT(PMDSetTraceVariable(hAxis1, 2, 0, 0x08)); // Sets trace variable 3 to Motion IC time
        PMD_RESULT(PMDSetTraceMode(hAxis1, PMDTraceModeRollingBuffer));
        PMD_RESULT(PMDGetTime(hAxis1, &TRACETIMESTART));
        PMD_RESULT(PMDSetTraceStart(hAxis1, 0, 0, 0, 0));
        PMDprintf("Setting Trace Variables.\r\n");
        traceSetup = 1;
    }
    //PMDTaskWait(5000);
    //PMD_RESULT(PMDGetTraceValue(hAxis1, actposloc, &actpos));
    //PMDprintf("%d \r\n", actpos);
    
	return result;
}