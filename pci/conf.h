
/* I'm asigning the last GB of 32bit RAM as PCI MMIO*/
#define PCI_MM_START  0xC0000000U
#define PCI_MM_END    0xFFFFFFFFULL

/*
  * QEMU I/O address space usage:
  *   0000 - 0fff    legacy isa, pci config, pci root bus, ...
  *   1000 - 9fff    free
  *   a000 - afff    hotplug (cpu, pci via acpi, i440fx/piix only)
  *   b000 - bfff    power management (PORT_ACPI_PM_BASE)
  *                  [ qemu 1.4+ implements pci config registers
  *                    properly so guests can place the registers
  *                    where they want, on older versions its fixed ]
  *   c000 - ffff    free, traditionally used for pci io
  *
  * source: SeaBios file: src/fw/pciinit.c:1012
  */
#define PCI_IO_START  0xC000U
#define PCI_IO_END    0xFFFFU

#define MAP_EXP_ROM /*if define the bios will map also the exapnsion rom*/
