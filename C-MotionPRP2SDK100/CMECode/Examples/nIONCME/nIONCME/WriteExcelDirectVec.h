//#if defined(__cplusplus)
//extern "C" {
//#endif

#include "PMDdevice.h"
#include <string>
#include <iostream>
#include <vector>

#pragma once
	//class Writing to Excel file with vectors
	//{
PMDresult WriteExcelDirectVec(PMDAxisHandle* phAxis, const std::string& wb, const std::string& ws, std::vector<PMDuint32>& TIME, std::vector<PMDint32>& POS, std::vector<PMDint32>& VeL, std::vector<int>& force,  std::vector<PMDint16>& motCom);
//};


//#if defined(__cplusplus)
//}
//#endif