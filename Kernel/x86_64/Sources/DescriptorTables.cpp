//////////////////////////////////////////////////////////////////////////////////
// File "DescriptorTables.cpp"                                                  //
// Written by : Juha Cho                                                        //
// Started Date : 2022.09.05                                                    //
// Description : Initializes descriptor tables(GDT, IDT, TSS) and loads them    //
//////////////////////////////////////////////////////////////////////////////////

#include <DescriptorTables.hpp>
#include <KernelSystemStructure.hpp>
#include <EssentialLibrary.hpp>
#include <TextScreen.hpp>
#include <ExceptionHandlers.hpp>
#include <PIT.hpp>
#include <Keyboard.hpp>
#include <Mouse.hpp>

#include <IDE.hpp>

// #include <PATA.hpp>

unsigned long *InterruptStackTableAddress;

void DescriptorTables::Initialize(void) {
    unsigned int ID;
    static DescriptorTables::GlobalDescriptorTable *GlobalDescriptorTable;
    static DescriptorTables::InterruptDescriptorTable *InterruptDescriptorTable;
    if(LocalAPIC::CheckBSP() == false) {
        __asm__ ("lgdt [%0]"::"r"(((unsigned long)GlobalDescriptorTable->GDTR)));
        __asm__ ("lidt [%0]"::"r"(((unsigned long)InterruptDescriptorTable->IDTR)));
        
        __asm__ ("ltr %0"::"r"((unsigned short)((TSS_SEGMENT+sizeof(struct TSSEntry)*LocalAPIC::GetCurrentAPICID()))));
        return;
    }
    // Allocate space for each descriptor respectively
    GlobalDescriptorTable = (DescriptorTables::GlobalDescriptorTable *)SystemStructure::Allocate(
                          sizeof(DescriptorTables::GlobalDescriptorTable));
    InterruptDescriptorTable = (DescriptorTables::InterruptDescriptorTable*)SystemStructure::Allocate(
                             sizeof(DescriptorTables::InterruptDescriptorTable));
#ifdef DEBUG
    printf("GDT Management Structure : 0x%X\n" , GlobalDescriptorTable);
    printf("IDT Management Structure : 0x%X\n" , InterruptDescriptorTable);
#endif
    // Initialize and allocate space for descriptor table entries and registers
    GlobalDescriptorTable->Initialize(SystemStructure::Allocate((GDT_ENTRYCOUNT*sizeof(GDTEntry))+(TSS_ENTRYCOUNT*sizeof(struct TSSEntry)))
                                    , SystemStructure::Allocate(sizeof(DescriptorTablesRegister)));
    InterruptDescriptorTable->Initialize(SystemStructure::Allocate(IDT_ENTRYCOUNT*sizeof(IDTEntry))
                                       , SystemStructure::Allocate(sizeof(DescriptorTablesRegister)));
}

void DescriptorTables::GlobalDescriptorTable::Initialize(unsigned long BaseAddress , unsigned long RegisterAddress) {
    // BaseAddress = Location of GDT Entry
    // BaseAddress+(GDT_ENTRYCOUNT*sizeof(struct GDTEntry))) = Location of TSS Entry
    // (GDT_ENTRYCOUNT*sizeof(struct GDTEntry))) = Size of GDT Entry
    int i;
    unsigned long ISTStackAddress = IST_STARTADDRESS+IST_SIZE;
    GDTEntry = (struct GDTEntry *)BaseAddress;
    TSSEntry = (struct TSSEntry *)(BaseAddress+(GDT_ENTRYCOUNT*sizeof(struct GDTEntry))); // Note : Always put "()" in the address ****
    TSS = (struct TSS *)SystemStructure::Allocate(sizeof(struct TSS)*TSS_ENTRYCOUNT);
    memset(TSS , 0 , sizeof(struct TSS)*TSS_ENTRYCOUNT);
    GDTR = (struct DescriptorTablesRegister *)RegisterAddress;
    // Size of "struct GDTEntry" : 8 , size of "struct TSSEntry" : 16
    GDTR->Size = ((GDT_ENTRYCOUNT*sizeof(struct GDTEntry))+(TSS_ENTRYCOUNT*sizeof(struct TSSEntry))); // Total size of GDT Entry
    GDTR->Address = BaseAddress;
    // Seperated descriptor table
#ifdef DEBUG
    printf("GDTEntry Structure : 0x%X\n" , GDTEntry);
    printf("TSSEntry Structure : 0x%X\n" , TSSEntry);
    printf("TSS Structure      : 0x%X(%d)\n" , TSS , sizeof(struct TSS));
    printf("Size of GDT+TSS    : %d\n" , GDTR->Size);
#endif
    SetGDTEntry(0 , 0x00 , 0x00 , 0x00 , 0x00);  // NullSegment, not using, leave it empty.
    // Code Segment : E=1 , DC=0 , RW=1 , P=1 , S=1 , L=1 , G=1
    // Data Segment : E=0 , DC=0 , RW=1 , P=1 , S=1 , L=1 , G=1
    /* Since Code & Data segments' base addresses, and limits are ignored in x86_64 architecture, and it needs to map entire memory,
     * The limit of each code, and data segments should be 0xFFFFF(Maximum), and Granularity bit must be set to 1.
     * -> Because of the large memory of x86_64 architecture(Maximum 16EB), the architecture was constructed to ignore the limits and
     * base of the segments. Check out more detailed information in:
     * Intel64 and IA-32 Architectures Software Developers Manual - Volume 3A - 3-12 - "Segment Loading Instruction in IA-32e Mode"
     */
    // Kernel Code Segment : E=1 , RW=1 , P=1 , DPL=0 , S=1 , L=1 , G=1
    SetGDTEntry(1 , 0x00 , 0xFFFFF , GDT_TYPE_E|GDT_TYPE_RW , GDT_FLAGS_P|GDT_FLAGS_DPL0|GDT_FLAGS_S|GDT_FLAGS_L|GDT_FLAGS_G); 
    // Kernel Data Segment : E=0 , DC=0 , RW=1 , P=1 , DPL=0 , S=1 , L=1 , G=1
    SetGDTEntry(2 , 0x00 , 0xFFFFF , GDT_TYPE_RW , GDT_FLAGS_P|GDT_FLAGS_DPL0|GDT_FLAGS_S|GDT_FLAGS_L|GDT_FLAGS_G);
    // Ring 1 Code/Data SegmentsIST는 16바이트 단위로 정렬해야 함
    SetGDTEntry(3 , 0x00 , 0xFFFFF , GDT_TYPE_E|GDT_TYPE_RW , GDT_FLAGS_P|GDT_FLAGS_DPL1|GDT_FLAGS_S|GDT_FLAGS_L|GDT_FLAGS_G);
    SetGDTEntry(4 , 0x00 , 0xFFFFF , GDT_TYPE_RW , GDT_FLAGS_P|GDT_FLAGS_DPL1|GDT_FLAGS_S|GDT_FLAGS_L|GDT_FLAGS_G);
    // Ring 2 Code/Data Segments
    SetGDTEntry(5 , 0x00 , 0xFFFFF , GDT_TYPE_E|GDT_TYPE_RW , GDT_FLAGS_P|GDT_FLAGS_DPL2|GDT_FLAGS_S|GDT_FLAGS_L|GDT_FLAGS_G);
    SetGDTEntry(6 , 0x00 , 0xFFFFF , GDT_TYPE_RW , GDT_FLAGS_P|GDT_FLAGS_DPL2|GDT_FLAGS_S|GDT_FLAGS_L|GDT_FLAGS_G);
    // User Code/Data Segments
    SetGDTEntry(7 , 0x00 , 0xFFFFF , GDT_TYPE_E|GDT_TYPE_RW , GDT_FLAGS_P|GDT_FLAGS_DPL3|GDT_FLAGS_S|GDT_FLAGS_L|GDT_FLAGS_G);
    SetGDTEntry(8 , 0x00 , 0xFFFFF , GDT_TYPE_RW , GDT_FLAGS_P|GDT_FLAGS_DPL3|GDT_FLAGS_S|GDT_FLAGS_L|GDT_FLAGS_G);
    // TSS Segment : Type : 0x09(32bit TSS available) , P=1 , S=0(Becasue it is system segment) , DPL=0 , L=1 , G=1
    // TSS segment's base address is the location of TSS(it must be specified, unlike code, data segments), and its limit is the size of TSS.
    // L bit should be cleared, and G should be set to 1 ******
    InterruptStackTableAddress = (unsigned long *)SystemStructure::Allocate(TSS_ENTRYCOUNT*sizeof(unsigned long));
    for(i = 0; i < TSS_ENTRYCOUNT; i++) {
        SetTSSEntry(i , (unsigned long)&(TSS[i]) , sizeof(struct TSS)-1 ,  GDT_TYPE_32BIT_TSS_AVAILABLE , GDT_FLAGS_P|GDT_FLAGS_DPL0|GDT_FLAGS_G);
        // Allocate stack for interrupt handler (IST), each core gets 8KB
        InterruptStackTableAddress[i] = (((unsigned long)MemoryManagement::Allocate(IST_SIZE_PER_CORE , MemoryManagement::ALIGN_4K))+IST_SIZE_PER_CORE);
        SetTSS_IST(&(TSS[i]) , 0 , InterruptStackTableAddress[i]); // IST Address : 0x620000 Size of IST : 0x100000
        TSS[i].IOPBOffset = 0xFFFF;              // Allow every port possible to access for user level
    }
    __asm__ ("lgdt [%0]"::"r"((RegisterAddress)));
    // Don't forget to set Task Segment to TSS Segment "BEFORE" IDT initialization!
    __asm__ ("ltr %0"::"r"((unsigned short)(TSS_SEGMENT)));
}

void DescriptorTables::GlobalDescriptorTable::SetGDTEntry(int Offset , unsigned int BaseAddress , unsigned int Limit , unsigned char Type , unsigned char Flags) {
    // Logical arithmetic for each values
    // Limit : Total 20 bits
    // Base  : Total 32 bits
    // Type  : Total 4 bits
    // Flags : Total 8 bits
    this->GDTEntry[Offset].LimitLow = Limit & 0xFFFF;
    this->GDTEntry[Offset].BaseLow = BaseAddress & 0xFFFFFF;
    this->GDTEntry[Offset].Type = Type & 0x0F;
    this->GDTEntry[Offset].FlagsLow = Flags & 0x0F;
    this->GDTEntry[Offset].FlagsHigh = (Flags >> 4) & 0x0F;
    this->GDTEntry[Offset].LimitHigh = (Limit >> 16) & 0x0f;
    this->GDTEntry[Offset].BaseHigh = (BaseAddress >> 24) & 0xFF;
}

void DescriptorTables::GlobalDescriptorTable::SetTSSEntry(int Offset , unsigned long BaseAddress , unsigned int Limit , unsigned char Type , unsigned char Flags) {
    // Limit : Total 20 bits
    // Base  : Total 64 bits (TSS entry needs 64 bits of base address)
    // Type  : Total 4 bits
    // Flags : Total 8 bits
    this->TSSEntry[Offset].LimitLow = Limit & 0xFFFF;
    this->TSSEntry[Offset].BaseLow = BaseAddress & 0xFFFFFF;
    this->TSSEntry[Offset].Type = Type & 0x0F;
    this->TSSEntry[Offset].FlagsLow = Flags & 0x0F;
    this->TSSEntry[Offset].FlagsHigh = (Flags >> 4) & 0x0F;
    this->TSSEntry[Offset].LimitHigh = (Limit >> 16) & 0x0f;
    this->TSSEntry[Offset].BaseHigh = BaseAddress >> 24;
}

void DescriptorTables::InterruptDescriptorTable::Initialize(unsigned long BaseAddress , unsigned long RegisterAddress) {
    int i;
    IDTEntry = (struct IDTEntry *)BaseAddress;
    IDTR = (DescriptorTablesRegister *)RegisterAddress;
    IDTR->Size = IDT_ENTRYCOUNT*sizeof(struct IDTEntry);
    IDTR->Address = BaseAddress;
#ifdef DEBUG
    printf("IDTEntry Structure : 0x%X\n" , IDTEntry);
    printf("IDTR Structure     : 0x%X\n" , IDTR);
#endif
    for(i = 0; i < IDT_ENTRYCOUNT; i++) {
        SetIDTEntry(i , 0 , 0 , 0 , 0 , 0);
    }
    // Note : If the IDT entry that uses IST keep occurs error, consider check if you set 
    // task register to TSS Segment. Check out line 66.
    SetIDTEntry(0 , (unsigned long)Exceptions::DividedByZero , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(1 , (unsigned long)Exceptions::Debug , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(2 , (unsigned long)Exceptions::NonMaskableInterrupt , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(3 , (unsigned long)Exceptions::Breakpoint , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(4 , (unsigned long)Exceptions::Overflow , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(5 , (unsigned long)Exceptions::BoundRangeExceeded , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(6 , (unsigned long)Exceptions::InvalidOpcode , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(7 , (unsigned long)Exceptions::DeviceNotAvailable , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(8 , (unsigned long)Exceptions::DoubleFault , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(9 , (unsigned long)Exceptions::CorprocessorSegmentOverrun , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(10 , (unsigned long)Exceptions::InvalidTSS , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(11 , (unsigned long)Exceptions::SegmentNotPresent , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(12 , (unsigned long)Exceptions::StackSegmentFault , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(13 , (unsigned long)Exceptions::GeneralProtectionFault , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(14 , (unsigned long)Exceptions::PageFault , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(15 , (unsigned long)Exceptions::Reserved15 , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(16 , (unsigned long)Exceptions::x87FloatPointException , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(17 , (unsigned long)Exceptions::AlignmentCheck , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(18 , (unsigned long)Exceptions::MachineCheck , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(19 , (unsigned long)Exceptions::SIMDFloatingPointException , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(20 , (unsigned long)Exceptions::VirtualizationException , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(21 , (unsigned long)Exceptions::ControlProtectionException , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(22 , (unsigned long)Exceptions::Reserved22 , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(23 , (unsigned long)Exceptions::Reserved23 , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(24 , (unsigned long)Exceptions::Reserved24 , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(25 , (unsigned long)Exceptions::Reserved25 , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(26 , (unsigned long)Exceptions::Reserved26 , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(27 , (unsigned long)Exceptions::Reserved27 , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(28 , (unsigned long)Exceptions::HypervisorInjectionException , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(29 , (unsigned long)Exceptions::VMMCommunicationException , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(30 , (unsigned long)Exceptions::SecurityException , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    
    SetIDTEntry(32 , (unsigned long)PIT::InterruptHandler , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(33 , (unsigned long)Keyboard::InterruptHandler , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(44 , (unsigned long)Mouse::InterruptHandler , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(41 , (unsigned long)LocalAPIC::Timer::InterruptHandler , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    
    SetIDTEntry(46 , (unsigned long)IDE::InterruptHandler_IRQ14, 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    SetIDTEntry(47 , (unsigned long)IDE::InterruptHandler_IRQ15 , 0x08 , IDT_TYPE_32BIT_INTERRUPT_GATE , IDT_FLAGS_P|IDT_FLAGS_DPL0 , 0x01);
    
    __asm__ ("lidt [%0]"::"r"((RegisterAddress)));
}

void DescriptorTables::InterruptDescriptorTable::SetIDTEntry(int Offset , unsigned int BaseAddress , unsigned short Selector , unsigned char Type , unsigned char Flags , unsigned char IST) {
    // Base     : Total 64 bits
    // (There is no limit field in IDT entry, because the base address is a location of handler, not a data)
    // Selector : Total 16 bits
    // IST      : 6 reserved bits, and 2 IST bit
    // Type     : Total 4 bits
    // Flags    : Total 4 bits
    this->IDTEntry[Offset].BaseLow = BaseAddress & 0xFFFF;
    this->IDTEntry[Offset].Selector = Selector;
    this->IDTEntry[Offset].IST = IST & 0b11;
    this->IDTEntry[Offset].Type = Type & 0x0F;
    this->IDTEntry[Offset].Flags = Flags & 0x0F;
    this->IDTEntry[Offset].BaseHigh = BaseAddress >> 16;
    this->IDTEntry[Offset].Reserved = 0x00;
}

unsigned long DescriptorTables::GetInterruptStackTable(unsigned long CoreID) {
    return InterruptStackTableAddress[CoreID];
}