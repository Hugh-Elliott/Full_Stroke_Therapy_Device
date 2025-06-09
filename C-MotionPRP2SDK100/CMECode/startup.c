#include <stdio.h>
#include <string.h>

extern char _bss_start[];
extern char _bss_size[];
extern char _data_start[];
extern char _data_size[];
extern char _data_load_start[];

void CopyDataSectionToRAM()
{
	if (_data_start != _data_load_start)
	{
		memcpy(_data_start, _data_load_start, (size_t)_data_size);
	}
	
	/* Zero bss */
	memset(_bss_start, 0, (size_t)_bss_size);
}
