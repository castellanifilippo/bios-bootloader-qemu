#COMPILER
CC = gcc
CFLAG = -fno-pie -no-pie -nostdlib -m32 -std=c99 -ffreestanding -fno-builtin -Wall -Wextra -O2 -g3 -fno-stack-protector -mno-sse -mno-sse2 -mno-mmx -mno-80387 -g

#LINKER
LSCRIPTS = scripts/linker.ld 
LBSCRIPT = scripts/boot.ld 
LFLAG = -T $(LSCRIPTS) -Wl,--no-warn-rwx-segments # -Map=bios.map # to debug the linker scripts
BOOTLOADER_LFLAG = -T $(LBSCRIPT) -Wl,--no-warn-rwx-segments 

#BIN
BINTOOL = objcopy
BINFLAG = -O binary

#DIR
BUILDDIR = build
LOGDIR = logfiles

#MACHINE
QEMU = qemu-system-x86_64
QEMUFLAG = -m 8M -vga std -no-reboot -no-shutdown -bios bios.bin -drive file=disk.img,format=raw
QEMUDEBUG =  -m 8M -trace enable=vga*,enable=pci* -d in_asm,cpu_reset,exec,unimp,int,vpu -D logfiles/logfile$$(date "+%y-%m-%d-%H:%M:%S").txt -S -s  #debug version require to digit "c" to start

#FILE
IN = entry/entry.s utilities/utilities.* vga/vga.* vga/dataVga.* vga/vgaIO.* main/main.c main/biosPrint.* pci/pci* pci/bar.* pci/conf.h ata/ata.* loader/loader.*  keyboard/keyboard.*
ELF = $(BUILDDIR)/bios.elf
BIN = bios.bin

#DISK
DISK = disk.img
DISK_SIZE_SECTORS = 20480 # 10MB

#BOOTLOADER
BOOTLOADER_IN = bootloader/bootloader.S utilities/utilities.* vga/vga.* vga/dataVga.* vga/vgaIO.* bootloader/bootloader.c bootloader/bootPrint.*
BOOTLOADER_ELF = $(BUILDDIR)/bootloader.elf
BOOTLOADER_BIN = bootloader.bin

#PHONY
.PHONY: all clean run cleanLog debug write_bootloader

all: $(BIN) $(DISK) write_bootloader

#COMPILE & RUN
$(ELF): $(IN) $(BUILDDIR) $(LSCRIPTS)
	$(CC) $(IN) $(CFLAG) $(LFLAG) -o $@

$(BIN): $(ELF)
	$(BINTOOL) $(BINFLAG) $(ELF) $@

$(BOOTLOADER_ELF): $(BOOTLOADER_IN) $(BUILDDIR) $(LBSCRIPT)
	$(CC) $(BOOTLOADER_IN) $(CFLAG) $(BOOTLOADER_LFLAG) -o $@

$(BOOTLOADER_BIN): $(BOOTLOADER_ELF)
	$(BINTOOL) $(BINFLAG) $(BOOTLOADER_ELF) $@

$(BUILDDIR):
	mkdir -p $@

run: $(BIN)
	$(QEMU) $(QEMUFLAG)

#DISK
$(DISK):
	dd if=/dev/zero of=$(DISK) bs=512 count=$(DISK_SIZE_SECTORS)
	( \
	echo o; \
	echo n; echo p; echo 1; echo 2048; echo $$(( $(DISK_SIZE_SECTORS) - 1 )); \
	echo a; \
	echo w; \
	) | fdisk $(DISK)

# WRITE BOOTLOADER ON DISK
write_bootloader: $(DISK) $(BOOTLOADER_BIN)
	dd if=$(BOOTLOADER_BIN) of=$(DISK) bs=512 seek=1 conv=notrunc

disk: $(DISK)

#DEBUG
$(LOGDIR):
	mkdir -p $@

debug: $(BIN) $(LOGDIR)
	$(QEMU) $(QEMUFLAG) $(QEMUDEBUG)

#CLEAN
clean: cleanLog
	rm -rf $(BIN) $(BOOTLOADER_BIN) $(BUILDDIR) $(DISK)

cleanLog:
	rm -rf $(LOGDIR)
