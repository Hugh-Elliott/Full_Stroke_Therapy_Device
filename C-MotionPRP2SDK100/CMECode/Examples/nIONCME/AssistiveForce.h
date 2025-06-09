#ifndef ASSISTIVEFORCE_H
#define ASSISTIVEFORCE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "PMDdevice.h"

#pragma once
	// Assistive mode based on force readings
	//{
	PMDresult AssistiveForce(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* phAxis, PMDPeriphHandle* hPeriphLOAD);
	//};


#if defined(__cplusplus)
}
#endif
#endif // !ASSISTIVEFORCE_H