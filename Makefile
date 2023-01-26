BOOTLOADERFOLDER = BootLoader
LOADERSFOLDER = Loader
MAINKERNELFOLDER = Kernel
KERNEL64FOLDER = Kernel64
TEMPRORYFOLDER = Temprory
VMWAREFOLDER = VMWare
ISOFOLDER = iso
OSCDIMG = "/home/juha/Oscdimg"

BASH = bash

QEMU = qemu-system-x86_64
TARGET = OS.iso 

windows: prepare BuildLoaders BuildMainKernel

all: prepare BuildLoaders BuildMainKernel $(TARGET)

prepare:
	mkdir $(TEMPRORYFOLDER)

BuildLoaders:
	make -C $(LOADERSFOLDER) all

BuildMainKernel:
	make -C $(MAINKERNELFOLDER) all

$(TARGET):
	wine $(OSCDIMG)/oscdimg.exe -b$(TEMPRORYFOLDER)/BootLoader.bin $(ISOFOLDER)/ $(TARGET) -m

clean:
	rm -rf $(TEMPRORYFOLDER)
	rm -rf $(ISOFOLDER)/BRAIN
	rm -rf $(TARGET)
	make -C $(LOADERSFOLDER) clean
	make -C $(MAINKERNELFOLDER) clean

run: virtualbox

qemurun:
	$(QEMU) -cdrom $(TARGET) -m 8192 -rtc base=localtime -M pc -boot d

debugrun: 
	$(QEMU) -cdrom $(TARGET) -m 8192 -rtc base=localtime -M pc -boot d -s -S -serial stdio

virtualbox:
	vboxmanage startvm "OS" -E VBOX_GUI_DBG_AUTO_SHOW=true -E VBOX_GUI_DBG_ENABLED=truesw
