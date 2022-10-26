BOOTLOADERFILE = BootLoader.asm
KERNEL16FILE = Kernel16.asm
APLOADERFILE = APLoader.asm
KERNEL32FILE = Kernel32.asm
TEMPRORYFOLDER = ../Temprory
ISOFOLDER = ../iso

CROSSLD = i686-linux-gnu-ld

TARGET = KernelLoader.ldr

all: prepare
	nasm $(BOOTLOADERFILE) -f bin -o $(TEMPRORYFOLDER)/BootLoader.bin
	nasm $(KERNEL16FILE) -f bin -o $(TEMPRORYFOLDER)/Loaders/Kernel16.bin
	nasm $(APLOADERFILE) -f bin -o $(TEMPRORYFOLDER)/Loaders/APLoader.bin
	nasm $(KERNEL32FILE) -f elf32 -o $(TEMPRORYFOLDER)/Loaders/Kernel32.obj
	$(CROSSLD) -T Linker.ld -o $(TEMPRORYFOLDER)/Loaders/Kernel32.elf $(TEMPRORYFOLDER)/Loaders/Kernel32.obj -nostdlib
	objcopy -O binary $(TEMPRORYFOLDER)/Loaders/Kernel32.elf $(TEMPRORYFOLDER)/Loaders/Kernel32.bin
	cat $(TEMPRORYFOLDER)/Loaders/Kernel16.bin $(TEMPRORYFOLDER)/Loaders/Kernel32.bin > $(ISOFOLDER)/$(TARGET)
	cp $(TEMPRORYFOLDER)/Loaders/APLoader.bin $(ISOFOLDER)/APLoader.ldr

prepare:
	mkdir $(TEMPRORYFOLDER)/Loaders

clean:
	rm -rf $(TEMPRORYFOLDER)/$(TARGET)