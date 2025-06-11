#include "Write2CSV.h"
#include <iostream>
#include <sstream>
#include "PMDtypes.h"

// Initialize static flag
bool Write2CSV::isFirstWrite = true;

// Constructor to open the file
Write2CSV::Write2CSV(const std::string& filename) {
    // Open the file in append mode if it's not the first write
    if (isFirstWrite) {
        file.open(filename, std::ios::out);  // Overwrite the file the first time
    }
    else {
        file.open(filename, std::ios::out | std::ios::app);  // Append after that
    }

    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
    }
}

// Destructor to close the file
Write2CSV::~Write2CSV() {
    if (file.is_open()) {
        file.close();
    }
}

// Helper function to handle writing any type (template version)
template <typename T>
void Write2CSV::writeValue(const T& value) {
    // Convert number types to string using ostringstream
    std::ostringstream oss;
    oss << value;
    file << oss.str();  // Write the string representation of the value
}

// Method to write a single row to the CSV file
void Write2CSV::writeRow(const std::vector<std::string>& row) {
    for (size_t i = 0; i < row.size(); ++i) {
        // Directly call writeValue for each item, since it handles both strings and numbers
        // For strings, it writes the string; for numbers, it converts them to strings
        writeValue(row[i]);

        if (i < row.size() - 1) {
            file << ",";  // Separate values with commas
        }
    }
    file << "\n";  // Newline at the end of the row
}

// Method to write a header row to the CSV file
void Write2CSV::writeHeader(const std::vector<std::string>& header) {
    if (isFirstWrite) {
        writeRow(header);
        isFirstWrite = false;  // After writing the header, set the flag to false
    }
}
