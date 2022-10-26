SOURCESFOLDER = Sources
HEADERSFOLDER = Headers
TEMPRORYFOLDER = ../Temprory
ESSENTIALLIBRARYFOLDER = EssentialLibrary
ISOFOLDER = ../iso
MAINKERNELTEMPRORYFOLDER = $(TEMPRORYFOLDER)/MainKernel

CC = g++
LD = ld
OBJCOPY = objcopy

CCOPTIONS = -g -mcmodel=large -m64 -ffreestanding -fpack-struct=1 -fno-stack-protector -fno-builtin -W -Wall -I $(HEADERSFOLDER)/ -masm=intel -nostdlib -mno-red-zone
LDOPTIONS = -m elf_x86_64
LINKERSCRIPT = Linker.ld
ELF = $(TEMPRORYFOLDER)/MainKernel.elf
MAINSOURCES = Main.obj DescriptorTables.obj KernelSystemStructure.obj TextScreen.obj \
ExceptionHandlers.obj PIT.obj Keyboard.obj IO.obj APIC.obj ACPI.obj MemoryManagement.obj

ASMSOURCE = Entry.asm
TARGET = $(ISOFOLDER)/Kernel.krn

prepare:
	mkdir $(MAINKERNELTEMPRORYFOLDER)
	make -C $(ESSENTIALLIBRARYFOLDER) clean all

all: prepare asm $(MAINSOURCES)
	cd $(MAINKERNELTEMPRORYFOLDER); bash -c "ld $(LDOPTIONS) -T ../../MainKernel/$(LINKERSCRIPT) -o ../$(ELF) Entry.obj $(MAINSOURCES) -L ../ -lEssentialLibrary"
	$(OBJCOPY) -O binary $(ELF) $(TARGET)

asm:
	nasm $(SOURCESFOLDER)/$(ASMSOURCE) -f elf64 -o $(MAINKERNELTEMPRORYFOLDER)/Entry.obj

clean:
	rm -rf $(MAINKERNELTEMPRORYFOLDER)
	rm -rf $(TARGET)

%.obj:
	$(CC) -m64 -c $(SOURCESFOLDER)/$*.cpp -o $(MAINKERNELTEMPRORYFOLDER)/$*.obj $(CCOPTIONS)
