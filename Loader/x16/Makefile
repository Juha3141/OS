BOOTLOADERFILE = BootLoader.asm
KERNEL16FILE = Kernel16.asm
APLOADERFILE = APLoader.asm
BINARYFOLDER = ../../Binary/Loader
x16BINARYFOLDER = $(BINARYFOLDER)/x16
IMGFOLDER = ../img

CROSSLD = i686-linux-gnu-ld

all:
	nasm $(BOOTLOADERFILE) -f bin -o $(x16BINARYFOLDER)/BootLoader.bin
	nasm $(KERNEL16FILE) -f bin -o $(x16BINARYFOLDER)/Kernel16.bin
	nasm $(APLOADERFILE) -f bin -o $(x16BINARYFOLDER)/APLoader.bin

clean:
	rm -rf $(x16BINARYFOLDER)/*.*