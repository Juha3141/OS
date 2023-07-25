#include <Essential.h>
#include <BIOS.h>
#include <Main.h>
#include <ISO9660.h>
#include <FAT16.h>

#define KERNEL32

struct QuerySystemAddressMap {
	unsigned int AddressLow;
    unsigned int AddressHigh;
    unsigned int LengthLow;
	unsigned int LengthHigh;
	unsigned int TypeLow;
    unsigned int TypeHigh;
};

void CreatePML4Entry(unsigned int Address);
unsigned int ManualProbe(unsigned int Address);

void Main(void) {
	int i;

	const char Kernel64Name[12] = "KERNEL  KRN";

	unsigned int RootDirectoryLocation = 0x500;

	unsigned int KernelSectorLocation;
	unsigned int KernelSectorSize;
	unsigned int KernelSize;

	unsigned int TotalSectorCount;
	
	unsigned int Address = KERNEL64_ADDRESS;
	
	BOOTLOADERINFO *BootLoaderInfo = (BOOTLOADERINFO *)BOOTLOADERINFO_ADDRESS;
	struct SFNEntry *SFNEntry = (struct SFNEntry *)RootDirectoryLocation;
	struct SFNEntry KernelSFNEntry;

	struct VBR VBR;

	ClearScreen(0x07);
	BIOSINT_PrintString("Hello world in Kernel32\r\n");
	// Backup E820 Entry
	memcpy((unsigned char *)0x3FF400 , (unsigned char *)0xA000 , sizeof(struct QuerySystemAddressMap)*128);
	
	// Find kernel file entry from SFN entry
	for(i = 0; i < 32; i++) {
		// BIOSINT_printf("SFNEntry->FileName : %s\r\n" , SFNEntry->FileName);
		if(strncmp(SFNEntry->FileName , Kernel64Name  , 11) == 0) {
			BIOSINT_printf("Found file!\r\n");
			memcpy(&(KernelSFNEntry) , SFNEntry , sizeof(struct SFNEntry));
			break;
		}

		SFNEntry = (struct SFNEntry *)(((unsigned int)SFNEntry)+sizeof(struct SFNEntry));
	}
	if(i == 32) {
		BIOSINT_printf("Kernel not found.\r\n");
		while(1) {
			;
		}
	}
	// Found file
	// objective : 1. Load Kernel to proper location
	// 			   2. Load AP Loader to proper location
	//             3. Copy disk to RAM disk location
	memcpy((unsigned int *)TEMPORARY_SAFE_ADDRESS , (unsigned int *)APLOADER_ADDRESS , BYTES_PER_SECTOR*1);
	
	// Checking whether the area for kernel is cleared
	*((unsigned int *)0x400000) = 0xCAFEBABE;
	BIOSINT_printf("Clearing Kernel Area ... ");
	// Location of Kernel64 stack : 0x400000~0x500000
	for(Address = KERNEL64_ADDRESS; Address < 0x500000; Address += 4) {
		*((unsigned int *)Address) = 0x00;
		if(*((unsigned int *)Address) != 0x00) {
			BIOSINT_printf("Error\r\n");
			while(1) {
				;
			}
		}
	}
	// If not cleared, panic
	if(*((unsigned int *)0x400000) == 0xCAFEBABE) {
		BIOSINT_printf("Error\r\n");
		while(1) {
			;
		}
	}

	BIOSINT_printf("Done\r\n");
	// There's some weird phenomenon that the data still exists after rebooting the machine...
	// So I made a makeshift heap initializer.
	BIOSINT_printf("BootLoaderInfo      : 0x%X\r\n" , BootLoaderInfo);
	BIOSINT_printf("BootLoaderInfo->DAP : 0x%X\r\n" , &(BootLoaderInfo->DAP));
	BIOSINT_printf("DriveNumber         : 0x%X\r\n" , BootLoaderInfo->DriveNumber);
	
	GetVBR(&(VBR));
	BIOSINT_printf("Loading Kernel ... ");
	KernelSectorSize = (KernelSFNEntry.FileSize/BYTES_PER_SECTOR)+(KernelSFNEntry.FileSize%BYTES_PER_SECTOR != 0);
	LoadSectorToMemory(KERNEL64_ADDRESS , ClusterToSector((KernelSFNEntry.StartingClusterHigh << 16)|KernelSFNEntry.StartingClusterLow , &(VBR)) , KernelSectorSize);
	BIOSINT_printf("Done\r\n");
	BIOSINT_printf("Relocating Kernel Loader ... ");
	memcpy((unsigned int *)APLOADER_ADDRESS , (unsigned int *)TEMPORARY_SAFE_ADDRESS , BYTES_PER_SECTOR*1);
	BIOSINT_printf("Done\r\n");
	BIOSINT_printf("Creating PML4 Table Entry ... ");
	CreatePML4Entry(PML4_ADDRESS);
	BIOSINT_printf("Done\r\n");
	JumpToKernel64();
	while(1) {
		;
	}
}

void CreatePML4Entry(unsigned int Address) {
	int i;
	unsigned int EntryAddress;
	unsigned char EntryAddressHigh;
	PML4TENTRY *PML4Entry = (PML4TENTRY *)Address;
	PDPTENTRY *PDPTEntry = (PDPTENTRY *)(Address+(512*8));
	PDENTRY *PDEntry = (PDENTRY *)(Address+(512*8)+(512*8));
	PML4Entry[0].BaseAddressAndFlags = ((unsigned int)PDPTEntry)|PML4TENTRY_P|PML4TENTRY_RW;
	PML4Entry[0].BaseAddressHigh = 0x00;
	PML4Entry[0].Reserved = 0x00;
	PML4Entry[0].EXB = 0x00;
	EntryAddress = (unsigned int)PDEntry;
	for(i = 0; i < 128; i++) {
		PDPTEntry[i].BaseAddressAndFlags = EntryAddress|PDPTENTRY_P|PDPTENTRY_RW;
		PDPTEntry[i].BaseAddressHigh = 0x00;
		PDPTEntry[i].Reserved = 0x00;
		PDPTEntry[i].EXB = 0x00;
		
		EntryAddress += 0x1000;
	}
	EntryAddress = 0x00;
	EntryAddressHigh = 0x00;
	for(i = 0; i < 128*512; i++) {
		PDEntry[i].BaseAddressAndFlags = EntryAddress|PDENTRY_P|PDENTRY_RW|PDENTRY_PS;
		PDEntry[i].BaseAddressHigh = EntryAddressHigh;
		PDEntry[i].Reserved = 0x00;
		PDEntry[i].EXB = 0x00;
		EntryAddress += 0x200000;
		if(EntryAddress == 0x00) {
			EntryAddressHigh += 1;
		}
	}
}

void LoadSectorToMemory(unsigned int MemoryAddress , unsigned int SectorNumber , unsigned int SectorSize) {
	int i;
	const short OneLoadSize = 20;
	const unsigned int LoadAddress = 0x500;
	BOOTLOADERINFO *BootLoaderInfo = (BOOTLOADERINFO *)BOOTLOADERINFO_ADDRESS;
	if(SectorSize < OneLoadSize) {
		BootLoaderInfo->DAP.MemoryAddress = LoadAddress;
		BootLoaderInfo->DAP.SectorStartAddress = SectorNumber;
		BootLoaderInfo->DAP.SectorCountToRead = SectorSize;
		DoBIOSInterrupt(0x13 , 0x4200 , 0x00 , 0x00 , (BootLoaderInfo->DriveNumber & 0xFF) , &(BootLoaderInfo->DAP) , 0x00);
		memcpy((unsigned int *)MemoryAddress , (unsigned int *)LoadAddress , SectorSize*BYTES_PER_SECTOR);
		return;
	}
	for(i = 0; i < (int)(SectorSize/OneLoadSize); i++) {
		BootLoaderInfo->DAP.MemoryAddress = LoadAddress;
		BootLoaderInfo->DAP.SectorStartAddress = SectorNumber;
		BootLoaderInfo->DAP.SectorCountToRead = OneLoadSize;
		DoBIOSInterrupt(0x13 , 0x4200 , 0x00 , 0x00 , (BootLoaderInfo->DriveNumber & 0xFF) , &(BootLoaderInfo->DAP) , 0x00);
		SectorNumber += OneLoadSize;
		memcpy((unsigned int *)MemoryAddress , (unsigned int *)LoadAddress , OneLoadSize*BYTES_PER_SECTOR);
		MemoryAddress += OneLoadSize*BYTES_PER_SECTOR;
	}
	if(SectorSize%OneLoadSize != 0) {
		BootLoaderInfo->DAP.MemoryAddress = LoadAddress;
		BootLoaderInfo->DAP.SectorStartAddress = SectorNumber;
		BootLoaderInfo->DAP.SectorCountToRead = SectorSize%OneLoadSize;
		DoBIOSInterrupt(0x13 , 0x4200 , 0x00 , 0x00 , (BootLoaderInfo->DriveNumber & 0xFF) , &(BootLoaderInfo->DAP) , 0x00);
		memcpy((unsigned int *)MemoryAddress , (unsigned int *)LoadAddress , (SectorSize%OneLoadSize)*BYTES_PER_SECTOR);
	}
	return;
}

void ClearScreen(unsigned char Color) {
	int i;
	unsigned char *VideoMemory = (unsigned char *)0xB8000;
	for(i = 0; i < 80*25; i++) {
		*(VideoMemory++) = 0x00;
		*(VideoMemory++) = Color;
	}
	BIOSINT_MoveCursor(0 , 0);
}