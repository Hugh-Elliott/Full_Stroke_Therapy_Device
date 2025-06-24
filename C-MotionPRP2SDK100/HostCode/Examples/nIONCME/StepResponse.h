#ifndef STEPRESPONSE_H
#define STEPRESPONSE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "PMDdevice.h"

#pragma once
	// Step response dynamics
	//{
	PMDresult StepResponse(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* hAxis1);		// Step Response for positive direction
	PMDresult NegStepResponse(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* hAxis1);	// Step Response for negative direction
	//};


#if defined(__cplusplus)
}
#endif

#endif // !STEPRESPONSE_H