BINARYFOLDER = ../Binary
LOADERBINARYFOLDER = $(BINARYFOLDER)/Loader
IMGFOLDER = ../img

LOADER = KernelLoader.ldr
TARGET = OS.img

clean:
	make -C x16 clean
	make -C x32 clean

all: Buildx16 Buildx32
	cp ../Kernel/Kernel.krn $(IMGFOLDER) 
	./KernelLoaderMaker $(LOADERBINARYFOLDER)/x16/Kernel16.bin $(LOADERBINARYFOLDER)/x32/Kernel32.bin $(LOADERBINARYFOLDER)/x16/APLoader.bin $(IMGFOLDER)/$(LOADER) 2048 0
	qemu-img create ../$(TARGET) -f raw 32M
	mkfs.fat ../$(TARGET)
	dd if=$(LOADERBINARYFOLDER)/x16/BootLoader.bin of=../$(TARGET) bs=512 count=1 conv=notrunc
	mcopy -i ../$(TARGET) $(IMGFOLDER)/*.* ::

Buildx16:
	make -C x16 all

Buildx32:
	make -C x32 all