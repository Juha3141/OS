#include <Graphics/VESABIOSExtensions.hpp>

unsigned char Consolas_8x16_Bold[4096] = {

};

struct Graphics::VBE::InfoStructure *Graphics::VBE::GetInfoStructure(void) {
    return (struct VBE::InfoStructure *)VBE_INFOSTRUCTURE_ADDRESS;
}

void Graphics::VBE::DrawPixel(int X , int Y , unsigned int Color) {
    struct VBE::InfoStructure *VBEInfoStructure = (struct VBE::InfoStructure *)VBE_INFOSTRUCTURE_ADDRESS;
    unsigned char *VideoMemory = (unsigned char *)VBEInfoStructure->Address;
    if((X < 0)||(Y < 0)) {
        return;
    }
    if((X >= VBEInfoStructure->Width)||(Y >= VBEInfoStructure->Height)) {
        return;
    }
    VideoMemory[((Y*VBEInfoStructure->Width)+X)*3] = Color & 0xFF;
    VideoMemory[(((Y*VBEInfoStructure->Width)+X)*3)+1] = (Color >> 8) & 0xFF;
    VideoMemory[(((Y*VBEInfoStructure->Width)+X)*3)+2] = (Color >> 16) & 0xFF;
}

unsigned int Graphics::VBE::GetPixel(int X , int Y) {
	unsigned int Color;
    struct VBE::InfoStructure *VBEInfoStructure = (struct VBE::InfoStructure *)VBE_INFOSTRUCTURE_ADDRESS;
    unsigned char *VideoMemory = (unsigned char *)VBEInfoStructure->Address;
    Color = VideoMemory[((Y*VBEInfoStructure->Width)+X)*3];
    Color |= VideoMemory[(((Y*VBEInfoStructure->Width)+X)*3)+1] << 8;
    Color |= VideoMemory[(((Y*VBEInfoStructure->Width)+X)*3)+2] << 16;
} 

void Graphics::VBE::DrawRectangle(int X1 , int Y1 , int X2 , int Y2 , unsigned int Color) {
    int X;
    int Y;
    for(Y = Y1; Y < Y2; Y++) {
        for(X = X1; X < X2; X++) {
            DrawPixel(X , Y , Color);
        }
    }
}

void Graphics::VBE::DrawText(int X , int Y , unsigned int Color , const char *Format , ...) {
	int i;
	int j;
	int k;
	va_list ap;
	int CurrentX;
	int CurrentY;
	unsigned char BitMask;
	int BitMaskStartAddress;
	char *String = (char *)Kernel::MemoryManagement::Allocate(512);
    struct VBE::InfoStructure *VBEInfoStructure = (struct VBE::InfoStructure *)VBE_INFOSTRUCTURE_ADDRESS;
    unsigned char *VideoMemory = (unsigned char *)VBEInfoStructure->Address;
	unsigned char *Consolas_8x16_Bold = (unsigned char *)0x9C00;
	const int Width = 8;
	const int Height = 16;
	int HeightBackup;
	va_start(ap , Format);

	vsprintf(String , Format , ap);
	CurrentX = X;
	for(k = 0; k < strlen(String); k++) {
		CurrentY = Y;
		BitMaskStartAddress = String[k]*Height;
		for(j = 0; j < Height; j++) {
			BitMask = Consolas_8x16_Bold[BitMaskStartAddress++];
			for(i = 0; i < Width; i++) {
				if(BitMask & (0x01 << (Width-i-1))) {
					VBE::DrawPixel(CurrentX+i , CurrentY , Color);
				}
			}
			CurrentY += 1;
		}
		CurrentX += Width;
	}
	memset(String , 0 , strlen(String));
	va_end(ap);
}