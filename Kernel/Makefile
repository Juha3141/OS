LIBRARYFOLDER = Library
KERNELCOREFOLDER = KernelCore
BINARYFOLDER = ../Binary
LINKERSCRIPT = ../Linker.ld

ARCHITECTURE = x86_64

ELF = Kernel.elf
TARGET = Kernel.krn

LIBRARIES = $(patsubst %.a,%,$(subst lib,-l,$(notdir $(wildcard $(BINARYFOLDER)/$(LIBRARYFOLDER)/*.a))))
FIRSTPRIORITYOBJ = $(BINARYFOLDER)/KernelCore/Entry.obj

clean:
	make -C $(LIBRARYFOLDER) clean
	make -C $(KERNELCOREFOLDER) clean
	make -C $(ARCHITECTURE) clean

	rm -rf $(TARGET)

all: BuildLibrary BuildKernelCore BuildArchitecture
	x86_64-elf-ld -nostdlib -T $(LINKERSCRIPT) -o $(ELF) $(FIRSTPRIORITYOBJ) $(filter-out $(FIRSTPRIORITYOBJ),$(wildcard $(BINARYFOLDER)/KernelCore/*.obj)) $(wildcard $(BINARYFOLDER)/$(ARCHITECTURE)/*.obj) -L $(BINARYFOLDER)/$(LIBRARYFOLDER) $(LIBRARIES)
	x86_64-elf-objcopy -O binary $(ELF) $(TARGET)

BuildLibrary:
	make -C $(LIBRARYFOLDER) all

BuildKernelCore:
	make -C $(KERNELCOREFOLDER) all

BuildArchitecture:
	make -C $(ARCHITECTURE) all
