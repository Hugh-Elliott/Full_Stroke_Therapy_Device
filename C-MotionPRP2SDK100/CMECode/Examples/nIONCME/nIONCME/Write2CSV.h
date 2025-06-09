#ifndef WRITE2CSV_H
#define WRITE2CSV_H

#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <type_traits>
#include "PMDtypes.h"

// Class definition
class Write2CSV {
private:
    std::ofstream file;  // Output file stream to write to the CSV
    static bool isFirstWrite;  // Flag to track if it's the first write to the file

    // Helper function to handle writing any type (generic version)
    template <typename T>
    void writeValue(const T& value);

public:
    // Constructor to open the file
    Write2CSV(const std::string& filename);

    // Destructor to close the file
    ~Write2CSV();

    // Method to write a single row to the CSV file
    void writeRow(const std::vector<std::string>& row);

    // Method to write a header row to the CSV file
    void writeHeader(const std::vector<std::string>& header);
};

#endif  // WRITE2CSV_H
