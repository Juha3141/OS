BOOTLOADERFILE = BootLoader.asm
KERNEL16FILE = Kernel16.asm
APLOADERFILE = APLoader.asm
TEMPRORYFOLDER = ../Temprory
ISOFOLDER = ../iso

CROSSLD = i686-linux-gnu-ld

all: prepare
	nasm $(BOOTLOADERFILE) -f bin -o $(TEMPRORYFOLDER)/BootLoader.bin
	nasm $(KERNEL16FILE) -f bin -o $(TEMPRORYFOLDER)/Loaders/Kernel16.bin
	nasm $(APLOADERFILE) -f bin -o $(TEMPRORYFOLDER)/Loaders/APLoader.bin

prepare:
	mkdir $(TEMPRORYFOLDER)/Loaders

clean:
	rm -rf $(TEMPRORYFOLDER)/$(TARGET)
