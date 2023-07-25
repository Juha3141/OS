#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdarg.h>

#define BOOTLOADER_ADDRESS 0x7C00
#define BOOTLOADERINFO_ADDRESS (BOOTLOADER_ADDRESS+512-(sizeof(BOOTLOADERINFO)))
#define KERNEL64_ADDRESS 0x100000
#define APLOADER_ADDRESS 0x8000
#define TEMPORARY_SAFE_ADDRESS 0xC000
#define PML4_ADDRESS 0x10000
#define BYTES_PER_SECTOR 512
#define TEMPAREA_ADDRESS 0x500

#define PML4_ENTRYCOUNT 512
#define PML4TENTRY_P   0b000001
#define PML4TENTRY_RW  0b000010
#define PML4TENTRY_US  0b000100
#define PML4TENTRY_PWT 0b001000
#define PML4TENTRY_PCD 0b010000
#define PML4TENTRY_A   0b100000

#define PDPTENTRY_P   PML4TENTRY_P
#define PDPTENTRY_RW  PML4TENTRY_RW
#define PDPTENTRY_US  PML4TENTRY_US
#define PDPTENTRY_PWT PML4TENTRY_PWT
#define PDPTENTRY_PCD PML4TENTRY_PCD
#define PDPTENTRY_A   PML4TENTRY_A

#define PDENTRY_P   0b000000001
#define PDENTRY_RW  0b000000010
#define PDENTRY_US  0b000000100
#define PDENTRY_PWT 0b000001000
#define PDENTRY_PCD 0b000010000
#define PDENTRY_A   0b000100000
#define PDENTRY_D   0b001000000
#define PDENTRY_PS  0b010000000
#define PDENTRY_G   0b100000000

typedef struct {
	unsigned char Size;
	unsigned char Reserved;
	unsigned short SectorCountToRead;
	unsigned int MemoryAddress;
	unsigned int SectorStartAddress;
	unsigned int SectorStartAddressHigh;
}DISKADDRESSPACKET;

typedef struct {
	unsigned int RootDirectoryLocation;
	unsigned short RootDirectorySize;
	unsigned int KernelLoaderLocation;
	unsigned int KernelLoaderSectorSize;

	DISKADDRESSPACKET DAP;

	unsigned char DriveNumber;
	unsigned char KernelLoaderName[12];
	unsigned int PartitionStartAddress;
	
	unsigned char Reserved[5];
	unsigned short Signature;
}BOOTLOADERINFO;

typedef struct {
    unsigned int BaseAddressAndFlags;
	unsigned char BaseAddressHigh;
    unsigned int Reserved:23;
    unsigned char EXB:1;
}PML4TENTRY , PDPTENTRY , PDENTRY;

void ClearScreen(unsigned char Color);
void FlyToLongMode(unsigned int PML4Address);

void LoadSectorToMemory(unsigned int MemoryAddress , unsigned int SectorNumber , unsigned int SectorSize);

#endif