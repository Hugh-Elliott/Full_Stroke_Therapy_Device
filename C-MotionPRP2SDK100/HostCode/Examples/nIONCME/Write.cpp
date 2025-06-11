#include "Write.h"
#include <iostream>
#include "Write2CSV.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"
#include "PMDtypes.h"
#include "TraceTest.h"

int head = 0;
//PMDuint32 TRACETIMESTART;

PMDresult Write(PMDAxisHandle* hAxis1) {
	PMDresult result = PMD_ERR_OK;
    PMDuint8 actposloc = 0x05, motcomloc = 0x07, ictimeloc = 0x08;
    PMDint32 actpos = 0, motcom = 0, ictemp = 0;
    PMDuint32 ictemp2 = 0;
    double ictime = 0, ictime2 = 0;
    int TEST = 1;
    PMDuint32 cycTime;
    PMD_RESULT(PMDGetSampleTime(hAxis1, &cycTime));

    // Create an instance of Write2CSV and open the file "output.csv"
    Write2CSV csv("output.csv");

    if (TEST == 0) {
        // Write the header row
        std::vector<std::string> header = { "Name", "Age", "Country" };
        csv.writeHeader(header);

        // Write some data rows
        std::vector<std::string> row1 = { "John Doe", "28", "USA" };
        std::vector<std::string> row2 = { "Jane Smith", "34", "UK" };
        csv.writeRow(row1);
        csv.writeRow(row2);

        PMD_RESULT(PMDGetTraceValue(hAxis1, actposloc, &actpos));
        PMD_RESULT(PMDGetTraceValue(hAxis1, motcomloc, &motcom));
        std::vector<std::string> row3 = { std::to_string(actpos), std::to_string(0), std::to_string(0) };
        csv.writeRow(row3);
    }
    else {
        if (head == 0) {
            // Write the header row
            std::vector<std::string> header = { "Cycle", "GetTime - cycle", "Actual Position", "Motor Command", "Temp", "Temp2", "STARTTIME"};
            csv.writeHeader(header);
            head = 1;
        }
        std::vector<std::string> row;
        PMD_RESULT(PMDGetTraceValue(hAxis1, ictimeloc, &ictemp));
        PMD_RESULT(PMDGetTime(hAxis1, &ictemp2));
        PMD_RESULT(PMDGetTraceValue(hAxis1, actposloc, &actpos));
        PMD_RESULT(PMDGetTraceValue(hAxis1, motcomloc, &motcom));
        ictime = (ictemp - TRACETIMESTART)*cycTime * 0.000001;
        ictime2 = (ictemp2 - TRACETIMESTART) * cycTime * 0.000001;
        row.clear();
        row.push_back(std::to_string(ictime));
        row.push_back(std::to_string(ictime2));
        row.push_back(std::to_string(actpos));
        row.push_back(std::to_string(motcom));
        //row.push_back(std::to_string(temp));
        //row.push_back(std::to_string(temp2));
        //row.push_back(std::to_string(TRACETIMESTART));

        csv.writeRow(row);
    }



    //std::cout << "Data written to CSV successfully." << std::endl;


	return result;
}