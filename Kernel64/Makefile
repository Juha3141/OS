SOURCESFOLDER = Sources
HEADERSFOLDER = Headers
TEMPORARYFOLDER = ../Temporary
ESSENTIALLIBRARYFOLDER = EssentialLibrary
ISOFOLDER = ../iso
MAINKERNELTEMPORARYFOLDER = $(TEMPORARYFOLDER)/Kernel64

CC = x86_64-elf-g++
OBJCOPY = objcopy

CCOPTIONS = -g -O0 -ffreestanding -fpack-struct=1 -W -Wall -I $(HEADERSFOLDER)/ -masm=intel -nostdlib -Werror=return-type
#  -fomit-frame-pointer -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables
LDOPTIONS = -m elf_x86_64
LINKERSCRIPT = Linker.ld
ELF = $(TEMPORARYFOLDER)/Kernel.elf
MAINSOURCES = Main.obj TextScreen.obj MemoryManagement.obj DescriptorTables.obj KernelSystemStructure.obj \
ExceptionHandlers.obj PIT.obj Keyboard.obj Mouse.obj IO.obj APIC.obj ACPI.obj MPFloatingTable.obj TaskManagement.obj \
ResourceAccessManagement.obj Paging.obj MutualExclusion.obj \
Drivers/DeviceDriver.obj Drivers/StorageDriver.obj Drivers/FileSystemDriver.obj \
Drivers/RAMDisk.obj Drivers/PATA.obj Drivers/PATA_CD.obj Drivers/PCI.obj Drivers/AHCI.obj \
FileSystem/MBR.obj FileSystem/GPT.obj FileSystem/ISO9660.obj FileSystem/FAT16.obj

ASMSOURCE = Entry.asm
ASMSOURCE2 = AssemblyInterruptHandler.asm
TARGET = $(ISOFOLDER)/Kernel.krn

prepare:
	mkdir $(MAINKERNELTEMPORARYFOLDER)
	mkdir $(MAINKERNELTEMPORARYFOLDER)/Drivers
	mkdir $(MAINKERNELTEMPORARYFOLDER)/FileSystem
	make -C $(ESSENTIALLIBRARYFOLDER) clean all

all: prepare asm $(MAINSOURCES)
	cd $(MAINKERNELTEMPORARYFOLDER); bash -c "ld -nostdlib -T ../../Kernel64/$(LINKERSCRIPT) -o ../$(ELF) Entry.obj AssemblyInterruptHandler.obj MemoryManagement_ASM.obj SwitchContext.obj MutualExclusion_ASM.obj $(MAINSOURCES) $(EXCLUDE_OPTIMIZATION) -L ../ -lEssentialLibrary"
	$(OBJCOPY) -O binary $(ELF) $(TARGET)

asm:
	nasm $(SOURCESFOLDER)/$(ASMSOURCE) -f elf64 -o $(MAINKERNELTEMPORARYFOLDER)/Entry.obj
	nasm $(SOURCESFOLDER)/$(ASMSOURCE2) -f elf64 -o $(MAINKERNELTEMPORARYFOLDER)/AssemblyInterruptHandler.obj
	nasm $(SOURCESFOLDER)/MemoryManagement.asm -f elf64 -o $(MAINKERNELTEMPORARYFOLDER)/MemoryManagement_ASM.obj
	nasm $(SOURCESFOLDER)/SwitchContext.asm -f elf64 -o $(MAINKERNELTEMPORARYFOLDER)/SwitchContext.obj
	nasm $(SOURCESFOLDER)/MutualExclusion.asm -f elf64 -o $(MAINKERNELTEMPORARYFOLDER)/MutualExclusion_ASM.obj

clean:
	rm -rf $(MAINKERNELTEMPORARYFOLDER)
	rm -rf $(TARGET)

%.obj:
	$(CC) -m64 -c $(SOURCESFOLDER)/$*.cpp -o $(MAINKERNELTEMPORARYFOLDER)/$*.obj $(CCOPTIONS)