#ifndef PCIINTERNAL
#define PCIINTERNAL
#include "../utilities/utilities.h"

/*
 * pciInterlnal.h is a set of preprocessor directive for the pci.c file
 * this header is not meant to be included outside of that file.
 */

/*===== CLASS, SUBCLASS =====*/
#define BRIDGE                    0x6
#define HOST_BRIDGE               0

/*===== LIMIT =====*/
#define NDEVICES                  32
#define NFUNCTIONS                7

/*===== MASK =====*/
#define IO_BAR_MASK               (~0x3)
#define MM_BAR_MASK_32            (~0xFU)
#define MM_BAR_MASK_64            (~0xFULL)
#define EXP_ROM_BAR_MASK          (~0x7FF)
#define VENDORID_MASK             0xFFFF
#define TYPE_BAR_MASK             0x06
#define CLASS_CODE_MASK           0xFF000000
#define SUBCLASS_MASK             0x00FF0000
#define INTR_PIN_MASK             0x0000FF00
#define INTR_LINE_MASK            0x000000FF

/*===== OFFSET =====*/
#define CLASS_CODE_OFFSET         0x08
#define PROG_IF_OFFSET            0x09
#define HEADER_OFFSET             0x0c
#define INTR_PIN_LINE_OFFSET      0x3c
#define COMMAND_OFFSET            0x04

/*===== SPECIAL VALUE =====*/
#define NORMAL_DEVICE             0
#define NO_INT                    0
#define NO_FUNCTION               0xFFFF

/*===== BIT =====*/
#define BAR64BIT_BIT              0x04
#define MULTIFUNCTION_BIT         0x80
#define ENABLE_ROM_BIT            1
#define ENABLE_IO_MAP_BIT         1
#define ENABLE_MM_MAP_BIT         2
#define ENABLE_BUS_MASTER_BIT     4
#define VGA_PALETTE_SNOOP_BIT     0x0020

/*===== ARRAYS ======*/
#define SIZE_MM_ADDRESS           0x10000
#define MMADDRESS                 (SIZE_MM_ADDRESS + sizeof(uint32_t))

#define SIZE_IO_ADDRESS           0x30000
#define IOADDRESS                 (SIZE_IO_ADDRESS + sizeof(uint32_t))

/*===== INTX ======*/
#define INTX_TO_PRIQ(intx,slot) \
    (((intx) + (slot)-2) & 3)       /*source qemu/hw/i386/pc_piix.c:86 */

#define INTX_TO_APIC_PIN(intx,slot) \
    (16+ INTX_TO_PRIQ((intx),(slot)))

#endif
