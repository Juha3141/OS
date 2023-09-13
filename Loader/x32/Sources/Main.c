#include <multiboot.h>
#include <strings.h>

#include <Paging.h>

typedef struct m_mmap_x32 {
	unsigned int size;
	unsigned int addr_low , addr_high;
	unsigned int length_low , length_high;
	unsigned int type;
}multiboot_memory_map_x32_t;

void DetectMemory(struct MemoryMap *mmap , struct multiboot_info *MultibootInfo) {
	unsigned int start = MultibootInfo->mmap_addr;
	unsigned int end = MultibootInfo->mmap_addr+MultibootInfo->mmap_length;
	multiboot_memory_map_x32_t *entry = (multiboot_memory_map_x32_t *)start;

	int i = 0;
	while(entry < end) {
		PrintString(0x07 , "h:%X,l:%X,h:%X,l:%X,T:%d\n" , entry->addr_high , entry->addr_low , entry->length_high , entry->length_low , entry->type);
		entry = (multiboot_memory_map_x32_t *)(((unsigned int)entry)+(entry->size+sizeof(entry->size)));
		memcpy(&(mmap[i++]) , entry+sizeof(entry->size) , sizeof(struct MemoryMap));
	}
}

unsigned int Align(unsigned int address , unsigned int alignment) {
	return (((unsigned int)(address/alignment))+1)*alignment;
}

char kernel_command[] = "This is a kernel by the way";


void JumpToKernel64(unsigned int address);

void Main(struct multiboot_info *multiboot_info) {
	multiboot_module_t *kernel_module;
	if((multiboot_info->mods_count == 0)) {
		PrintString(0x04 , "No Modules found!\n");
		while(1) {
			;
		}
	}
	PrintString(0x07 , "Modules count   : %d\n" , multiboot_info->mods_count);
	// Search module, compare the command line
	char found = 0;
	for(int i = 0; i < multiboot_info->mods_count; i++) {
		kernel_module = (multiboot_module_t *)(multiboot_info->mods_addr+(i*sizeof(multiboot_module_t)));
		PrintString(0x07 , "Module %d : \"%s\"\n" , i , kernel_module->cmdline);
		if(strcmp(kernel_command , kernel_module->cmdline) == 0) {
			found = 1;
			break;
		}
	}
	if(found == 0) {
		PrintString(0x04 , "Kernel not found!\n");
	}
	// https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html
	// https://wiki.osdev.org/Creating_a_64-bit_kernel_using_a_separate_loader
	
	// Relocating kernel to higher half : to 0x800000000000 // physical memory : just module location
	// Relocate Kernel in 4MB Area??
	PrintString(0x07 , "Kernel Image : 0x%X~0x%X\n" , kernel_module->mod_start , kernel_module->mod_end);
	unsigned int kernel_struct_addr = Align(kernel_module->mod_end , 4096);
	
	// Global memory map
	struct MemoryMap *memmap = (struct MemoryMap *)kernel_struct_addr;
	// write the signature to the kernel struct area
	(*((unsigned int *)(kernel_struct_addr += 4))) = 0xDEAFBABE;
	// initialize memory map area
	memset(memmap , 0 , sizeof(struct MemoryMap)*32);
	kernel_struct_addr += sizeof(struct MemoryMap)*32; // maximum 32

	kernel_struct_addr = Align(kernel_struct_addr , 4096);
	
	PrintString(0x07 , "memmap location : 0x%X\n" , memmap);
	DetectMemory(memmap , multiboot_info);

	PrintString(0x07 , "PML4 table location : 0x%X\n" , kernel_struct_addr);
	SetupPML4_custom(kernel_struct_addr , memmap);

	JumpToKernel64(kernel_struct_addr);
	while(1) {
		;
	}
}

// kinda changed the way I name the variables...

void PrintString(unsigned char color , const char *fmt , ...) {
	static int off=0;
	unsigned char *vmem = (unsigned char  *)0xB8000;
	va_list ap;
	char string[512] = {0 , };
	va_start(ap , fmt);
	vsprintf(string , fmt , ap);
	va_end(ap);
	for(int i = 0; string[i] != 0; i++) {
		switch(string[i]) {
			case '\n':
				off = ((off/80)+(off%80 != 0))*80;
				break;
			default:
				*(vmem+(off*2)) = string[i];
				*(vmem+(off*2)+1) = color;
				off++;
				break;
		}
	}
}