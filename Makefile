BOOTLOADERFOLDER = BootLoader
LOADERSFOLDER = Loader
KERNEL32FOLDER = Kernel32
KERNEL64FOLDER = Kernel64
TEMPRORYFOLDER = Temprory
VMWAREFOLDER = VMWare
ISOFOLDER = iso
OSCDIMG = "/home/juha/Oscdimg"

BASH = bash

QEMU = qemu-system-x86_64
TARGET = OS.iso 

windows: prepare BuildLoaders BuildKernel32

all: prepare BuildLoaders BuildKernel32 BuildKernel64 $(TARGET)

prepare:
	mkdir $(TEMPRORYFOLDER)

BuildLoaders:
	make -C $(LOADERSFOLDER) all

BuildKernel32:
	make -C $(KERNEL32FOLDER) all

BuildKernel64:
	make -C $(KERNEL64FOLDER) all

$(TARGET):
	wine $(OSCDIMG)/oscdimg.exe -b$(TEMPRORYFOLDER)/BootLoader.bin $(ISOFOLDER)/ $(TARGET) -m

clean:
	rm -rf $(TEMPRORYFOLDER)
	rm -rf $(ISOFOLDER)/BRAIN
	rm -rf $(TARGET)
	make -C $(LOADERSFOLDER) clean
	make -C $(KERNEL32FOLDER) clean
	make -C $(KERNEL64FOLDER) clean

run: virtualbox

qemurun:
	$(QEMU) -cdrom $(TARGET) -hda FAT32.img -m 8192 -rtc base=localtime -M pc -boot c

debugrun: 
	$(QEMU) -cdrom $(TARGET) -hda FAT32.img -m 8192 -rtc base=localtime -M pc -boot c -s -S -serial stdio

virtualbox:
	vboxmanage startvm "OS" -E VBOX_GUI_DBG_AUTO_SHOW=true -E VBOX_GUI_DBG_ENABLED=truesw
