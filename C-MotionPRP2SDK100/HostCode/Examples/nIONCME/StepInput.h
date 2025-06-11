#if defined(__cplusplus)
extern "C" {
#endif

#include "PMDdevice.h"

#pragma once
	//class for testing COM communication
	//{
	PMDresult StepInput(PMDAxisHandle* phAxis);
	PMDresult WriteExcel(PMDAxisHandle* hAxis1, PMDuint8 var1, PMDuint8 var2, PMDuint8 var3, PMDuint8 var4);
	PMDresult SetTrace(PMDAxisHandle* hAxis1, PMDuint8 var1, PMDuint8 var2, PMDuint8 var3, PMDuint8 var4);
	//};


#if defined(__cplusplus)
}
#endif