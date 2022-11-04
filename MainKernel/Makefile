SOURCESFOLDER = Sources
HEADERSFOLDER = Headers
TEMPRORYFOLDER = ../Temprory
ESSENTIALLIBRARYFOLDER = EssentialLibrary
ISOFOLDER = ../iso
MAINKERNELTEMPRORYFOLDER = $(TEMPRORYFOLDER)/MainKernel

CC = g++
LD = ld
OBJCOPY = objcopy

CCOPTIONS = -mcmodel=large -ffreestanding -fpack-struct=1 -fno-stack-protector -fno-builtin -W -Wall -I $(HEADERSFOLDER)/ -masm=intel -nostdlib -mno-red-zone -mno-red-zone -mno-mmx -mno-sse -mno-sse2
#  -fomit-frame-pointer -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables
LDOPTIONS = -m elf_x86_64
LINKERSCRIPT = Linker.ld
ELF = $(TEMPRORYFOLDER)/MainKernel.elf
MAINSOURCES = Main.obj MemoryManagement.obj DescriptorTables.obj KernelSystemStructure.obj TextScreen.obj \
ExceptionHandlers.obj PIT.obj Keyboard.obj Mouse.obj IO.obj APIC.obj ACPI.obj

ASMSOURCE = Entry.asm
ASMSOURCE2 = AssemblyInterruptHandler.asm
TARGET = $(ISOFOLDER)/Kernel.krn

prepare:
	mkdir $(MAINKERNELTEMPRORYFOLDER)
	make -C $(ESSENTIALLIBRARYFOLDER) clean all

all: prepare asm $(MAINSOURCES)
	cd $(MAINKERNELTEMPRORYFOLDER); ld $(LDOPTIONS) -T ../../MainKernel/$(LINKERSCRIPT) -o ../$(ELF) Entry.obj AssemblyInterruptHandler.obj $(MAINSOURCES) -L ../ -lEssentialLibrary
	$(OBJCOPY) -O binary $(ELF) $(TARGET) # -j .text -j .data -j .bss -j .rodata -j .rodata1 -R .eh_frame -R .eh_frame_hdr -R .comment

asm:
	nasm $(SOURCESFOLDER)/$(ASMSOURCE) -f elf64 -o $(MAINKERNELTEMPRORYFOLDER)/Entry.obj
	nasm $(SOURCESFOLDER)/$(ASMSOURCE2) -f elf64 -o $(MAINKERNELTEMPRORYFOLDER)/AssemblyInterruptHandler.obj

clean:
	rm -rf $(MAINKERNELTEMPRORYFOLDER)
	rm -rf $(TARGET)

%.obj:
	$(CC) -m64 -c $(SOURCESFOLDER)/$*.cpp -o $(MAINKERNELTEMPRORYFOLDER)/$*.obj $(CCOPTIONS)
