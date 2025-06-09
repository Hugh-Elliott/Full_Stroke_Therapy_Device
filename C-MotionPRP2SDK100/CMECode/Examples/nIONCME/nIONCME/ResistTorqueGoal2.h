#ifndef RESISTIVETORQUEGOAL2_H
#define RESISTIVETORQUEGOAL2_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "PMDdevice.h"

#pragma once
	// Resist torque mode with updated goals
	//{
	PMDresult ResistTorqueGoal2(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* hAxis1, PMDPeriphHandle* hAnlgPeriph);
	//};


#if defined(__cplusplus)
}
#endif

#endif // !RESISTIVETORQUEGOAL2_H