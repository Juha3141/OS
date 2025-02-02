# OS
(The development of this project has been closed indefinitely.)
### 1. Goal of this project
The goal of this project is to make a modularized, universal operating system, which can emulate various operating system, without any virtual machine. This project is the experience of figuring whether it is possible to make the universal operating system, thus, two goal that this project should accomplish is:
1. Modularizing (mostly) everything except for kernel core
2. Capability of implementing various operating system's executable files.

The operating system will be capable of implementing various driver and other operating system's library, architecture and system call. This would allow embedded system to easily customize operating system without making completely new operating system.

### 2. Specification & Progress
```
Architecture      : Intel x86_64
Compiled Language : C++, Assembly
Booting Method    : Legacy (Considering developing UEFI kernel)
Multicore Support : O
File System curently develpoing:                    Storage Driver currently developing:
- [X] FAT16                                         - [ ] PCI (Currently in progress)
- [ ] FAT32                                         - [X] IDE
- [ ] ISO9660 (Currently in progress)               - [ ] SATA
- [ ] NTFS                                          - [ ] NVME
- [ ] Ext4                                          - [ ] USB
```

### 3. Compiling & Running
##### Prerequisits : 
1. x86_64-linux-gnu-gcc
2. x86_64-linux-gnu-binutils
3. x86_64-elf-g++
4. x86_64-elf-binutils
5. i686-gnu-linux-binutils
6. make
7. nasm
8. wine(For oscdimg)
9. any virtual machine
 
**Note : You should compile in Linux System. If you want to compile on Windows System, you should install all prerequisits in Linux bash subsystem.**
##### Compile : 
Go to the directory and type :
```
make clean all
```
Use this command if you are on Windows System : 
```
make --makefile WindowsMakefile clean all
```
You should get the target file "OS.iso". Boot it to your virtual machine.

### 4. Future Plan
I am planning to make a document containing detail information of this operating system, especially focused on structure, module, kernel service and executable file. Currently the components and drivers are very scattered across the source of the operating system, but in future the system will be integrated to a sort of module, made of file inside the disk of the operating system. I am hoping that this operating system solves the problem of different platform between various operating system, and even integrates different operating systems in developer's taste.
