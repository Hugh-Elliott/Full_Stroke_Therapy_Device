#include "WriteExcel.h"
#include <iostream>
#include "Write2XLSX.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"
#include "PMDtypes.h"
#include "SetTrace.h"

// Create an instance of Write2XLSX to open the file "output.xlsx"
Write2XLSX excel("Step.xlsx");

PMDresult WriteExcel(PMDAxisHandle* hAxis1, const std::string& ws, PMDuint8 var1, PMDuint8 var2, PMDuint8 var3, PMDuint8 var4, int save) {
    PMDresult result = PMD_ERR_OK;
    PMDuint8 actposloc = 0x05, motcomloc = 0x07, ictimeloc = 0x08;
    PMDint32 actpos = 0, motcom = 0, ictemp = 0;
    PMDuint32 ictemp2 = 0;
    double ictime = 0, ictime2 = 0;
    int TEST = 1;
    PMDuint32 cycTime;
    PMD_RESULT(PMDGetSampleTime(hAxis1, &cycTime));

    if (TEST == 0) {
        // Write the header row to "Sheet1"
        std::vector<std::string> header = { "Name", "Age", "Country" };
        excel.setWorksheet("Sheet1");
        excel.writeRow({}, &header);

        std::vector<std::string> row1 = { "John Doe", "28", "USA" };
        std::vector<std::string> row2 = { "Jane Smith", "34", "UK" };
        excel.writeRow(row1);
        excel.writeRow(row2);

        PMD_RESULT(PMDGetTraceValue(hAxis1, actposloc, &actpos));
        PMD_RESULT(PMDGetTraceValue(hAxis1, motcomloc, &motcom));
        std::vector<std::string> row3 = { std::to_string(actpos), std::to_string(0), std::to_string(0) };
        excel.writeRow(row3);
    }
    else {
        excel.setWorksheet(ws);

        std::vector<std::string> header = { "Cycle", "GetTime - cycle", "Actual Position", "Motor Command" };
        // Only write the header if it hasn't been written yet
        if (!excel.isHeaderWritten(ws)) {
            excel.writeRow(header, &header);
        }
        // Write data rows
        std::vector<std::string> row;
        PMD_RESULT(PMDGetTraceValue(hAxis1, ictimeloc, &ictemp));
        PMD_RESULT(PMDGetTime(hAxis1, &ictemp2));
        PMD_RESULT(PMDGetTraceValue(hAxis1, actposloc, &actpos));
        PMD_RESULT(PMDGetTraceValue(hAxis1, motcomloc, &motcom));
        ictime = (ictemp - TRACETIMESTART) * cycTime * 0.000001;
        ictime2 = (ictemp2 - TRACETIMESTART) * cycTime * 0.000001;

        row.push_back(std::to_string(ictime));
        row.push_back(std::to_string(ictime2));
        row.push_back(std::to_string(actpos));
        row.push_back(std::to_string(motcom));

        excel.writeRow(row);  // Write the data row without the header
    }
    if (save == 1) {
        // Save the Excel file
        excel.save();
    }

    return result;
}