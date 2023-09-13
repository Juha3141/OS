#ifndef _PAGING_H_
#define _PAGING_H_

#define PAGE_MAX_ENTRY_COUNT     512
#define PAGE_PML4ENTRY_FLAGS_P   0b000000000001
#define PAGE_PML4ENTRY_FLAGS_RW  0b000000000010
#define PAGE_PML4ENTRY_FLAGS_US  0b000000000100
#define PAGE_PML4ENTRY_FLAGS_PWT 0b000000001000
#define PAGE_PML4ENTRY_FLAGS_PCD 0b000000010000
#define PAGE_PML4ENTRY_FLAGS_A   0b000000100000
#define PAGE_PML4ENTRY_FLAGS_EXB 0b100000000000

#define PAGE_PDPTENTRY_FLAGS_P   0b00000000000001
#define PAGE_PDPTENTRY_FLAGS_RW  0b00000000000010
#define PAGE_PDPTENTRY_FLAGS_US  0b00000000000100
#define PAGE_PDPTENTRY_FLAGS_PWT 0b00000000001000
#define PAGE_PDPTENTRY_FLAGS_PCD 0b00000000010000
#define PAGE_PDPTENTRY_FLAGS_A   0b00000000100000
#define PAGE_PDPTENTRY_FLAGS_D   0b00000001000000 // Ignored when PDPTE references Page Directory
#define PAGE_PDPTENTRY_FLAGS_PS  0b00000010000000 // Page size, 1 = 1GB page, 0 = 2MB or 4KB page.
#define PAGE_PDPTENTRY_FLAGS_G   0b00000100000000 // 
#define PAGE_PDPTENTRY_FLAGS_PAT 0b01000000000000
#define PAGE_PDPTENTRY_FLAGS_EXB 0b10000000000000

#define PAGE_PDENTRY_FLAGS_P     0b00000000000001
#define PAGE_PDENTRY_FLAGS_RW    0b00000000000010
#define PAGE_PDENTRY_FLAGS_US    0b00000000000100
#define PAGE_PDENTRY_FLAGS_PWT   0b00000000001000
#define PAGE_PDENTRY_FLAGS_PCD   0b00000000010000
#define PAGE_PDENTRY_FLAGS_A     0b00000000100000
#define PAGE_PDENTRY_FLAGS_D     0b00000001000000
#define PAGE_PDENTRY_FLAGS_PS    0b00000010000000
#define PAGE_PDENTRY_FLAGS_G     0b00000100000000
#define PAGE_PDENTRY_FLAGS_EXB   0b10000000000000

typedef struct PageEntry {
    unsigned short Flags:12;
    unsigned int BaseAddressLow:32;  // Total 52bit, Note that base address should be 
    unsigned int BaseAddressHigh:20; // aligned to 4KB
}
PageEntry , 
PageDirectoryPointerEntry , 
PageDirectoryEntry , 
PageTableEntry;

struct MemoryMap {
	// unsigned int Size;
	unsigned int addr_low , addr_high;
	unsigned int length_low , length_high;
	unsigned int type;
};

void SetPageEntry(struct PageEntry *PageEntry , unsigned int BaseAddressLow , unsigned int BaseAddressHigh , unsigned short Flags);
void SetupPML4_custom(unsigned int start_address , struct MemoryMap *memmap);

#endif