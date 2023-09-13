SOURCESFOLDER = Sources
HEADERSFOLDER = Headers
BINARYFOLDER = ../../Binary/Loader
KERNEL32BINARYFOLDER = $(BINARYFOLDER)/x32
LINKERSCRIPT = ../Linker.ld

CC = x86_64-linux-gnu-gcc
LD = x86_64-linux-gnu-ld
OBJCOPY = x86_64-linux-gnu-objcopy
ASM = nasm

CCOPTIONS = -Os -masm=intel -fno-builtin -Wno-implicit-function-declaration -Wno-int-to-pointer-cast -W -Wall -fpack-struct=1 -I$(HEADERSFOLDER) -fno-stack-protector
LDOPTIONS = -m elf_i386
MAINSOURCES = $(notdir $(wildcard $(SOURCESFOLDER)/*.c))
ASMSOURCES = $(notdir $(wildcard $(SOURCESFOLDER)/*.asm))
FIRSTPRIORITYOBJ = $(KERNEL32BINARYFOLDER)/Entry.obj

TARGET = Loader.elf

all: $(ASMSOURCES) $(MAINSOURCES)
	$(LD) $(LDOPTIONS) -T $(LINKERSCRIPT) -o $(KERNEL32BINARYFOLDER)/$(TARGET) $(FIRSTPRIORITYOBJ) $(filter-out $(FIRSTPRIORITYOBJ),$(wildcard $(KERNEL32BINARYFOLDER)/*.obj)) -nostdlib

clean:
	rm -rf $(KERNEL32BINARYFOLDER)/*.*

%.c:
	$(CC) -m32 -c $(SOURCESFOLDER)/$@ -o $(KERNEL32BINARYFOLDER)/$(subst .c,.obj,$@) $(CCOPTIONS)

%.asm:
	$(ASM) $(SOURCESFOLDER)/$@ -f elf32 -o $(KERNEL32BINARYFOLDER)/$(subst .asm,.obj,$@)
