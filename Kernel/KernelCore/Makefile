SOURCESFOLDER = Sources
HEADERSFOLDER = Headers
BINARYFOLDER = ../../Binary
MAINKERNELBINARYFOLDER = $(BINARYFOLDER)/KernelCore

CC = x86_64-elf-g++
ASM = nasm
OBJCOPY = x86_64-elf-objcopy
# -fPIC : Position Independent Code
CCOPTIONS = -g -fPIC -ffreestanding -fpack-struct=1 -I $(HEADERSFOLDER)/ -masm=intel -nostdlib -Werror=return-type -fno-stack-protector -fno-use-cxa-atexit -fno-threadsafe-statics -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -Wno-write-strings
LDOPTIONS = -m elf_x86_64
MAINSOURCES = $(notdir $(wildcard $(SOURCESFOLDER)/*.cpp))
ASMSOURCES = $(notdir $(wildcard $(SOURCESFOLDER)/*.asm))

all: $(ASMSOURCES) $(MAINSOURCES)
	echo $(wildcard $(MAINKERNELBINARYFOLDER)/*.obj)

clean:
	rm -rf $(MAINKERNELBINARYFOLDER)/*.*

%.cpp:
	$(CC) -m64 -c $(SOURCESFOLDER)/$@ -o $(MAINKERNELBINARYFOLDER)/$(subst .cpp,.obj,$@) $(CCOPTIONS)

%.asm:
	$(ASM) $(SOURCESFOLDER)/$@ -f elf64 -o $(MAINKERNELBINARYFOLDER)/$(subst .asm,.obj,$@)