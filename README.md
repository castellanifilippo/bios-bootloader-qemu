# BIOS BOOTLOADER QEMU

This project contains a minimal BIOS and Bootloader for x86 architecture, written in C and Assembly for my university thesis.

## Main commands

- `make all`       Builds BIOS, bootloader, and creates the disk image
- `make run`       Runs the BIOS in QEMU
- `make debug`     Runs QEMU with debug options
- `make clean`     Removes all generated files

## Main structure

- `ata/`         ATA driver
- `bootloader/`  Bootloader and print utilities
- `entry/`       Assembly entry point
- `keyboard/`    Keyboard handling
- `loader/`      Bootloader loader
- `main/`        C entry point and print functions
- `pci/`         PCI management
- `scripts/`     Linker scripts
- `utilities/`   Utility functions
- `vga/`         VGA video management

## Notes

- QEMU must be installed to run the system.
- The first version of the project was developed by Michele Castrucci https://github.com/M1ke324/Bios_Qemu.

---

For details on each component, see the respective source files.
