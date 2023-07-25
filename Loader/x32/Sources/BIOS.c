#include <BIOS.h>
#include <Essential.h>
#include <Main.h>

int LocalX;
int LocalY;

void BIOSINT_PrintString(const char *String) {
	int i;
	for(i = 0; String[i] != 0; i++) {
		DoBIOSInterrupt(0x10 , 0x0E00+String[i] , 0x00 , 0x00 , 0x00 , 0x00 , 0x00);
	}
}

void BIOSINT_printf(const char *Format , ...) {
	int i;
    va_list ap;
    char String[128];
    va_start(ap , Format);
    vsprintf(String , Format , ap);
	for(i = 0; String[i] != 0; i++) {
		DoBIOSInterrupt(0x10 , 0x0E00+String[i] , 0x00 , 0x00 , 0x00 , 0x00 , 0x00);
	}
    va_end(ap);
}

void BIOSINT_MoveCursor(int X , int Y) {
    LocalX = X;
    LocalY = Y;
	DoBIOSInterrupt(0x10 , 0x0200 , 0x00 , 0x00 , ((Y & 0xFF) << 8)+(X & 0xFF) , 0x00 , 0x00);
}

void BIOSINT_GetCursor(int *X , int *Y) {
    *X = LocalX;
    *Y = LocalY;
}

void BIOSINT_ReadSector(unsigned int SectorNumber , unsigned int SectorCountToRead , unsigned char *Buffer) {
	const unsigned int LoadAddress = 0x500;
	BOOTLOADERINFO *BootLoaderInfo = (BOOTLOADERINFO *)BOOTLOADERINFO_ADDRESS;
    BootLoaderInfo->DAP.MemoryAddress = (unsigned int)Buffer;
	BootLoaderInfo->DAP.SectorStartAddress = SectorNumber+BootLoaderInfo->PartitionStartAddress;
    BootLoaderInfo->DAP.SectorStartAddressHigh = 0;
	BootLoaderInfo->DAP.SectorCountToRead = SectorCountToRead;
	DoBIOSInterrupt(0x13 , 0x4200 , 0x00 , 0x00 , (BootLoaderInfo->DriveNumber & 0xFF) , &(BootLoaderInfo->DAP) , 0x00);
}