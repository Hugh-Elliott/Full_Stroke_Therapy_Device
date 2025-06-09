#ifndef WRITE2XLSX_H
#define WRITE2XLSX_H

#include <xlsxwriter.h>
#include <string>
#include <vector>
#include <map>

class Write2XLSX {
private:
    lxw_workbook* workbook;  // Pointer to the workbook
    lxw_worksheet* worksheet; // Pointer to the worksheet
    int currentRow;          // Track the current row number
    std::string currentWorksheetName; // Track the name of the current worksheet

    std::map<std::string, lxw_worksheet*> worksheets; // Map of worksheet names to worksheet pointers
    std::map<std::string, int> worksheetsRow; // Map to store the last written row for each worksheet
    std::map<std::string, bool> headerWritten; // Map to track if the header has been written for each worksheet

public:
    // Constructor to open or create an XLSX file
    Write2XLSX(const std::string& filename);

    // Destructor to close the workbook
    ~Write2XLSX();

    // Method to set a specific worksheet by name
    void setWorksheet(const std::string& worksheetName);

    // Method to write a single row of data dynamically
    void writeRow(const std::vector<std::string>& row, const std::vector<std::string>* header = nullptr);

    // Method to write a formula to a specific cell dynamically
    void writeFormula(int row, int col, const std::string& formula);

    // Method to save the file
    void save();

    // Method to check if the header is written
    bool Write2XLSX::isHeaderWritten(const std::string& worksheetName);
};

#endif  // WRITE2XLSX_H
