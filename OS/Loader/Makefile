BOOTLOADERFILE = BootLoader.asm
KERNEL16FILE = Kernel16.asm
APLOADERFILE = APLoader.asm
TEMPORARYFOLDER = ../Temporary
IMGFOLDER = ../img

CROSSLD = i686-linux-gnu-ld

all: prepare
	nasm $(BOOTLOADERFILE) -f bin -o $(TEMPORARYFOLDER)/BootLoader.bin
	nasm $(KERNEL16FILE) -f bin -o $(TEMPORARYFOLDER)/Loaders/Kernel16.bin
	nasm $(APLOADERFILE) -f bin -o $(TEMPORARYFOLDER)/Loaders/APLoader.bin

prepare:
	mkdir $(TEMPORARYFOLDER)/Loaders

clean:
	rm -rf $(TEMPORARYFOLDER)/$(TARGET)
