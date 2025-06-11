#ifndef ASSISTIVETRAJECTORY_H
#define ASSISTIVETRAJECTORY_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "PMDdevice.h"

#pragma once
	// Assistive mode using an impedance controller and a desired x0
	//{
	PMDresult AssistTrajectory(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* phAxis, PMDPeriphHandle* hPeriphLOAD);
	//};


#if defined(__cplusplus)
}
#endif
#endif // !ASSISTIVETRAJECTORY_H