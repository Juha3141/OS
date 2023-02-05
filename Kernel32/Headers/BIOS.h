#ifndef _BIOS_H_
#define _BIOS_H_

unsigned char DoBIOSInterrupt(unsigned char InterruptNumber , unsigned short AX , unsigned short BX , unsigned short CX , unsigned short DX , unsigned short SI , unsigned short DI);
void BIOSINT_MoveCursor(int X , int Y);
void BIOSINT_GetCursor(int *X , int *Y);
void BIOSINT_PrintString(const char *String);
void BIOSINT_printf(const char *Format , ...);

#endif