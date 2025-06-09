#include "WriteExcelDirectVec.h"
#include <iostream>
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"
#include "PMDtypes.h"
#include "xlsxwriter.h"
#include <vector>

PMDresult WriteExcelDirectVec(PMDAxisHandle* hAxis1, const std::string& wb, const std::string& ws, std::vector<PMDuint32>& TIME, std::vector<PMDint32>& POS, std::vector<PMDint32>& VeL, std::vector<int>& force, std::vector<PMDint16>& motCom) {
	
	PMDresult result = PMD_ERR_OK;
	// Converts string to char for naming
	const char* WORKSHEET = ws.c_str();
	const char* WORKBOOK = wb.c_str();

	// Sample time
	PMDuint32 cycTime;
	PMD_RESULT(PMDGetSampleTime(hAxis1, &cycTime));

    // Create an instance of Write2XLSX to open the file based on wb
    lxw_workbook* workbook = workbook_new(WORKBOOK);
    if (workbook == NULL) {
        PMDprintf("Error creating workbook! ");
        return PMD_ERR_OK; // no matching pmd error
    }
    lxw_worksheet* worksheet = workbook_add_worksheet(workbook, WORKSHEET);
    if (worksheet == NULL) {
        PMDprintf("Error creating worksheet!");
        workbook_close(workbook);
        return PMD_ERR_OK; // no matching pmd error
    }

    /* Add a number format for motCom. */
    lxw_format* MoT = workbook_add_format(workbook);
    format_set_num_format(MoT, "#.##");
    /* Add a number format for small motCom*/
    lxw_format* MotS = workbook_add_format(workbook);
    format_set_num_format(MoT, "#.######");

    // Write header row
    worksheet_write_string(worksheet, 0, 0, "Time (ms)", NULL);
    worksheet_write_string(worksheet, 0, 1, "Position (counts)", NULL);
    worksheet_write_string(worksheet, 0, 2, "Velocity (counts/ms)", NULL);
    worksheet_write_string(worksheet, 0, 3, "Force (units?)", NULL);
    worksheet_write_string(worksheet, 0, 4, "Motor Command (PWM%)", NULL);
    worksheet_write_string(worksheet, 0, 6, "Delta T (ms)", NULL);

    // Write data rows
    int size = TIME.size();
    double t, p, v, f, m;
    PMDprintf("Start writing to excel\r\n");
    for (int i = 0; i < size; i++) {
        t = TIME[i];
        p = POS[i];
        v = VeL[i];
        f = force[i];
        m = motCom[i] / double(32768 / 100);

        // Write Time to Excel
        worksheet_write_number(worksheet, i + 1, 0, (double)t, NULL);
        // Write Position to Excel
        worksheet_write_number(worksheet, i + 1, 1, (double)p, NULL);
        // Write Velocity to Excel
        worksheet_write_number(worksheet, i + 1, 2, (double)v, NULL);
        // Write Force to Excel
        worksheet_write_number(worksheet, i + 1, 3, (double)f, NULL);
        // Write Motor Command to Excel
        if (m == 0) {
            worksheet_write_number(worksheet, i + 1, 4, (double)m, NULL);
        }
        else if (abs(m) < 0.01) {
            worksheet_write_number(worksheet, i + 1, 4, (double)m, MotS);
        }
        else {
            worksheet_write_number(worksheet, i + 1, 4, (double)m, MoT);
        }
        PMDprintf("Time = %f\r\n", t);
        if (i > 0) {
            // Generate the formula for each row
            char formula[50];
            snprintf(formula, sizeof(formula), "A%d-A%d", i + 2, i + 1); // e.g., "A3-A2", "A4-A3", ...
            // worksheet, row, col, formula, format
            // Write the formula into the corresponding cell in column E
            worksheet_write_formula(worksheet, i + 1, 6, formula, NULL); // G3 corresponds to (2, 6), G4 to (3, 6), etc.
        }
    }

    // Adding Max, Min and Average Delta T
    worksheet_write_string(worksheet, 1, 8, "Max", NULL);
    worksheet_write_formula(worksheet, 2, 8, "=MAX(G:G)", NULL);
    worksheet_write_string(worksheet, 1, 9, "Min", NULL);
    worksheet_write_formula(worksheet, 2, 9, "=MIN(G:G)", NULL);
    worksheet_write_string(worksheet, 1, 10, "Ave", NULL);
    worksheet_write_formula(worksheet, 2, 10, "=AVERAGE(G:G)", NULL);

    // Change Column Width
    worksheet_set_column(worksheet, COLS("B:B"), 14, NULL);
    worksheet_set_column(worksheet, COLS("C:C"), 17, NULL);
    worksheet_set_column(worksheet, COLS("D:D"), 11, NULL);
    worksheet_set_column(worksheet, COLS("E:E"), 15, NULL);

    // Save the Excel file
    fflush(stdout);  // Flush stdout in case print statements are buffered
    if (workbook != NULL) {
        PMDprintf("Closing workbook...\r\n");
        workbook_close(workbook);  // Finalize and close the workbook
    }

    return result;
}