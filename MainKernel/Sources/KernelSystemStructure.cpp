////////////////////////////////////////////////////////////////////////////////////////
// File "KernelSystemStructure.cpp"                                                   //
// Written by : Juha Cho                                                              //
// Started Date : 2022.09.04                                                          //
// Description : Manages Kernel System Structure - such like descriptor tables        //
// Allocates space for structures, gives the location of needing structure for Kernel //
////////////////////////////////////////////////////////////////////////////////////////

#include <KernelSystemStructure.hpp>
#include <EssentialLibrary.hpp>

// Function Kernel::SystemStructure::Initialize
// Arguments   : None
// Description : Initializes system structure manager
void Kernel::SystemStructure::Initialize(void) {
    // Location of system structure manager       : 0x500000
    // Size of system structure manager structure : 8 bytes
    // Start of the memory pool                   : 0x500008 
    Kernel::SystemStructure::Manager *StructureManager = (Kernel::SystemStructure::Manager*)KERNELSYSTEMSTRUCTURE_LOCATION;
    StructureManager->Initialize(); // Set CurrentAddress to 0x500008
}

//Function Kernel::SystemStructure::Allocate
// Argument : Size , *ID
// Decription : Allots empty area for system structure, for example (as I said before) GDT or IDT.
// Saves the information of system structure before the area.
// Memory Map : 
// 0           8                 Size+8 -> (This is Relative Address) 
// +-----------+----------------------+
// | Info. of  |                      |
// |  System   |   System Structure   |
// | Structure |                      |
// +-----------+----------------------+
unsigned long Kernel::SystemStructure::Allocate(unsigned int Size , unsigned int *ID) {
    ///////////////////////////////////////////////////////////
    // System Structure Memory Area Management Method        //
    //                                                       //
    // <Memory map of Information area of system structure>  //
    // 0               31               63 -> Total 8 bytes  //
    // +----------------+----------------+                   //
    // |      Size      |       ID       | -> Located before // 
    // +----------------+----------------+    memory area    //
    // Size : Size of the system structure                   //
    // ID   : Location of the system structure(Indicated by  //
    //        Segment:Offset type address)                   //
    // <Memory map of ID area in Info Area>                  //
    // 0                7               15                   //
    // +----------------+----------------+                   //
    // |      Segment   |     Offset     |                   //
    // +----------------+----------------+                   //
    // Real address is calculated by multiplying Segment to  //
    // 256, and then adding Offset to the multiplied value.  //
    // -> Very similar to 16bit segmentation in              //
    //    intel processor.                                   //
    // * The Equation : RealAddress = (Segment*256)+Offset * // 
    ///////////////////////////////////////////////////////////
    unsigned long StructureAddress; // Return Address (Address to be used by other system structures)
    unsigned int *SizePointer;      // Pointer type of size
    unsigned short *Segment;        // Pointer type of segment
    unsigned short *Offset;         // Pointer type of offset
    Kernel::SystemStructure::Manager *StructureManager = (Kernel::SystemStructure::Manager*)KERNELSYSTEMSTRUCTURE_LOCATION;
    // Manager structure is always saved in 0x500000 (which is - as you know - KERNELSYSTEMSTRUCTURE_LOCATION)
    StructureAddress = StructureManager->GetCurrentAddress();            // Get current address to allot
    StructureManager->IncreaseCurrentAddress(Size+8);                    // Increase CurrentAddress for next structure
    SizePointer = (unsigned int *)StructureAddress;                      // Address of Size    : Equals to StructureAddress
    Segment = (unsigned short *)(StructureAddress+sizeof(unsigned int)); // Address of Segment : Equals to StructureAddress + size of Size area(= sizeof(unsigned int))
    Offset = (unsigned short *)(StructureAddress+sizeof(unsigned int)+sizeof(unsigned short));
                                                                         // Address of Segment : Equals to StructureAddress + size of Size area(= sizeof(unsigned int))
                                                                         //                                                 + size of Segment area(= sizeof(unsigned short))
    StructureAddress += 8; // sizeof(unsigned int)+sizeof(unsigned short)+sizeof(unsigned short) = 4+2+2 = 8
    *(Segment) = StructureAddress/256; // Real addresss is calculated by multiplying segment to 256, and adding offset
                                       // So, by dividing StructureAddress to 256, you can get value of the Segment.
    *(Offset) = StructureAddress%256;  // And also, the left value of division is Offset.
    *(SizePointer) = Size;             // Save the size of the area
    *(ID) = ((*(Segment) << 16))|(*Offset); // Check <Memory map of Id area in Info Area> if you need more details.
    memset((unsigned char *)(StructureAddress) , 0 , Size);              // Initialize Structure Area
    return StructureAddress;           // Return available address(except for the 8 bytes information area)
}