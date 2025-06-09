/******************************************************************************
 *
 * linkPMD.ld - Linker script for PMD N-Series ION C-Motion Engine applications
 *
 * Copyright (c) 2020 Performance Motion Devices, Inc.  
 *
 *****************************************************************************/

MEMORY
{
	FLASH (rx)  : ORIGIN = 0x08140000, LENGTH = 256K
	  RAM (rwx) : ORIGIN = 0x20010000, LENGTH = 64K
}

SECTIONS
{
	.text :
	{
		KEEP(*(header))
		KEEP(*(CMEversion))
		FILL(0xFF)
		. = 0x18;
		KEEP(*(filename))
		KEEP(*(fileversion))
		. = 0x20;
		KEEP(*(date))
		. = 0x40;
		KEEP(*(funcstart))
		. = 0x60;
		KEEP(*(functable))
		. = . + 4;
		KEEP(*(funcstack))
		. = . + 4;
		KEEP(*(funcnames))
		*(.text*)
		*(.rodata*)
		. = ALIGN(8);
		_text_end = .;
	} > FLASH

	/* initialized data in RAM which is copied in startup.c */
	.data : 
	{
		_data_start = .;
		*(.data)
		*(.data*)
		. = ALIGN(8);
		_data_end = .;
	} > RAM AT > FLASH

	/* uninitialized data in RAM */
	.bss :
	{
		_bss_start = .;
		*(.bss*)
		*(COMMON)
		. = ALIGN(8);
		end = .;
		_end = .;
		_bss_end = .;
	} > RAM

	_bss_size = SIZEOF(.bss);
	_data_size = SIZEOF(.data);
	_data_load_start = LOADADDR(.data);

}

