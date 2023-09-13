ISOFOLDER = iso
TARGET = OS.iso
BINARYFOLDER = ../Binary/Loader
KERNEL32BINARYFOLDER = $(BINARYFOLDER)/x32
LOADERTARGET = Loader.elf

clean:
	make -C x32 clean

all: Buildx32
# copy loader to the folder
	cp $(KERNEL32BINARYFOLDER)/$(LOADERTARGET) $(ISOFOLDER)/boot/$(LOADERTARGET)
	grub-mkrescue --output=../$(TARGET) $(ISOFOLDER)/

Buildx32:
	make -C x32 all

