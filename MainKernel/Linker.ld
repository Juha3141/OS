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
  .bss : {
    *(.bss .bss.*)
  }
  .rodata : {
    *(.rodata .rodata.*)
  }
  .rodata1 : {
    *(.rodata1)
  }
}