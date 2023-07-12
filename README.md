# OS
### 1. Goal of this project
The goal of this project is to make a modularized, universal operating system, which can emulate various operating system, without any virtual machine. This project is the experience of figuring whether it is possible to make the universal operating system, thus, two goal that this project should accomplish is:
1. Modularizing (mostly) everything except for kernel core
2. Capability of implementing various operating system's executable files.

The operating system will be capable of implementing various driver and other operating system's library, architecture and system call. This would allow embedded system to easily customize operating system without making completely new operating system.

### 2. Specification
```
Architecture      : Intel x86_64
Compiled Language : C++, Assembly
Booting Method    : Legacy (Considering developing UEFI kernel)
Multicore Support : O
File System curently develpoing:                    Device Driver currently developing:
- [X] FAT16                                         - [ ] PCI (Currently in progress)
- [ ] FAT32                                         - [X] IDE
- [ ] ISO9660 (Currently in progress)               - [ ] SATA
- [ ] NTFS                                          - [ ] NVME
- [ ] Ext4                                          - [ ] USB
```

### 3. How to compile
```Prerequisits : 
1. g++      2. 

```
