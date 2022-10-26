#ifndef _KERNEL_HPP_
#define _KERNEL_HPP_

#include <KernelSystemStructure.hpp>
#include <DescriptorTables.hpp>
#include <TextScreen.hpp>
#include <MemoryManagement.hpp>
#include <ACPI.hpp>
#include <APIC.hpp>
#include <PIT.hpp>

namespace IO {
    unsigned char Read(unsigned short Port);
    void Write(unsigned short Port , unsigned char Data);
}

#endif