#ifndef _PCI_HPP_
#define _PCI_HPP_

#include <Kernel.hpp>
#include <Drivers/DeviceDriver.hpp>

#define PCI_MAX_DEVICE_COUNT    256*32

#define PCI_CONFIG_ADDRESS      0xCF8
#define PCI_CONFIG_DATA         0xCFC
#define PCI_FOWARDING_REGISTER  0xCFA // BUS Number for subsequent PCI space addresses

#define PCI_CLASSCODE_UNCLASSIFIED 0x00
#define PCI_SUBCLASS_NON_VGA_UNIDENTIFIABLE      0x00
#define PCI_SUBCLASS_VGA_COMPATIBLE_UNCLASSIFIED 0x01

namespace Kernel {
    namespace Drivers {
        namespace PCI {
            struct CommonHeader {
                unsigned short VendorID;
                unsigned short DeviceID;

                unsigned short Command;
                unsigned short Status;

                unsigned char RevisionID;
                unsigned char ProgIF;
                unsigned char Subclass;
                unsigned char ClassCode;

                unsigned char CacheLineSize;
                unsigned char LatencyTimer;
                unsigned char HeaderType;
                unsigned char BIST;
            };
            struct HeaderType0x00 {
                struct CommonHeader CommonHeader;
                unsigned int BaseAddress[5];
                unsigned int CardbusCISPointer;
                unsigned short SubsystemVendorID;
                unsigned short SubsystemID;
                unsigned int ExpansionROMAddress;
                unsigned char CapabilitiesPointer;
                unsigned char Reserved[3+4];
                unsigned char InterruptLine;
                unsigned char InterruptPin;
                unsigned char MinGrant;
                unsigned char MaxLatency;
            };
            struct HeaderType0x01 {
                struct CommonHeader CommonHeader;
                unsigned int BaseAddress[2];
                unsigned char PrimaryBusNumber;
                unsigned char SecondaryBusNumber;
                unsigned char SubordinaryBusNumber;
                unsigned char SecondaryLatencyTimer;
                unsigned char IOBase;
                unsigned char IOLimit;
                unsigned short SecondaryStatus;
                unsigned short MemoryBase;
                unsigned short MemoryLimit;
                unsigned short PrefetchableMemoryBase;
                unsigned short PrefetchableMemoryLimit;
                unsigned int UpperPrefetchableBase;
                unsigned int UpperPrefetchableLimits;
                unsigned short UpperIOBase;
                unsigned short LowerIOBase;
                unsigned char CapabilityPointer;
                unsigned char Reserved[3];
                unsigned int ExpansionROMBaseAddress;
                unsigned char InterruptLine;
                unsigned char InterruptPin;
                unsigned char BridgeControl[3];
            };
            struct HeaderType0x02 {
                struct CommonHeader CommonHeader;
                unsigned int CardBusSocketBaseAddress;
                unsigned char CapabilitiesListOffset;
                unsigned char Reserved1;
                unsigned short SecondaryStatus;
                unsigned char PCIBusNumber;
                unsigned char CardBusNumber;
                unsigned char SubordinaryBusNumber;
                unsigned char CardBusLatencyTimer;

                unsigned int MemoryBaseAddress0;
                unsigned int MemoryLimit0;
                unsigned int MemoryBaseAddress1;
                unsigned int MemoryLimit1;
                
                unsigned int IOBaseAddress0;
                unsigned int IOLimit0;
                unsigned int IOBaseAddress1;
                unsigned int IOLimit1;

                unsigned char InterruptLine;
                unsigned char InterruptPin;
                unsigned short BridgeControl;
                unsigned short SubsystemDeviceID;
                unsigned short SubsystemVendorID;

                unsigned int PCCardLegacyModeBaseAddress;
            };
            struct StandardDeviceInfo {
                unsigned char HeaderType;
                
                unsigned long Header;

                unsigned char BusNumber;
                unsigned char SlotNumber;
                unsigned char FunctionNumber;
            };
            class DeviceManager {
                public:
                    void Initialize(void) {
                        DeviceList = (StandardDeviceInfo *)Kernel::MemoryManagement::Allocate(PCI_MAX_DEVICE_COUNT*sizeof(StandardDeviceInfo));
                        DeviceCount = 0;
                    }
                    bool AddDevice(unsigned long HeaderPointer , int BusNumber , int SlotNumber , int FunctionNumber);
                    int GetDevice(unsigned long *HeaderPointer , int BusNumber , int SlotNumber , int FunctionNumber);
                    int GetDeviceByID(unsigned long *HeaderPointer , unsigned char ClassID , unsigned char SubclassID); // ProgIf : Interface Value
                    
                    unsigned int GetBaseAddressRegister(int BAR , int BusNumber , int SlotNumber , int FunctionNumber);
                    static DeviceManager *GetInstance(void) {
                        static DeviceManager *Instance;
                        if(Instance == 0x00) {
                            Instance = (DeviceManager *)Kernel::MemoryManagement::Allocate(sizeof(DeviceManager));
                            return Instance;
                        }
                        return Instance;
                    }
                private:
                    StandardDeviceInfo *DeviceList;
                    int DeviceCount;
            };
            void Detect(void);
            void WriteConfigAddress(unsigned char BusNumber
                                  , unsigned char SlotNumber
                                  , unsigned char FunctionNumber
                                  , unsigned char RegisterOffset);
            unsigned int ReadConfigData(void);
            void CheckBus(unsigned char BusNumber);
            void CheckDevice(unsigned char BusNumber , unsigned char SlotNumber);
            void CheckFunction(unsigned char BusNumber , unsigned char SlotNumber , unsigned char FunctionNumber);
            void *GetDeviceData(unsigned char BusNumber , unsigned char SlotNumber , unsigned char FunctionNumber);
        }
    }
}

#endif