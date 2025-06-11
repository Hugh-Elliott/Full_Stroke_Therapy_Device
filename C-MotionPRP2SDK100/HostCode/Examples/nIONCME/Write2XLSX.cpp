#include "Write2XLSX.h"
#include <iostream>
#include <sstream>

// Constructor to create a new workbook and worksheet
Write2XLSX::Write2XLSX(const std::string& filename) {
    // Create a new workbook and add a worksheet
    workbook = workbook_new(filename.c_str());
    if (!workbook) {
        std::cerr << "Error creating workbook: " << filename << std::endl;
        return;
    }

    // Default to a sheet named "Sheet1"
    worksheet = workbook_add_worksheet(workbook, NULL);
    if (!worksheet) {
        std::cerr << "Error creating worksheet" << std::endl;
        return;
    }

    // Store the default worksheet in the map
    worksheets["Sheet1"] = worksheet;

    // Initialize currentRow to start at 0
    currentRow = 0;
    currentWorksheetName = "Sheet1"; // Track the name of the active worksheet

    // Initialize HEAD map to track headers for each worksheet
    headerWritten ["Sheet1"] = false;
}

// Destructor to close the workbook
Write2XLSX::~Write2XLSX() {
    if (workbook) {
        workbook_close(workbook);
    }
}

// Method to set a specific worksheet by name
void Write2XLSX::setWorksheet(const std::string& worksheetName) {
    // Check if the worksheet already exists
    if (worksheets.find(worksheetName) != worksheets.end()) {
        // If it exists, set the current worksheet to the existing one
        worksheet = worksheets[worksheetName];
        currentRow = worksheetsRow[worksheetName]; // Use the last written row for this worksheet
        currentWorksheetName = worksheetName; // Update currentWorksheetName
    }
    else {
        // If it doesn't exist, create a new worksheet
        worksheet = workbook_add_worksheet(workbook, worksheetName.c_str());
        worksheets[worksheetName] = worksheet;  // Store it for future use

        // Start at the first row for a new worksheet
        currentRow = 0;
        currentWorksheetName = worksheetName; // Set the new worksheet as the active one
    }
}

// Method to write a single row of data dynamically
void Write2XLSX::writeRow(const std::vector<std::string>& row, const std::vector<std::string>* header) {
    // Check if the header has been written for the current worksheet
    if (!headerWritten[currentWorksheetName] && header != nullptr) {
        // Write the header row if not already written
        for (size_t col = 0; col < header->size(); ++col) {
            worksheet_write_string(worksheet, 0, col, (*header)[col].c_str(), NULL);
        }
        // After writing the header, mark it as written
        headerWritten[currentWorksheetName] = true;

    }

    // Write the data row (this will always happen, regardless of whether the header was written)
    for (size_t col = 0; col < row.size(); ++col) {
        worksheet_write_string(worksheet, currentRow, col, row[col].c_str(), NULL);
    }

    // Increment currentRow after writing the data row
    currentRow++;

    // Update the last row written for this worksheet
    worksheetsRow[currentWorksheetName] = currentRow;
}


// Method to write a formula to a specific cell dynamically
void Write2XLSX::writeFormula(int row, int col, const std::string& formula) {
    worksheet_write_formula(worksheet, row, col, formula.c_str(), NULL);
}

// Method to save the file
void Write2XLSX::save() {
    if (workbook) {
        workbook_close(workbook);
        workbook = nullptr; // Nullify the pointer to prevent further usage
    }
}

// Method to check if the header has been written for the current worksheet
bool Write2XLSX::isHeaderWritten(const std::string& worksheetName) {
    return headerWritten[worksheetName];
}

