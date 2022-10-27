#ifndef _EXCEPTIONHANDLERS_HPP_
#define _EXCEPTIONHANDLERS_HPP_

#include <DescriptorTables.hpp>
#include <EssentialLibrary.hpp>
#include <TextScreen.hpp>

namespace Kernel {
    namespace Exceptions {
        __attribute__ ((naked)) void DividedByZero(void);
        __attribute__ ((naked)) void Debug(void);
        __attribute__ ((naked)) void NonMaskableInterrupt(void);
        __attribute__ ((naked)) void Breakpoint(void);
        __attribute__ ((naked)) void Overflow(void);
        __attribute__ ((naked)) void BoundRangeExceeded(void);
        __attribute__ ((naked)) void InvalidOpcode(void);
        __attribute__ ((naked)) void DeviceNotAvailable(void);
        __attribute__ ((naked)) void DoubleFault(void);
        __attribute__ ((naked)) void CorprocessorSegmentOverrun(void);
        __attribute__ ((naked)) void InvalidTSS(void);
        __attribute__ ((naked)) void SegmentNotPresent(void);
        __attribute__ ((naked)) void StackSegmentFault(void);
        __attribute__ ((naked)) void GeneralProtectionFault(void);
        __attribute__ ((naked)) void PageFault(void);
        __attribute__ ((naked)) void Reserved15(void);
        __attribute__ ((naked)) void x87FloatPointException(void);
        __attribute__ ((naked)) void AlignmentCheck(void);
        __attribute__ ((naked)) void MachineCheck(void);
        __attribute__ ((naked)) void SIMDFloatingPointException(void);
        __attribute__ ((naked)) void VirtualizationException(void);
        __attribute__ ((naked)) void ControlProtectionException(void);
        __attribute__ ((naked)) void Reserved22(void);
        __attribute__ ((naked)) void Reserved23(void);
        __attribute__ ((naked)) void Reserved24(void);
        __attribute__ ((naked)) void Reserved25(void);
        __attribute__ ((naked)) void Reserved26(void);
        __attribute__ ((naked)) void Reserved27(void);
        __attribute__ ((naked)) void HypervisorInjectionException(void);
        __attribute__ ((naked)) void VMMCommunicationException(void);
        __attribute__ ((naked)) void SecurityException(void);

        void ProcessExceptions(int ExceptionNumber , unsigned long ErrorCode);
    };
};

#endif