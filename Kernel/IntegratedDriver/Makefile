CURRENTFOLDER = x86_64

SOURCESFOLDER = Sources
HEADERSFOLDER = Headers
BINARYFOLDER = ../../Binary
MAINBINARYFOLDER = $(BINARYFOLDER)/$(CURRENTFOLDER)
KERNELCOREFOLDER = ../KernelCore

CC = x86_64-elf-g++
ASM = nasm
OBJCOPY = x86_64-elf-objcopy

CCOPTIONS = -g -ffreestanding -fpack-struct=1 -I $(HEADERSFOLDER)/ -I $(KERNELCOREFOLDER)/Headers/ -masm=intel -nostdlib -Werror=return-type -fno-use-cxa-atexit -fno-threadsafe-statics -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings
LDOPTIONS = -m elf_x86_64
MAINSOURCES = $(notdir $(wildcard $(SOURCESFOLDER)/*.cpp))
ASMSOURCES = $(notdir $(wildcard $(SOURCESFOLDER)/*.asm))

all: $(ASMSOURCES) $(MAINSOURCES)
	
clean:
	rm -rf $(MAINBINARYFOLDER)/*.*
	
%.cpp:
	$(CC) -m64 -c $(SOURCESFOLDER)/$@ -o $(MAINBINARYFOLDER)/$(subst .cpp,.obj,$@) $(CCOPTIONS)

%.asm:
	$(ASM) $(SOURCESFOLDER)/$@ -f elf64 -o $(MAINBINARYFOLDER)/$(subst .asm,.obj,$@)