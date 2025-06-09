#if defined(__cplusplus)
extern "C" {
#endif

#include "PMDdevice.h"

#pragma once
	//class for setting/ starting up trace variables
	//{
	PMDresult SetTrace(PMDAxisHandle* phAxis, PMDuint8 var1, PMDuint8 var2, PMDuint8 var3, PMDuint8 var4);
	extern PMDuint32 TRACETIMESTART;
	//};


#if defined(__cplusplus)
}
#endif