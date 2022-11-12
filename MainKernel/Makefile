SOURCESFOLDER = Sources
HEADERSFOLDER = Headers
TEMPRORYFOLDER = ../Temprory
ESSENTIALLIBRARYFOLDER = EssentialLibrary
ISOFOLDER = ../iso
MAINKERNELTEMPRORYFOLDER = $(TEMPRORYFOLDER)/MainKernel

CC = x86_64-elf-g++
LD = ld
OBJCOPY = objcopy

CCOPTIONS = -mcmodel=large -ffreestanding -fpack-struct=1 -fno-exceptions -fno-rtti -fno-stack-protector -fno-builtin -W -Wall -I $(HEADERSFOLDER)/ -masm=intel -nostdlib -mno-red-zone -mno-mmx -mno-sse -mno-sse2
#  -fomit-frame-pointer -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables
LDOPTIONS = -m elf_x86_64
LINKERSCRIPT = Linker.ld
ELF = $(TEMPRORYFOLDER)/MainKernel.elf
MAINSOURCES = Main.obj TextScreen.obj MemoryManagement.obj DescriptorTables.obj KernelSystemStructure.obj \
ExceptionHandlers.obj PIT.obj Keyboard.obj Mouse.obj IO.obj APIC.obj ACPI.obj \
Graphics/Graphic.obj Graphics/VESABIOSExtensions.obj

ASMSOURCE = Entry.asm
ASMSOURCE2 = AssemblyInterruptHandler.asm
TARGET = $(ISOFOLDER)/Kernel.krn

prepare:
	mkdir $(MAINKERNELTEMPRORYFOLDER)
	mkdir $(MAINKERNELTEMPRORYFOLDER)/Graphics
	make -C $(ESSENTIALLIBRARYFOLDER) clean all

all: prepare asm $(MAINSOURCES)
	cd $(MAINKERNELTEMPRORYFOLDER); ld -T ../../MainKernel/$(LINKERSCRIPT) -o ../$(ELF) Entry.obj AssemblyInterruptHandler.obj MemoryManagement_ASM.obj $(MAINSOURCES) -L ../ -lEssentialLibrary
	$(OBJCOPY) -O binary $(ELF) $(TARGET)

asm:
	nasm $(SOURCESFOLDER)/$(ASMSOURCE) -f elf64 -o $(MAINKERNELTEMPRORYFOLDER)/Entry.obj
	nasm $(SOURCESFOLDER)/$(ASMSOURCE2) -f elf64 -o $(MAINKERNELTEMPRORYFOLDER)/AssemblyInterruptHandler.obj
	nasm $(SOURCESFOLDER)/MemoryManagement.asm -f elf64 -o $(MAINKERNELTEMPRORYFOLDER)/MemoryManagement_ASM.obj

clean:
	rm -rf $(MAINKERNELTEMPRORYFOLDER)
	rm -rf $(TARGET)

%.obj:
	$(CC) -m64 -c $(SOURCESFOLDER)/$*.cpp -o $(MAINKERNELTEMPRORYFOLDER)/$*.obj $(CCOPTIONS)
