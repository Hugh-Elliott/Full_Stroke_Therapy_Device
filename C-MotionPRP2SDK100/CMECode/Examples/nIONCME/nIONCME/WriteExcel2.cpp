#include "WriteExcel2.h"
#include <iostream>
#include "Write2XLSX.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"
#include "PMDtypes.h"
#include "SetTrace.h"
#include "xlsxwriter.h"

PMDresult WriteExcel2(PMDAxisHandle* hAxis1, const std::string& ws, PMDuint32 time[405], PMDint32 pos[405], PMDint16 motcom[405]){
    PMDresult result = PMD_ERR_OK;
    //PMDint32 actpos = 0, motcom = 0, ictemp = 0;
    //PMDuint32 ictemp2 = 0;
    //double ictime = 0, ictime2 = 0;
    PMDuint32 cycTime;
    PMD_RESULT(PMDGetSampleTime(hAxis1, &cycTime));

    // Create an instance of Write2XLSX to open the file "Step.xlsx"
    Write2XLSX excellllll("TT.xlsx");


    excellllll.setWorksheet("Sheet1");

    std::vector<std::string> header = { "Time (ms)", "Actual Position", "Motor Command" };
    // Only write the header if it hasn't been written yet
    excellllll.writeRow(header, &header);
    
    // Write data rows
    std::vector<std::string> row;
    lxw_workbook* workbook = workbook_new("Step.xlsx");
    lxw_worksheet* worksheet = workbook_add_worksheet(workbook, NULL);
    int i, t, p, m;
    PMDprintf("Start writing to excellllll\r\n");
    for (i = 0; i < 405; i++) {
        row.clear();  // Clear the previous row data
        row.push_back(std::to_string(time[i]));
        row.push_back(std::to_string(pos[i]));
        row.push_back(std::to_string(motcom[i]));
        PMDprintf("time = %d\r\n", time[i]);
        excellllll.writeRow(row);  // Write the data row without the header
        t = time[i];
        p = pos[i];
        m = motcom[i];
        worksheet_write_number(worksheet, i, 0, 5, NULL);
        worksheet_write_number(worksheet, i, 1, 6, NULL);
        worksheet_write_number(worksheet, i, 2, 8, NULL);
    }
    std::vector<std::string> simpleRow = { "1", "2", "3" };
    //excellllll.writeRow(simpleRow);  // Write a simple row

    worksheet_write_number(worksheet, 0, 0, 10, NULL);
    worksheet_write_number(worksheet, 0, 1, 20, NULL);
    worksheet_write_number(worksheet, 0, 2, 30, NULL);

    // Save the Excel file
    excellllll.save();
    PMDprintf("Excel saved!\r\n");
    workbook_close(workbook);

    return result;
}