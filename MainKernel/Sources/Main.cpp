#include <EssentialLibrary.hpp>
#include <Kernel.hpp>

#include <Queue.hpp>

struct VBEInfoStructure {
	unsigned short Attributes;
	unsigned char WinA;
	unsigned char WinB;
	unsigned short Granularity;
	unsigned short WinSize;
	unsigned short SegmentA;
	unsigned short SegmentB;
	unsigned int ForRealMode1;
	unsigned short BytesPerScanLine;
	unsigned short Width;
	unsigned short Height;	
	unsigned char WidthCharSize;
	unsigned char HeightCharSize;
	unsigned char NumberOfPlane;
	unsigned char BitsPerPixel;
	unsigned char NumberOfBanks;
	unsigned char MemoryModel;
	unsigned char BankSize;
	unsigned char NumberOfImagedPages;
	unsigned char Reserved1;

	unsigned char RedMaskSize;
	unsigned char RedFieldPosition;
	unsigned char GreenMaskSize;
	unsigned char GreenFieldPosition;
	unsigned char BlueMaskSize;
	unsigned char BlueFieldPosition;

	unsigned char ReservedMaskSize;
	unsigned char ReservedFieldPosition;
	unsigned char DirectColorModeInfo;

	unsigned int Address;
	unsigned int Reserved2;
	unsigned int Reserved3;

	unsigned short LinearBytesPerScanLine;
	unsigned char BankNumberOfImagePages;
    unsigned char LinearNumberOfImagePages;
    unsigned char LinearRedMaskSize;
    unsigned char LinearRedFieldPosition;
    unsigned char LinearGreenMaskSize;
    unsigned char LinearGreenFieldPosition;
    unsigned char LinearBlueMaskSize;
    unsigned char LinearBlueFieldPosition;
    unsigned char LinearReservedMaskSize;
    unsigned char LinearReservedFieldPosition;
    unsigned int MaxPixelClock;

    unsigned char Reserved4[189];
};

void DrawPixel(int X , int Y , unsigned int Color) {
    struct VBEInfoStructure *VBEInfoStructure = (struct VBEInfoStructure *)0x8C09;
    unsigned char *VideoMemory = (unsigned char *)VBEInfoStructure->Address;
    VideoMemory[((Y*VBEInfoStructure->Width)+X)*3] = Color & 0xFF;
    VideoMemory[(((Y*VBEInfoStructure->Width)+X)*3)+1] = (Color >> 8) & 0xFF;
    VideoMemory[(((Y*VBEInfoStructure->Width)+X)*3)+2] = (Color >> 16) & 0xFF;
}

extern "C" void Main(void) {
    /*
    int i;
    long double X = -7;
    long double Y = -7;
    long double Value;
    int ScreenX = 0;
    int ScreenY = 0; // (((x)^2+(y)^2-36))^3+(-1)(x)^2(y)^3=0
    unsigned char *VideoMemory = (unsigned char *)0xA0000;

    for(Y = 7; Y > -7; Y -= 0.1) {
        for(X = -7; X < 7; X += 0.1) {
            Value = (((double)(X*X)+(double)(Y*Y))-36);
            Value = ((double)(Value*Value*Value)-((double)(X*X*Y*Y*Y)));
            if((Value <= 40) && (Value >= -40)) {
                VideoMemory[ScreenY*320+ScreenX] = 0x0F;
            }
            ScreenX += 1;
        }
        ScreenX = 0;
        ScreenY += 1;
    }
    while(1) {
        ;
    }*/
    __asm__ ("cli");
    Kernel::SystemStructure::Initialize();
    Kernel::TextScreen80x25::Initialize();
    Kernel::ClearScreen(0x00 , 0x07);
    Kernel::DescriptorTables::Initialize();
    Kernel::MemoryManagement::Initialize();
    Kernel::PIT::Initialize();

    int Color = 0;
    int Add = 1;
    int Delta = 8;
    int X;
    int Y;
    struct VBEInfoStructure *VBEInfoStructure = (struct VBEInfoStructure *)0x8C09;
    unsigned short *VideoMemory = (unsigned short *)VBEInfoStructure->Address;
    while(1) {
        for(Y = 0; Y < VBEInfoStructure->Height; Y++) {
            for(X = 0; X < VBEInfoStructure->Width; X++) {
                DrawPixel(X , Y , (Color & 0xFF));
            }
        }
        Color += Delta*Add;
        if(Color >= 0xFF) {
            Color = 0xFF;
            Add = -1;
        }
        if(Color <= 0) {
            Color = 0;
            Add = 1;
        }
        Kernel::PIT::DelayMilliseconds(10);
    }
    /*
    if(Kernel::ACPI::SaveCoresInformation() == 0) {
        Kernel::printf("Using MP Configurating Table\n");
    }
    
    Kernel::LocalAPIC::EnableLocalAPIC();
    Kernel::LocalAPIC::ActiveAPCores();   
    */
   
    __asm__ ("sti");
    while(1) {
        ;
    }
}

extern "C" void APStartup(void) {
    /* To do : Seperate AP & BSP , 
     * Create TSS for AP
     * Create Memory Management System
     * Create Synchronization System(Spinlock)
     * Create Task Management System
     * Create IO Redirecting table
     */
    
    
    while(1) {
        
    }
}