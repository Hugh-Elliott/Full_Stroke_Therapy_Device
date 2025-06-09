#include "Write2M.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"
#include "PMDtypes.h"

#include <iostream>
#include <vector>
#include <fstream>
using namespace std;

PMDresult Write2M(PMDAxisHandle* hAxis1, const std::string& wb, std::vector<PMDuint32>& TIME, std::vector<PMDint32>& POS, std::vector<PMDint32>& VeL, std::vector<int>& force, std::vector<PMDint16>& motCom, std::vector<PMDint32>& DesVeL) {
	PMDresult result = PMD_ERR_OK;

	ofstream MyFile(wb.c_str());

    double count2mm = 74.0 / 4000; // Units: mm/count

    // Check if the file is open
    if (MyFile.is_open()) {
        // clears variables
        /*
        MyFile << "clc;" << std::endl;
        MyFile << "clear;" << std::endl;
        MyFile << "clf;" << std::endl;
        */

        // Write the time vector in the desired format
        MyFile << "time = [";
        for (size_t i = 0; i < TIME.size(); ++i) {
            MyFile << TIME[i];
            if (i != TIME.size() - 1) {
                MyFile << ", ";  // Add comma if it's not the last element
            }
        }
        MyFile << "];" << std::endl;

        // write the position vector
        MyFile << "pos = [";
        for (size_t i = 0; i < POS.size(); ++i) {
            MyFile << POS[i];
            if (i != POS.size() - 1) {
                MyFile << ", ";  // Add comma if it's not the last element
            }
        }
        MyFile << "];" << std::endl;

        // write the velocity vector
        MyFile << "vel = [";
        for (size_t i = 0; i < VeL.size(); ++i) {
            MyFile << VeL[i]*count2mm*100;
            if (i != VeL.size() - 1) {
                MyFile << ", ";  // Add comma if it's not the last element
            }
        }
        MyFile << "];" << std::endl;

        // write the direction vector
        MyFile << "force = [";
        for (size_t i = 0; i < force.size(); ++i) {
            MyFile << force[i];
            if (i != force.size() - 1) {
                MyFile << ", ";  // Add comma if it's not the last element
            }
        }
        MyFile << "];" << std::endl;

        // write the motor command vector
        MyFile << "motCom = [";
        for (size_t i = 0; i < motCom.size(); ++i) {
            MyFile << motCom[i]/ double(32768.0 / 100);
            if (i != motCom.size() - 1) {
                MyFile << ", ";  // Add comma if it's not the last element
            }
        }
        MyFile << "];" << std::endl;

        // write the motor command vector
        MyFile << "desvel = [";
        for (size_t i = 0; i < DesVeL.size(); ++i) {
            MyFile << float(count2mm * 100 * DesVeL[i]);
            PMDprintf("DesVeL = %f\r\n", double(1000 * DesVeL[i] * count2mm));
            PMDprintf("DesVeLN = %f\r\n", double(DesVeL[i]));
            //PMDprintf("scale = %f\r\n",double(74/4000));
            if (i != DesVeL.size() - 1) {
                MyFile << ", ";  // Add comma if it's not the last element
            }
        }
        MyFile << "];" << std::endl;

        // Close the file
        MyFile.close();
    }
    else {
        std::cerr << "Unable to open file" << std::endl;
    }


	return result;
}
