#ifndef IMPEDANCE_H
#define IMPEDANCE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "PMDdevice.h"

#pragma once
	// Control scheme acting as a mass, spring, damper system
	//{
	PMDresult Impedance(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* phAxis, PMDPeriphHandle* hPeriphLOAD);
	//};


#if defined(__cplusplus)
}
#endif
#endif // !IMPEDANCE_H