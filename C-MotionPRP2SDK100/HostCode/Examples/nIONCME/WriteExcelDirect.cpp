#include "WriteExcelDirect.h"
#include <iostream>
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"
#include "PMDtypes.h"
#include "xlsxwriter.h"

//#include "SetTrace.h"
//#include "Write2XLSX.h"

PMDresult WriteExcelDirect(PMDAxisHandle* hAxis1, const std::string& ws, PMDuint32 time[405], PMDint32 pos[405], PMDint16 motcom[405]) {
    PMDresult result = PMD_ERR_OK;
    PMDuint32 cycTime;
    PMD_RESULT(PMDGetSampleTime(hAxis1, &cycTime));
    const char* WORKSHEET = ws.c_str();;

    // Create an instance of Write2XLSX to open the file "Snail.xlsx"
    //lxw_workbook* workbook = nullptr;
    lxw_workbook* workbook = workbook_new("StepInput3.xlsx");
    if (workbook == NULL) {
        PMDprintf("Error creating workbook! ");
        return PMD_ERR_OK;
    }
    //lxw_worksheet* worksheet = nullptr;
    lxw_worksheet* worksheet = workbook_add_worksheet(workbook, WORKSHEET);
    if (worksheet == NULL) {
        PMDprintf("Error creating worksheet!");
        workbook_close(workbook);
        return PMD_ERR_OK;
    }
    /* Add a number format for motCom. */
    lxw_format* MoT = workbook_add_format(workbook);
    format_set_num_format(MoT, "#.##");

    // Write header row
    worksheet_write_string(worksheet, 0, 0, "Time (ms)", NULL);
    worksheet_write_string(worksheet, 0, 1, "Actual Position", NULL);
    worksheet_write_string(worksheet, 0, 2, "Motor Command", NULL);
    worksheet_write_string(worksheet, 0, 4, "Delta T", NULL);

    // Write data rows
    int i;
    double t, p, m;
    PMDprintf("Start writing to excellllll\r\n");
    for (i = 0; i < 405; i++) {
        // Debugging: print the data being written
        //break;
        t = time[i];
        p = pos[i];
        m = motcom[i] / double(32768 / 100);
   
        // Write Time to Excel
        worksheet_write_number(worksheet, i + 1, 0, (double)t, NULL);      
        // Write Position to Excel
        worksheet_write_number(worksheet, i + 1, 1, (double)p, NULL);       
        // Write Motor Command to Excel
        if (m == 0) {
            worksheet_write_number(worksheet, i + 1, 2, (double)m, NULL);
        }
        else {
            worksheet_write_number(worksheet, i + 1, 2, (double)m, MoT);
        }
        PMDprintf("Time = %f\r\n", t);
        if (i > 0) {
            // Generate the formula for each row
            char formula[50];
            snprintf(formula, sizeof(formula), "A%d-A%d", i+2, i+1); // e.g., "A3-A2", "A4-A3", ...
            // worksheet, row, col, formula, format
            // Write the formula into the corresponding cell in column E
            worksheet_write_formula(worksheet, i+1, 4, formula, NULL); // E3 corresponds to (2, 4), E4 to (3, 4), etc.
        }

    }

    // Adding Max, Min and Average Delta T
    worksheet_write_string(worksheet, 1, 6, "Max", NULL);
    worksheet_write_formula(worksheet, 2, 6, "=MAX(E:E)", NULL);
    worksheet_write_string(worksheet, 1, 7, "Min", NULL);
    worksheet_write_formula(worksheet, 2, 7, "=MIN(E:E)", NULL);
    worksheet_write_string(worksheet, 1, 8, "Ave", NULL);
    worksheet_write_formula(worksheet, 2, 8, "=AVERAGE(E:E)", NULL);

    // Change Column Width
    worksheet_set_column(worksheet, COLS("B:B"), 13, NULL);
    worksheet_set_column(worksheet, COLS("C:C"), 15, NULL);

    // Save the Excel file
    fflush(stdout);  // Flush stdout in case print statements are buffered
    if (workbook != NULL) {
        PMDprintf("Closing workbook...\r\n");
        workbook_close(workbook);  // Finalize and close the workbook
    }

    return result;
}