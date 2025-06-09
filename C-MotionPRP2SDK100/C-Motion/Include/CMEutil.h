//
// CMEutil.c :  

// CME support functions 


 
//  | 15 14 13 12 | 11 10  9  8 | 7     6 | 5  4  3  2  1  0  |
//  |             |             |         |                   |
//  |   bitrate   | chipselect  | SPImode |     datasize      |

#define GET_SPIMODE(modeword, datasize, SPImode, bitrate, chipselect) \
datasize   = (modeword & 0x003F) >> 0;  /* 5-32 */ \
SPImode    = (modeword & 0x00C0) >> 6;  /* 0-3  */ \
chipselect = (modeword & 0x0F00) >> 8;  /* 0-4  */ \
bitrate    = (modeword & 0xF000) >> 12; /* 0-7  */

#define SET_SPIMODE(modeword, datasize, SPImode, bitrate, chipselect) \
modeword =  ((bitrate    & 0x000F) << 12) | \
            ((chipselect & 0x000F) << 8) | \
            ((SPImode    & 0x0003) << 6) | \
            ((datasize   & 0x003F) << 0);


void GetFaultCodes(PMDDeviceHandle* phDevice);
void GetDeviceInfo(PMDDeviceHandle* phDevice);
void GetTaskInfo(PMDDeviceHandle* phDevice, int tasknumber);
void UserCodeFunctions(PMDDeviceHandle *phDevice);
int  GetAllTaskInfo(PMDDeviceHandle* phDevice);
PMDresult PrintCMEInfo(PMDDeviceHandle *phDevice);
