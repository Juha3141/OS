LOADERSFOLDER = Loader
KERNELFOLDER = Kernel
BINARYFOLDER = Binary
VMWAREFOLDER = VMWare

BASH = bash

QEMU = qemu-system-x86_64

TARGET = OS.img

all: BuildKernel BuildLoaders

BuildKernel:
	make -C $(KERNELFOLDER) all

BuildLoaders:
	make -C $(LOADERSFOLDER) all

clean:
	make -C $(LOADERSFOLDER) clean
	make -C $(KERNELFOLDER) clean

run: virtualbox

qemurun:
	$(QEMU) -hda $(TARGET) -m 8192 -rtc base=localtime -M pc -boot c

debugrun: 
	$(QEMU) -hda $(TARGET) -m 8192 -rtc base=localtime -M pc -boot c -s -S -serial stdio

virtualbox:
	vboxmanage startvm "OS" -E VBOX_GUI_DBG_AUTO_SHOW=true -E VBOX_GUI_DBG_ENABLED=truesw
