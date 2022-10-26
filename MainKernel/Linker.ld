OUTPUT_FORMAT(elf64-x86-64)
ENTRY(LongModeEntry)
SECTIONS {
  . = 0x100000;
  .text 0x100000 : {
    *(.text .text.*)
  }
  .data : {
    *(.data .data.*)
  }
  .rodata : {
    *(.rodata .rodata.*)
  }
  .rodata1 : {
    *(.rodata1)
  }
  .bss : {
    *(.bss .bss.*)
  }
}