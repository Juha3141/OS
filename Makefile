LOADERSFOLDER = Loader
KERNELFOLDER = Kernel
BINARYFOLDER = Binary
VMWAREFOLDER = VMWare

BASH = bash

QEMU = qemu-system-x86_64

TARGET = OS.iso

all: BuildKernel BuildLoaders

BuildKernel:
	make -C $(KERNELFOLDER) all

BuildLoaders:
	make -C $(LOADERSFOLDER) all

clean:
	make -C $(LOADERSFOLDER) clean
	make -C $(KERNELFOLDER) clean

run: virtualbox

qemu:
	$(QEMU) -cdrom $(TARGET) -m 8192 -rtc base=localtime -M pc -boot c

debug: 
	$(QEMU) -cdrom $(TARGET) -m 8192 -rtc base=localtime -M pc -boot c -s -S -serial stdio

qemu_hd_old:
	$(QEMU) -hda $(TARGET) -m 8192 -rtc base=localtime -M pc -boot c

debug_hd_old: 
	$(QEMU) -hda $(TARGET) -m 8192 -rtc base=localtime -M pc -boot c -s -S -serial stdio

virtualbox:
	# qemu-img convert -O qcow2 $(TARGET) $(patsubst %.img,%.qcow2,$(TARGET))
	vboxmanage startvm "OS" -E VBOX_GUI_DBG_AUTO_SHOW=true -E VBOX_GUI_DBG_ENABLED=truesw
