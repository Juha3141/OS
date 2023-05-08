//////////////////////////////////////////////////////////////////////////////
// File "DescriptorTables.hpp"                                              //
// Written by : Juha Cho                                                    //
// Started Date : 2022.09.04                                                //
// Description : Header file of DescriptorTables.cpp, contains structure of //
// descriptors manager, entries, and registers                              //
//////////////////////////////////////////////////////////////////////////////

#ifndef _DESCRIPTORTABLES_H_
#define _DESCRIPTORTABLES_H_
////////////////////////////
// <List of Segments>     //
// 0. Null Segment        //
// 1. Kernel Code Segment //
// 2. Kernel Data Segment //
// 3. Ring 1 Code Segment //
// 4. Ring 1 Data Segment //
// 5. Ring 2 Code Segment //
// 6. Ring 2 Data Segment //
// 7. User   Code Segment //
// 8. User   Data Segment //
// -> Total Nine Segments //
////////////////////////////

#define GDT_ENTRYCOUNT 9
// Flags and type of GDT, more details are down below.
#define GDT_TYPE_RW                  0b00000010
#define GDT_TYPE_DC                  0b00000100
#define GDT_TYPE_E                   0b00001000

#define GDT_TYPE_16BIT_TSS_AVAILABLE 0x01
#define GDT_TYPE_LDT                 0x02
#define GDT_TYPE_16BIT_TSS_BUSY      0x03
#define GDT_TYPE_32BIT_TSS_AVAILABLE 0x09
#define GDT_TYPE_32BIT_TSS_BUSY      0x0B

#define GDT_FLAGS_S    0b00000001
#define GDT_FLAGS_DPL0 0b00000000
#define GDT_FLAGS_DPL1 0b00000010
#define GDT_FLAGS_DPL2 0b00000100
#define GDT_FLAGS_DPL3 0b00000110
#define GDT_FLAGS_P    0b00001000
#define GDT_FLAGS_L    0b00100000
#define GDT_FLAGS_DB   0b01000000
#define GDT_FLAGS_G    0b10000000

#define IDT_ENTRYCOUNT                255
#define IDT_FLAGS_DPL0                0b0000
#define IDT_FLAGS_DPL1                0b0010
#define IDT_FLAGS_DPL2                0b0100
#define IDT_FLAGS_DPL3                0b0110
#define IDT_FLAGS_P                   0b1000

#define IDT_TYPE_TASK_GATE            0x05
#define IDT_TYPE_16BIT_INTERRUPT_GATE 0x06
#define IDT_TYPE_32BIT_INTERRUPT_GATE 0x0E
#define IDT_TYPE_32BIT_TRAP_GATE      0x0F

#define TSS_ENTRYCOUNT    128
#define IST_STARTADDRESS  0x620000
#define IST_SIZE          0x100000
#define IST_SIZE_PER_CORE 8*1024

#define KERNEL_CS   0x08
#define KERNEL_DS   0x10
#define RING1_CS    (0x18|1)
#define RING1_DS    (0x20|1)
#define RING2_CS    (0x28|2)
#define RING2_DS    (0x30|2)
#define USER_CS     (0x38|3)
#define USER_DS     (0x40|3)
#define TSS_SEGMENT 0x48

namespace Kernel {
    namespace DescriptorTables {
        struct GDTEntry {           // 8 bytes GDT structure
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // <Structure of GDT Entry>                                                                                //
            // +-----------------+----------------+------------------+---------------+-----------+-------------------+ //
            // | BaseHigh[24:31] | FlagsHigh[4:7] | LimitHigh[16:19] | FlagsLow[0:3] | Type[0:3] | BaseMiddle[16:23] | //
            // +-----------------+----------------+------------------+---------------+-----------+-------------------+ //
            // 31              24 23            20 19              16 15           12 11        8 7                  0 //
            // +-------------------------------------------------+---------------------------------------------------+ //
            // |                  BaseLow[0:15]                  |                    LimitLow[0:15]                 | //
            // +-------------------+-----------------------------+---------------------------------------------------+ //
            // 31                                              16 15                                                 0 //
            // Base  : Base address that descriptor indicates, total 4 bytes                                           //
            // Limit : Limit of the segment, total 20bits(Max:0xFFFFF)                                                 // 
            // Flags : Flags of descriptor table (G , DB , L  , DC , P , DPL , S)                                      //
            // Type  : Type of descriptor table (E , DC , RW , A)                                                      //
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                       // for common code , data segments
            unsigned short LimitLow;
            unsigned int BaseLow:24;
            unsigned char Type:4;      // E[3:3] , DC[2:2] , RW[1:1] , A[0:0]
            unsigned char FlagsLow:4;  // P[3:3] , DPL[1:2] , S[0:0]
            unsigned char LimitHigh:4;
            unsigned char FlagsHigh:4; // G[3:3] , DB[2:2] , L[1:1] , 0[0:0]
            unsigned char BaseHigh;
            ////////////////////////////////////////////////////////////////////////////////////////
            // <Type Field Description>                                                   //
            // E   : Executable bit , 1 : Code Segment                                            //
            //                        0 : Data Segment                                            //
            // DC  : Direction/Confirming bit                                                     //
            //       For data segment : 1 = Data segment grows down (high -> low)                 //
            //                          0 = Data segment grows up (low -> high)                   //
            //       For code segment : 1 = Code can be executed by low or same privilage level.  //
            //                          0 = Code can only be executed by same privilage level.    //
            // RW  : R/W bit , 1 = readable, or writable                                          //
            //                 0 = Not readable, or writable                                      //
            // A   : Access bit, needs to be cleared.                                             //
            ////////////////////////////////////////////////////////////////////////////////////////
            // <Flags Field Description>                                                --+       //
            // P   : Present bit , 1 : the segment is present                             |       //
            //                     0 : the segment is not present                         |       //
            // DPL : Descriptor Privilage Level , 0 : Privilage is set to kernel level    | Flags //
            //                                    1 : Privilage is set to Ring 1          |  Low  //
            //                                    2 : Privilage is set to Ring 2          |       //
            //                                    3 : Privilage is set to user level      |       //
            // S   : Segment type ,  1 : the segment is code, or data segment.          --+       //
            //                       0 : the segment is system segment(Ex. TSS)         --+       //
            // L   : Long Mode bit , 1 : the segment is long-mode segment.                |       //
            //                       0 : the segment is not long-mode segment(other.)     |       //
            // D/B : Size bit ,      1 : 32bit protect mode segment                       | Flags //
            //                       0 : 16bit protect mode segment                       | High  //
            //       In long mode segment, D/B bit needs to be cleared.                   |       //
            // G   : Granularity bit , 1 : Limit is multiplied to 4KB.                    |       //
            //                         0 : No multiplication in Limit.                  --+       //
            // Note : Some of texts are paraphrased paragraph in wikipedia.                       //
            ////////////////////////////////////////////////////////////////////////////////////////
        };
        struct IDTEntry {
            // Base : Location of the interrupt handler (ISR/IRQ)
            unsigned short BaseLow;
            unsigned short Selector;    // Code segment(selector) to run the IDT handler
            unsigned char IST;          // IST : 1~7 possible, We are going to use IST#1
            unsigned char Type:4;       // defined in types of macro at Line 
            unsigned char Flags:4;      // P[3:3] DPL[2:1] , S[0:0]
            unsigned long BaseHigh:48;
            unsigned int Reserved;
        };
        struct TSSEntry {           // 16 bytes GDT structure
                                    // for TSS Segment
            unsigned short LimitLow;
            unsigned int BaseLow:24;
            unsigned char Type:4;       // In x86_64 structure, 0x0E is Interrupt Gate, and
                                        //                      0x0F is Trap Gate.
            unsigned char FlagsLow:4;   // P[3:3] , DPL[1:2] , S[0:0]
            unsigned char LimitHigh:4;  
            unsigned char FlagsHigh:4;  // G[3:3] , DB[2:2] , L[1:1]
            unsigned long BaseHigh:40;
            unsigned int Reserved;
        };
        struct TSS {                // Data of TSS (TSS Segment indicates THIS TSS data.)
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
        // <Description of TSS Usage in x86_64 Architecture>                                                     //
        // In x86_64 architecture, hardware task switching is not supported.                                     //
        // So, TSS is not used for hardware task switching, but that does not mean that TSS is useless.          //
        // TSS in long mode is used for :                                                                        //
        // 1. Stores stack address for each privilage level                                                      //
        //    RSP[3] contains the stack address for three different privilage levels                             //
        //    (For example, RSP[0] is for PL 0, RSP[1] is for PL 1, ...)                                         //
        // 2. Stores address of IST(Interrupt Stack Table)                                                       //
        //    IST[7] contains the address of IST                                                                 //
        //    In IDT entry, IST field has space of three bits, so it can choose what IST to use.                 //
        //    Maximum value that 3 bits digit can produce is seven, so IST value can be zero to seven.           //
        // 3. Stores offset of IOPB(IO permission bitmap)                                                        //
        //    Each bit refers to availability of the port.                                                       //
        //    For example, Assume that bit 0x310 is set to 1 -> it means the port 0x310 is granted to access.    //
        //    If the port is granted to access, user program can access the port by IN/OUT instructions.         //
        //    -> Kernel can manage IO port by setting right value to IOPB in TSS.                                //
        // Note : Some of texts are paraphrased paragraph in wikipedia.                                          //
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////
            unsigned int Reserved1;
            unsigned long RSP[3];      // Three of RSP
            unsigned long Reserved2;
            unsigned long IST[7];      // Seven of IST
            unsigned long Reserved3;
            unsigned short Reserved4;
            unsigned short IOPBOffset; // unsigned short, which means, Maximum 65535 ports are available for controlling.
        };
        struct DescriptorTablesRegister {
            unsigned short Size;
            unsigned long Address;
        };
        class GlobalDescriptorTable { // what it should do : 
            public:
                void Initialize(unsigned long BaseAddress , unsigned long RegisterAddress);
                void SetGDTEntry(int Offset , unsigned int BaseAddress , unsigned int Limit , unsigned char Type , unsigned char Flags);
                void SetTSSEntry(int Offset , unsigned long BaseAddress , unsigned int Limit , unsigned char Type , unsigned char Flags);

                struct GDTEntry *GDTEntry;
                struct TSSEntry *TSSEntry;
                struct TSS *TSS;
                DescriptorTablesRegister *GDTR;
            private:
                inline void SetTSS_IST(struct TSS *TSS , int ISTNumber , unsigned long IST) {
                    TSS->IST[ISTNumber] = IST;
                }
                inline void SetTSS_RSP(struct TSS *TSS , int PrivilageLevel , unsigned long RSP) {
                    TSS->RSP[PrivilageLevel] = RSP;
                }
                inline void SetTSS_IOPB(struct TSS *TSS , int PortNumber , unsigned char PortFlag) {
                    switch(PortFlag) {
                        case 1:
                            TSS->IOPBOffset |= (1 << PortNumber);
                            break;
                        case 0:
                            if((TSS->IOPBOffset & (1 << PortNumber)) == (1 << PortNumber)) {
                                TSS->IOPBOffset ^= (1 << PortNumber);
                            }
                            else {
                                TSS->IOPBOffset &= (1 << PortNumber);
                            }
                            break;
                    }
                }
        };
        class InterruptDescriptorTable {
            public:
                void Initialize(unsigned long BaseAddress , unsigned long RegisterAddress);
                void SetIDTEntry(int Offset , unsigned int BaseAddress , unsigned short Selector , unsigned char Type , unsigned char Flags , unsigned char IST);
                
                struct IDTEntry *IDTEntry;
                DescriptorTablesRegister *IDTR;
            private:
        };
        void Initialize(void);
        unsigned long GetInterruptStackTable(unsigned long CoreID);
    }
    struct InterruptStackTable {
        unsigned long GS;
        unsigned long FS;
        unsigned long ES;
        unsigned long DS;
        unsigned long R15;
        unsigned long R14;
        unsigned long R13;
        unsigned long R12;
        unsigned long R11;
        unsigned long R10;

        unsigned long R9;
        unsigned long R8;
        unsigned long RSI;
        unsigned long RDI;
        unsigned long RDX;
        unsigned long RCX;
        unsigned long RBX;
        unsigned long RAX;
        // Now I get it! If interrupt occurrs, system puts those values to stack
        // and after interrupt, by iretq instruction, all pushed instructions are put back to 
        // Original registers! Now I get it!!
        unsigned long RBP;
        unsigned long RIP;
        unsigned long CS;
        unsigned long RFlags;
        unsigned long RSP;
        unsigned long SS;
    };
}

#endif