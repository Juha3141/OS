#ifndef _APIC_HPP_
#define _APIC_HPP_

#include <Kernel.hpp>
#include <EssentialLibrary.hpp>
#include <ACPI.hpp>

#define LAPIC_ID_REGISTER                                   0x20
#define LAPIC_VERSION_REGISTER                              0x30
#define LAPIC_TASK_PRIORITY_REGISTER                        0x80
#define LAPIC_ARTIBRATION_PRIORITY_REGISTER                 0xA0
#define LAPIC_EOI_REGISTER                                  0xB0
#define LAPIC_REMOTE_READ_REGISTER                          0xC0
#define LAPIC_LOGICAL_DESTINATION_REGISTER                  0xD0
#define LAPIC_DESTINATION_FORMAT_REGISTER                   0xE0
#define LAPIC_LVT_SPURIOUS_REGISTER                         0xF0
#define LAPIC_IN_SERVICE_REGISTER                           0x100
#define LAPIC_TRIGGER_MODE_REGISTER                         0x180
#define LAPIC_INTERRUPT_REQUEST_REGISTER                    0x200
#define LAPIC_ERROR_STATUS_REGISTER                         0x280
#define LAPIC_CMCMI_REGISTER                                0x2F0
#define LAPIC_INTERRUPT_COMMAND_REGISTER                    0x300
#define LAPIC_INTERRUPT_COMMAND_REGISTER_HIGH               0x310
#define LAPIC_LVT_TIMER_REGISTER                            0x320
#define LAPIC_LVT_THERMAL_SENSOR_REGISTER                   0x330
#define LAPIC_LVT_PERFORMANCE_MONITORING_COUNTERS_REGISTER  0x340
#define LAPIC_LVT_LINT0_REGISTER                            0x350
#define LAPIC_LVT_LINT1_REGISTER                            0x360
#define LAPIC_LVT_ERROR_REGISTER                            0x370
#define LAPIC_INITIAL_COUNT_REGISTER                        0x380
#define LAPIC_CURRENT_COUNT_REGISTER                        0x390
#define LAPIC_DIVIDE_CONFIG_REGISTER                        0x3E0

// Interrup Command Register 

#define LAPIC_ICR_PHYSICAL_DESTINATION_MODE                 0b000000000000
#define LAPIC_ICR_SENT_STATUS_PENDING                       0b1000000000000
#define LAPIC_ICR_LEVEL_TRIGGER                             0b100000000000000

#define LAPIC_ICR_IPI_INIT                                  0b10100000000
#define LAPIC_ICR_IPI_STARTUP                               0b11000000000

// For Logical Destination Mode
// Destination Shorthand
#define LAPIC_ICR_NOT_USING_ABBREVIATION                    0b00000000000000000000
#define LAPIC_ICR_SENDING_JUST_FOR_ME                       0b01000000000000000000
#define LAPIC_ICR_SENDING_FOR_EVERYONE_INCLUDING_ME         0b10000000000000000000
#define LAPIC_ICR_SENDING_FOR_EVERYONE_EXCEPT_FOR_ME        0b11000000000000000000

#define LAPIC_ICR_LEVEL_ASSERT                              0b100000000000000
#define LAPIC_ICR_LEVEL_DEASSERT                            0b000000000000000
                                                            
#define LAPIC_TIMER_DELIBERY_STATE_PENDING                  0b0000001000000000000
#define LAPIC_TIMER_MASK_INTERRUPT                          0b0010000000000000000
#define LAPIC_TIMER_MODE_SINGLE_PULSE                       0b0000000000000000000
#define LAPIC_TIMER_MODE_PERIODIC_PULSE                     0b0100000000000000000

#define LAPIC_TIMER_DELAY_MS                                50

#define LAPIC_LVT_NMI                                       0b00000010000000000
#define LAPIC_LVT_INTERRUPT_PENDING                         0b00001000000000000
#define LAPIC_LVT_LOW_TRIGGERED                             0b00010000000000000
#define LAPIC_LVT_REMOTE_IRR                                0b00100000000000000
#define LAPIC_LVT_LEVEL_TRIGGER                             0b01000000000000000
#define LAPIC_LVT_MASKED                                    0b10000000000000000

#define IOAPIC_REGISTER_SELECTOR                            0x00
#define IOAPIC_REGISTER_WINDOW                              0x10

#define IOAPIC_REGISTER_APICID                              0x00
#define IOAPIC_REGISTER_IOAPIC_VERSION                      0x01
#define IOAPIC_REGISTER_IO_REDIRECTION_TABLE                0x10 // 0x10 ~ 0x3F

#define IOAPIC_IOREDTBL_DELIVERY_MODE_FIXED                 0b000 // Trigger Mode : Edge / Level
#define IOAPIC_IOREDTBL_DELIVERY_MODE_LOWEST_PRIORITY       0b001 // Trigger Mode : Edge / Level
#define IOAPIC_IOREDTBL_DELIVERY_MODE_SMI                   0b010 // SMI : System Management Interrupt
#define IOAPIC_IOREDTBL_DELIVERY_MODE_NMI                   0b100 // Trigger Mode : Edge
#define IOAPIC_IOREDTBL_DELIVERY_MODE_INIT                  0b101 // Trigger Mode : Edge
#define IOAPIC_IOREDTBL_DELIVERY_MODE_EXTINT                0b111 // Edge

namespace LocalAPIC {
    struct LocalVectorTableRegister {
        unsigned char VectorNumber;
        unsigned char NMI:3;
        unsigned char Reserved1:1;
        unsigned char InterruptPending:1;
        unsigned char Polarity:1;
        unsigned char RemoteIRR:1;
        unsigned char TriggerMode:1;
        unsigned char SetToMask;
        unsigned short Reserved2;
    };
    void WriteRegister(unsigned int RegisterAddress , unsigned int Data);
    void WriteRegister_L(unsigned int RegisterAddress , unsigned long Data);
    
    unsigned int ReadRegister(unsigned int RegisterAddress);
    unsigned long ReadRegister_L(unsigned int RegisterAddress);
    void GlobalEnableLocalAPIC(void);
    void EnableLocalAPIC(void);
    bool CheckBSP(void);
    void ActivateAPCores(void);
    bool SendInitSignal(unsigned int APICID);
    bool SendActivateSignal(unsigned int APICID);
    unsigned int GetActivatedCoreCount(void);
    unsigned int GetCurrentAPICID(void);
    void SendEOI(void);
    namespace Timer {
        void Initialize(void);
        unsigned int GetInitialValue(void);
        void SetInitialValue(unsigned int Value);
        void Enable(void);
        void Disable(void);
        unsigned int GetTickCount(void);
        void InterruptHandler(void);
        void MainInterruptHandler(void);
        
    }
}
namespace IOAPIC {
    struct IORedirectionTable {
        unsigned char InterruptVector;
        unsigned char DeliveryMode:3;
        unsigned char DestinationMode:1; // 0 : Physical Mode , 0 : Logical Mode
        unsigned char DeliveryStaus:1; // 0 : Idle , 1 : Send Pending
        unsigned char InterruptPinPolarity:1; // 0 : High Active , 1 : Low Active
        unsigned char RemoteIRR:1; // Read Only
        unsigned char TriggerMode:1; // 0 : Edge sensitive , 1 : Level sensitive
        unsigned char InterruptMask:1; // 0 : Unmasked , 1 : Masked
        unsigned long Reserved:39;
        unsigned char DestinationAddress;
    };
    void InitializeRedirectionTable(void);
    void ReadIORedirectionTable(int INTIN , struct IORedirectionTable *RedirectionTable);
    void WriteIORedirectionTable(int INTIN , struct IORedirectionTable RedirectionTable);
    void WriteRegister(unsigned char RegisterAddress , unsigned int Data);
    unsigned int ReadRegister(unsigned char RegisterAddress);
}

#endif