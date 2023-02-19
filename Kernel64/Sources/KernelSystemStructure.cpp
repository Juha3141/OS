////////////////////////////////////////////////////////////////////////////////////////
// File "KernelSystemStructure.cpp"                                                   //
// Written by : Juha Cho                                                              //
// Started Date : 2022.09.04                                                          //
// Fixed major bug : 2022.11.08                                                       //
// Description : Manages Kernel System Structure - such like descriptor tables        //
// Allocates space for structures, gives the location of needing structure for Kernel //
////////////////////////////////////////////////////////////////////////////////////////

#include <KernelSystemStructure.hpp>
#include <EssentialLibrary.hpp>

static Kernel::SystemStructure::Manager *StructureManager = (Kernel::SystemStructure::Manager*)KERNELSYSTEMSTRUCTURE_LOCATION;

void Kernel::SystemStructure::Initialize(void) {
    unsigned long Address;
    unsigned char *Error = (unsigned char *)0xB8000;
    // Location of system structure manager       : 0x500000
    // Size of system structure manager structure : 8 bytes
    // Start of the memory pool                   : 0x500008 
    for(Address = KERNELSYSTEMSTRUCTURE_LOCATION; Address < KERNELSYSTEMSTRUCTURE_LOCATION+KERNELSYSTEMSTRUCTURE_SIZE; Address += 8) {
        *((unsigned long *)Address) = 0x00;
        if(*((unsigned long *)Address) != 0x00) {
            Error[0] = 'M';
            Error[2] = 'E';
            Error[4] = 'R';
            while(1) {
                ;
            }
        }
    }
    StructureManager->CurrentAddress = (KERNELSYSTEMSTRUCTURE_LOCATION+sizeof(Kernel::SystemStructure::Manager));
}

// Description : Allot the memory and head next
// Pretty simple huh...
unsigned long Kernel::SystemStructure::Allocate(unsigned int Size) {
    unsigned long Address = StructureManager->CurrentAddress;
    StructureManager->CurrentAddress += Size;
    return Address;           // Return available address(except for the 8 bytes information area)
}