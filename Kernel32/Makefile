SOURCESFOLDER = Sources
TEMPORARYFOLDER = ../Temporary
KERNEL32TEMPORARYFOLDER = $(TEMPORARYFOLDER)/Kernel32

CC = gcc
LD = ld
OBJCOPY = objcopy

GCCOPTIONS = -Os -masm=intel -W -Wall -fpack-struct=1 -IHeaders -fno-stack-protector
LDOPTIONS = -m elf_i386

MAINASMSOURCE = Kernel32.asm
ENTRYSOURCE	 = Entry.asm
CO-ASMSOURCE = Assembly.asm
MAINSOURCES = Main.obj string.obj vsprintf.obj BIOS.obj
LINKERSCRIPT = Linker.ld
ELF = $(TEMPORARYFOLDER)/Kernel32.elf
PREP = $(TEMPORARYFOLDER)/Kernel32.bin
ISOFOLDER = ../iso
TARGET = KernelLoader.ldr

prepare:
	mkdir $(KERNEL32TEMPORARYFOLDER)

all: prepare asm $(MAINSOURCES)
	cd $(KERNEL32TEMPORARYFOLDER); $(LD) $(LDOPTIONS) -T ../../Kernel32/$(LINKERSCRIPT) -o ../$(ELF) Entry.obj $(MAINSOURCES) Assembly.obj
	$(OBJCOPY) -O binary $(ELF) $(PREP)
	./KernelLoaderMaker $(TEMPORARYFOLDER)/Loaders/Kernel16.bin $(PREP) $(TEMPORARYFOLDER)/Loaders/APLoader.bin $(ISOFOLDER)/$(TARGET) 2048 0
asm:
	nasm $(SOURCESFOLDER)/$(ENTRYSOURCE) -f elf32 -o $(KERNEL32TEMPORARYFOLDER)/Entry.obj
	nasm $(SOURCESFOLDER)/$(CO-ASMSOURCE) -f elf32 -o $(KERNEL32TEMPORARYFOLDER)/Assembly.obj

%.obj:
	$(CC) -m32 -c $(SOURCESFOLDER)/$*.c -o $(KERNEL32TEMPORARYFOLDER)/$*.obj $(GCCOPTIONS)

clean:
	rm -rf $(TARGET)