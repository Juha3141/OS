BOOTLOADERFOLDER = BootLoader
LOADERSFOLDER = Loader
KERNEL32FOLDER = Kernel32
KERNEL64FOLDER = Kernel64
TEMPORARYFOLDER = Temporary
VMWAREFOLDER = VMWare
# ISOFOLDER = iso
IMGFOLDER = img
OSCDIMG = "/home/juha/Oscdimg"

BASH = bash

QEMU = qemu-system-x86_64
# TARGET = OS.iso
TARGET = OS.img

windows: prepare BuildLoaders BuildKernel32

rebuild:
	qemu-img create Disk.img 128M -f raw

all: prepare BuildLoaders BuildKernel32 BuildKernel64 $(TARGET)

prepare:
	mkdir $(TEMPORARYFOLDER)

BuildLoaders:
	make -C $(LOADERSFOLDER) all

BuildKernel32:
	make -C $(KERNEL32FOLDER) all

BuildKernel64:
	make -C $(KERNEL64FOLDER) all

$(TARGET):
# wine $(OSCDIMG)/oscdimg.exe -b$(TEMPORARYFOLDER)/BootLoader.bin $(ISOFOLDER)/ $(TARGET) -m

	qemu-img create $(TARGET) -f raw 32M
	mkfs.fat $(TARGET)

	dd if=$(TEMPORARYFOLDER)/BootLoader.bin of=$(TARGET) bs=512 count=1 conv=notrunc

# copy
	mcopy -i $(TARGET) $(wildcard $(IMGFOLDER)/*.*) ::

clean:
	rm -rf $(TEMPORARYFOLDER)
	rm -rf $(IMGFOLDER)/BRAIN
	rm -rf $(TARGET)
	make -C $(LOADERSFOLDER) clean
	make -C $(KERNEL32FOLDER) clean
	make -C $(KERNEL64FOLDER) clean

run: virtualbox

qemurun:
	$(QEMU) -hda $(TARGET) -hdb Disk.img -m 8192 -rtc base=localtime -M pc -boot c

debugrun: 
	$(QEMU) -hda $(TARGET) -hdb Disk.img -m 8192 -rtc base=localtime -M pc -boot c -s -S -serial stdio

virtualbox:
	vboxmanage startvm "OS" -E VBOX_GUI_DBG_AUTO_SHOW=true -E VBOX_GUI_DBG_ENABLED=truesw
