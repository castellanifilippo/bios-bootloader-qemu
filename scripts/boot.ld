OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)

ENTRY(_start);

MEMORY
{
    ram (wx) : ORIGIN = 0x100000 , LENGTH = 1M
}

SECTIONS
{
	. =(ORIGIN(ram));
	.text : {
		*(.text)
		*(.text.*)
	} > ram
	_etext = .;
	. = ALIGN(0x1000);
	.rodata : {
		*(.rodata)
		*(.rodata.*)
	} > ram
	. = ALIGN(0x1000);
	.data : { 
		*(.data) 
	} > ram
	_edata = .;
	. = ALIGN(0x1000);
	.bss : { *(.bss) } > ram
	_end = .;

	/DISCARD/ : /*unused sections*/
  	{
		*(.note.gnu.property)
		*(.comment)
		*(.eh_frame)
  	}
}