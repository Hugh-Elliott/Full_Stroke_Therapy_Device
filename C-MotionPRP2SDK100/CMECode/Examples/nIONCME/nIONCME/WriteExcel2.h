//#if defined(__cplusplus)
//extern "C" {
//#endif

#include "PMDdevice.h"
#include <string>

#pragma once
	//class Writing to Excel file
	//{
PMDresult WriteExcel2(PMDAxisHandle* phAxis, const std::string& ws, PMDuint32 time[405], PMDint32 pos[405], PMDint16 motcom[405]);
//};


//#if defined(__cplusplus)
//}
//#endif