#include <Paging.hpp>

void Paging::SetEntry(struct Paging::Entry *PageEntry , unsigned long BaseAddress , unsigned short Flags) {
    PageEntry->BaseAddress = (BaseAddress >> 12);
    if((Flags & (1 << 13)) == (1 << 13)) {  // Set PAT Flag
        PageEntry->BaseAddress |= 0x01;
    }
    if((Flags & (1 << 14)) == (1 << 14)) {  // Set EXB Flag
        PageEntry->BaseAddress |= (1 << 52);
    }
    PageEntry->Flags = Flags;
}