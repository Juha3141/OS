#ifndef _VESA_LIBRARY_HPP_
#define _VESA_LIBRARY_HPP_

#include <Kernel.hpp>

#define VBE_INFOSTRUCTURE_ADDRESS 0x8C09

namespace Graphics {
    namespace VBE {        
        struct InfoStructure {
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
        struct VBE::InfoStructure *GetInfoStructure(void);
        
        void DrawPixel(int X , int Y , unsigned int Color);
    };
};

#endif