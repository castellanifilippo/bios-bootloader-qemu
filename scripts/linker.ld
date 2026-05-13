OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH(i386)

ENTRY(entry);

MEMORY
{
    rom (rx) : ORIGIN = 0xFFFE0000 , LENGTH = 128K
}

SECTIONS
{
  /*32 bit, usual sections*/
  . = ORIGIN(rom);
  .text :
  {
    *(.text)
    *(.text.*)
  } > rom

  .rodata :
  {
    *(.rodata)
    *(.rodata.*)
  } > rom

  .data :
  {
    *(.data)
    *(.data.*)
  } > rom

  .bss :
  {
    *(.bss)
    *(.bss.*)
    *(COMMON)
  } > rom

  /*16 bit code, has to be near the entry point to make the jmp possible*/
  .16 :
  {
    . = (ORIGIN(rom) + LENGTH(rom) - 0x10000); /*last 64kB */
    *(.rodata16)
    . = ALIGN(2);
    *(.text16)
  } > rom

  /*real entry point of the bios*/
  .entry :
  {
    . = (ORIGIN(rom) + LENGTH(rom) - 16);/*index of the entrypoint*/
    *(.entry);
    . = (ORIGIN(rom) + LENGTH(rom) -1);
    BYTE(0x00);/*last byte #128kB */
  } > rom

  /DISCARD/ : /*unused sections*/
  {
    *(.note.gnu.property)
    *(.comment)
    *(.eh_frame)
  }
}
