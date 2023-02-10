#ifndef _EXCEPTIONHANDLERS_HPP_
#define _EXCEPTIONHANDLERS_HPP_

#include <DescriptorTables.hpp>
#include <EssentialLibrary.hpp>
#include <TextScreen.hpp>

namespace Kernel {
    namespace Exceptions {
        extern "C" void DividedByZero(void);
        extern "C" void Debug(void);
        extern "C" void NonMaskableInterrupt(void);
        extern "C" void Breakpoint(void);
        extern "C" void Overflow(void);
        extern "C" void BoundRangeExceeded(void);
        extern "C" void InvalidOpcode(void);
        extern "C" void DeviceNotAvailable(void);
        extern "C" void DoubleFault(void);
        extern "C" void CorprocessorSegmentOverrun(void);
        extern "C" void InvalidTSS(void);
        extern "C" void SegmentNotPresent(void);
        extern "C" void StackSegmentFault(void);
        extern "C" void GeneralProtectionFault(void);
        extern "C" void PageFault(void);
        extern "C" void Reserved15(void);
        extern "C" void x87FloatPointException(void);
        extern "C" void AlignmentCheck(void);
        extern "C" void MachineCheck(void);
        extern "C" void SIMDFloatingPointException(void);
        extern "C" void VirtualizationException(void);
        extern "C" void ControlProtectionException(void);
        extern "C" void Reserved22(void);
        extern "C" void Reserved23(void);
        extern "C" void Reserved24(void);
        extern "C" void Reserved25(void);
        extern "C" void Reserved26(void);
        extern "C" void Reserved27(void);
        extern "C" void HypervisorInjectionException(void);
        extern "C" void VMMCommunicationException(void);
        extern "C" void SecurityException(void);

        void ProcessExceptions(int ExceptionNumber , unsigned long ErrorCode);
    }

    namespace PIC {
        void Mask(int InterruptNumber);
        void Unmask(int InterruptNumber);

        void SendEOI(int InterruptNumber);
    }
}

#endif