#include "LoadCellFunctions.h"
#include "PMDperiph.h"
#include "PMDdiag.h"

LoadCellConfig loadCellConfig = { 119040, 1/205.184006, 25 };  // Initialize offset, scaleFactor and time interval

PMDint32 read(PMDPeriphHandle* hPeriphLoad) {
	int p = 0;
	PMDuint8 TEMP[3];
	PMDint32 sensordata = 0;
	PMDuint16 clockDATA = 0x00000001;
	PMDuint32 buffer = 10;
	PMDresult result;

	PMD_RESULT(PMDPeriphSend(hPeriphLoad, &clockDATA, 2, 20));
	PMD_RESULT(PMDPeriphReceive(hPeriphLoad, &TEMP, &buffer, sizeof(TEMP), 20));
	sensordata = TEMP[0] << 16;
	sensordata |= TEMP[1] << 8;
	sensordata |= TEMP[2];
	if (TEMP[0] & 0x80) sensordata |= 0xFF << 24;
	//PMDTaskWait(100);
	//PMDprintf("Read value: %d\r\n", sensordata);
	return sensordata;
}
// Averages read value
PMDint32 read_average(PMDPeriphHandle* hPeriphLoad, int times) {
	int p = 0;
	PMDuint8 TEMP[3];
	PMDint32 sum = 0;

	if (times <= 0) times = 1;

	for (p = 0; p < times; p++) {
		sum += read(hPeriphLoad);
		//PMDTaskWait(200);
	}
	return sum / times;
}

// Zeroes Load Cell
void tareLoadCell(PMDPeriphHandle* hPeriphLoad, int times) {
	loadCellConfig._offset = read_average(hPeriphLoad, times);
	PMDprintf("Offset: %d\r\n", loadCellConfig._offset);
}
// Returns offset
PMDint32 getTare() {
	return -loadCellConfig._offset;
}
// Manually sets offset
void setTare(PMDint32 offset) {
	loadCellConfig._offset = offset;
}
// Calibrates scale factor
void calibrateScale(PMDPeriphHandle* hPeriphLoad, double weight, int times) {
	loadCellConfig._scaleFactor = (1.0 * weight) / (read_average(hPeriphLoad, times) - loadCellConfig._offset);
	if (loadCellConfig._scaleFactor == 0) loadCellConfig._scaleFactor = 1;
	PMDprintf("Scale Factor: %f\r\n", getScale());
}
// Returns scale factor
float getScale(void) {
	return 1 / loadCellConfig._scaleFactor;
}
// Manually sets scale factor
void setScale(float scale) {
	if (scale == 0) return;
	loadCellConfig._scaleFactor = 1 / scale;
}
// Returns read value with corrected offset
PMDint32 getValue(PMDPeriphHandle* hPeriphLoad, int times) {
	PMDint32 raw = 0;
	raw = read_average(hPeriphLoad, times);
	return raw - loadCellConfig._offset;
}
// Returns scaled and offset value
PMDint32 getUnits(PMDPeriphHandle* hPeriphLoad, int times) {
	PMDint32 sensordata = 0;
	sensordata = getValue(hPeriphLoad, times) * loadCellConfig._scaleFactor;
	return sensordata;
}