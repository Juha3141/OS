#include <Paging.h>

void SetPageEntry(struct PageEntry *PageEntry , unsigned int BaseAddressLow , unsigned int BaseAddressHigh , unsigned short Flags) {
    PageEntry->BaseAddressLow = (BaseAddressLow >> 12)|(BaseAddressHigh<<(32-12));
    PageEntry->BaseAddressHigh = BaseAddressHigh >> 12;
    if((Flags & (1 << 13)) == (1 << 13)) {  // Set PAT Flag
        PageEntry->BaseAddressLow |= 0x01;
    }
    if((Flags & (1 << 14)) == (1 << 14)) {  // Set EXB Flag
        PageEntry->BaseAddressHigh |= (1 << 20);
    }
    PageEntry->Flags = Flags & 0xFFF;
}

void SetupPML4_custom(unsigned int start_address , struct MemoryMap *memmap) {
    unsigned int pml4_entry_address = start_address; // total 512 entries
    unsigned int pdpt_entry_address = pml4_entry_address+(PAGE_MAX_ENTRY_COUNT*sizeof(struct PageEntry)); // total 512 entries
    unsigned int pde_entry_address = pdpt_entry_address+(PAGE_MAX_ENTRY_COUNT*sizeof(struct PageEntry));

    unsigned int physicaladdr_low = 0;
    unsigned int physicaladdr_high = 0;
    for(int i = 0; i < PAGE_MAX_ENTRY_COUNT; i++) {
        SetPageEntry(pml4_entry_address+(i*sizeof(struct PageEntry)) , 0 , 0 , 0);
    }
    SetPageEntry(pml4_entry_address , pdpt_entry_address , 0x00 , PAGE_PML4ENTRY_FLAGS_P|PAGE_PML4ENTRY_FLAGS_RW);
    for(int i = 0 , j = 0; i < PAGE_MAX_ENTRY_COUNT; i++) {
        SetPageEntry(pdpt_entry_address+(i*sizeof(struct PageEntry)) , pde_entry_address , 0x00 , PAGE_PDPTENTRY_FLAGS_P|PAGE_PDPTENTRY_FLAGS_RW);
        for(int j = 0; j < PAGE_MAX_ENTRY_COUNT; j++) {
            SetPageEntry(pde_entry_address+(j*sizeof(struct PageEntry)) , physicaladdr_low , physicaladdr_high , PAGE_PDENTRY_FLAGS_P|PAGE_PDENTRY_FLAGS_RW|PAGE_PDENTRY_FLAGS_PS);
            physicaladdr_low += 0x200000;
            physicaladdr_high += (physicaladdr_low == 0);
        }
        pde_entry_address += PAGE_MAX_ENTRY_COUNT*sizeof(struct PageEntry);
    }
}