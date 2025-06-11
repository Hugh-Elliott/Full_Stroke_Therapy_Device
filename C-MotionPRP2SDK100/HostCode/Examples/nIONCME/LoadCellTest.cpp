#include "LoadCellTest.h"
#include "PMDperiph.h"
#include "c-motion.h"
#include "PMDsys.h"
#include "PMDdiag.h"
#include "PMDutil.h"
/* Rabbit hole of maybe useful things
	N-ION User Manual
	4.5 I/O Functions
	4.6.4.2 Expansion SPI Port aka muxxing
	4.10.2 Pin Multiplexing Control
*/
#include <chrono>
#include <thread>  // For sleep functionality
#include <iostream>
#include <fstream>
using namespace std;

PMDresult LoadCellTest(PMDAxisHandle* hAxis1, PMDPeriphHandle* hPeriphLoad) {
	PMDresult result = PMD_ERR_OK, sendres = PMD_ERR_OK, recres = PMD_ERR_OK;
	int run = 1, Write = 0, debug = 0, save = 0;
	PMDuint8 TEMP[3] = {}, TEMPBUFFER;
	int inc = 0, end = 200, p;
	PMDuint32 buffer = 10;
	int sensordata = 0, sum = 0, _offset = 0;
	int senarray[200];
	PMDprintf("Starting Load Cell Test\r\n");
	if (debug) end = 10;
	PMDprintf("hAxis1 address: %p, hPeriphLoad address: %p\r\n", hAxis1, hPeriphLoad);

	if (hAxis1 == NULL || hPeriphLoad == NULL) {
		PMDprintf("Invalid handles: hAxis1 or hPeriphLoad is NULL.\r\n");
	}
	if (Write) {
		PMDprintf("Write Mode\r\n");
	}
	else {
		if (debug)
		{
			PMDprintf("Send Debug Mode\r\n");
		}
		else
		{
			PMDprintf("Send Normal Mode\r\n");
		}
	}
	PMDuint16 clockDATA = 0x00000001;
	PMDresult ReadRes = PMD_ERR_OK;

	for (p = 0; p < 10; p++) {
		PMDPeriphSend(hPeriphLoad, &clockDATA, 2, 100);
		PMDPeriphReceive(hPeriphLoad, &TEMP, &buffer, sizeof(TEMP), 100);
		sensordata = TEMP[0] << 16;
		sensordata |= TEMP[1] << 8;
		sensordata |= TEMP[2];
		if (TEMP[0] & 0x80) sensordata |= 0xFF << 24;
		sum += sensordata;
		PMDprintf("Reading: %d\r\n", sensordata);
		PMDTaskWait(200);
	}
	_offset = sum / p;
	PMDprintf("Offset: %d\r\n", _offset);

	while (run) {
		if (Write) {			
			//PMD_RESULT(PMDPeriphWrite(hPeriphLoad, &clockDATA, buffer, sizeof(clockDATA)));
			ReadRes = (PMDPeriphRead(hPeriphLoad, &TEMPBUFFER, 0x228, sizeof(TEMP)));
			/*
			TEMP[0] = TEMPBUFFER[0];
			TEMP[1] = TEMPBUFFER[1];
			TEMP[2] = TEMPBUFFER[2];
			TEMP[3] = TEMPBUFFER[3];
			PMDprintf("TEMP[0] = %u, TEMP[1] = %u, TEMP[2] = %u, TEMP[3] = %u\r\n", TEMP[0], TEMP[1], TEMP[2], TEMP[3]);
			*/
			//TEMP = TEMPBUFFER;
			PMDGetErrorMessage(result);
			PMDprintf("sizeof(TEMP): %zu\r\n", sizeof(TEMP));
			if (ReadRes != PMD_ERR_OK) {
				PMDprintf("PMDPeriphRead failed with error code: %d\r\n", ReadRes);
			}
			else {
				PMDprintf("PMDPeriphRead succeeded, TEMP_BUFFER = %u\r\n", TEMPBUFFER);
			}
			PMDTaskWait(1000);
		}
		else {
			//char clockDATA = 0x01;
			clockDATA = 0x00000001;  // Try sending a 32-bit clock pulse (might be more compatible)
									
			//sendres = (PMDPeriphSend(hPeriphLoad, &clockDATA, sizeof(clockDATA), 0));
			//if (sendres != PMD_ERR_OK) {
				//PMDprintf("Send clock pulse failed: %d\r\n", sendres);
				//return sendres;
			//}
			if (debug) {
				// Debugging the parameters
				PMDprintf("hPeriphLoad Address: %p\r\n", hPeriphLoad);
				PMDprintf("clockDATA (char): 0x%02X\r\n", clockDATA);
				PMDprintf("sizeof(clockDATA): %zu\r\n", sizeof(clockDATA));
				PMDprintf("sizeof(TEMP): %zu\r\n", sizeof(TEMP));
				PMDprintf("buffer address: %p\r\n", &buffer);

				// Assuming you send 24 pulses in a loop
				// Send 24 clock pulses to HX711 (you typically need 24 clock pulses for each conversion)
				for (int pulseCount = 0; pulseCount < 24; pulseCount++) {
					//PMD_RESULT(PMDPeriphSend(hPeriphLoad, &clockDATA, sizeof(clockDATA), 100));  // Send each clock pulse
					sendres = (PMDPeriphSend(hPeriphLoad, &clockDATA, sizeof(clockDATA), 100));
					PMDGetErrorMessage(sendres);
					if (sendres != PMD_ERR_OK) {
						PMDprintf("Send clock pulse failed at pulse %d: 0x%X\r\n", pulseCount, sendres);  // Print error code in hex
						return sendres;
					}
					// Add a 100 ms delay between each pulse to match the 10 samples per second rate
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
				//recres = (PMDPeriphReceive(hPeriphLoad, &TEMP, &buffer, 10, 100));
				// Now that the clock pulses have been sent, read data from the HX711
				recres = PMDPeriphReceive(hPeriphLoad, &TEMP, &buffer, sizeof(TEMP), 100);  // Read data (e.g., 4 bytes for a float)
				if (recres != PMD_ERR_OK) {
					PMDprintf("Receive data failed: %d\r\n", recres);
					return recres;
				}
			}
			else {
				//for (int pulseCount = 0; pulseCount < 25; pulseCount++) {
					sendres = (PMDPeriphSend(hPeriphLoad, &clockDATA, 2, 100));
				//}
				recres = (PMDPeriphReceive(hPeriphLoad, &TEMP, &buffer, sizeof(TEMP), 100));
				//PMDprintf("Receive data: %d\r\n", recres);
				//PMDprintf("Send data: %d\r\n", sendres);
				sensordata = TEMP[0] << 16;
				sensordata |= TEMP[1] << 8;
				sensordata |= TEMP[2];
				if (TEMP[0] & 0x80) sensordata |= 0xFF << 24;
				//senarray[inc] = sensordata;
			}
		}
		sensordata = sensordata - _offset;
		PMDprintf("Reading: %d\r\n", sensordata);
		PMDGetErrorMessage(sendres);
		PMDGetErrorMessage(recres);
		PMDTaskWait(200);
		if (inc > end) {
			PMDprintf("\r\n");
			inc = 0;
			if (save) {
				run = 0;
			}
			else run = 1;
		}
		inc = inc + 1;
		switch (sendres) // Used to check if there was a timeout when sending
		{
		case PMD_ERR_Timeout:
			PMDprintf("Send timeout\r\n");
			break;
		case PMD_ERR_OK:
			//PMDprintf("Send OK\r\n");
			break;
		default:
			PMDprintf("Send Other Error\r\n");
			break;
		}
		switch (recres)
		{
		case PMD_ERR_Timeout:
			PMDprintf("Recieve timeout\r\n");
			break;
		case PMD_ERR_OK:
			//PMDprintf("Recieve OK\r\n");
			break;
		default:
			PMDprintf("Recieve Other Error\r\n");
			break;
		}
	}
	if (save) {
		ofstream MyFile("s750.m");
		int size = sizeof(senarray) / sizeof(senarray[0]);
		MyFile << ("sen750 = [");
		for (int j = 0; j < size; j++) {
			MyFile << senarray[j];
			if (j != size - 1) {
				MyFile << ", ";  // Add comma if it's not the last element
			}
		}
		MyFile << "];" << std::endl;

		MyFile.close();
	}
	PMDuint16 STAT = 0; //TEMP = 0;
	PMDGetEventStatus(hAxis1, &STAT);
	PMDprintf("Status = %d\r\n", STAT);
	PMDprintf("Stopping\r\n");
	return result;
}