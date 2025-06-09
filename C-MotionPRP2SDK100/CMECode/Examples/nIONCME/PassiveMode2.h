#ifndef PASSIVEMODE2_H
#define PASSIVEMODE2_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "PMDdevice.h"

#pragma once
	//class Used to go through a movement cycle
	//{
	PMDresult PassiveMode2(PMDPeriphHandle* hPeriphSer, PMDAxisHandle* phAxis, PMDPeriphHandle* hPeriphLOAD);
	//};


#if defined(__cplusplus)
}
#endif
#endif // !PASSIVEMODE2_H