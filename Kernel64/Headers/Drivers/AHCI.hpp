#ifndef _AHCI_HPP_
#define _AHCI_HPP_

#include <Kernel.hpp>
#include <Drivers/StorageDriver.hpp>
#include <Drivers/PATA.hpp>

#define AHCI_FIS_REG_HOST_TO_DEVICE 0x27
#define AHCI_FIS_REG_DEVICE_TO_HOST 0x34
#define AHCI_FIS_DMA_ACTIVATE       0x39
#define AHCI_FIS_DMA_SETUP          0x41
#define AHCI_TYPE_DATA              0x46
#define AHCI_TYPE_BIST              0x58
#define AHCI_TYPE_PIO_SETUP         0x5F
#define AHCI_TYPE_DEVICE_BITS       0xA1

namespace Kernel {
    namespace Drivers {
        namespace AHCI {
            struct FISHostToDeviceReg {
                unsigned char FISType;
                unsigned char PortMultiplier:4;
                unsigned char Reserved1:3;
                unsigned char Option:1; // 1 : command , 0 : control
                
                unsigned char Command;
                unsigned char FeatureRegLow;

                unsigned char LBAReg0;
                unsigned char LBAReg1;
                unsigned char LBAReg2;
                unsigned char DeviceReg;

                unsigned char LBAReg3;
                unsigned char LBAReg4;
                unsigned char LBAReg5;
                unsigned char FeatureRegHigh;

                unsigned char CountRegLow;
                unsigned char CountRegHigh;
                unsigned char ICC;
                unsigned char ControlRegister;

                unsigned int Reserved2;
            };
            struct FISDeviceToHostReg {
                unsigned char FISType;
                unsigned char PortMultiplier:4;

                unsigned char Reserved1:2;
                unsigned char Interrupt:1;
                unsigned char Reserved2:1;

                unsigned char StatusReg;
                unsigned char ErrorReg;

                unsigned char LBAReg0;
                unsigned char LBAReg1;
                unsigned char LBAReg2;
                unsigned char DeviceReg;

                unsigned char LBA3Reg;
                unsigned char LBA4Reg;
                unsigned char LBA5Reg;
                unsigned char Reserved3;

                unsigned char CountRegisterLow;
                unsigned char CountRegiserHigh;
                
                unsigned long Reserved4;
            };
            struct FISDataReg {
                unsigned char FISType;
                unsigned char PortMultiplier:4;
                unsigned char Reserved1:4;

                unsigned char Reserved2[2];

                unsigned int Data[1]; // DWORD[1 ~ N]
            };
            struct FISPIOSetupReg {
                unsigned char FISType;
                unsigned char PortMultiplier:4;

                unsigned char Reserved1:1;
                unsigned char DataTransferDirection:1;
                unsigned char Interrupt:1;
                unsigned char Reserved2:1;

                unsigned char Status;
                unsigned char Error;

                unsigned char LBAReg0;
                unsigned char LBAReg1;
                unsigned char LBAReg2;
                unsigned char DeviceReg;

                unsigned char LBA3Reg;
                unsigned char LBA4Reg;
                unsigned char LBA5Reg;
                unsigned char Reserved3;

                unsigned char CountRegisterLow;
                unsigned char CountRegiserHigh;
                unsigned char Reserved4;
                unsigned char ExtStatus;
                
                unsigned short TransferCount;
                unsigned short Reserved5;
            };
            struct FISDMASetupReg {
                unsigned char FISType;
                unsigned char PortMultiplier:4;
                unsigned char Reserved1:1;
                unsigned char DataTransferDirection:1;
                // 1 : device to host , 0 : host to device
                unsigned char Interrupt:1;
                unsigned char AutoActivate:1;

                unsigned short Reserved2;
                
                unsigned long DMABufferID;
                
                unsigned int Reserved3;

                unsigned int DMABufferOffset;
                unsigned int TansferCount;

                unsigned int Reserved4;
            };
            struct HBAPort {
                unsigned int CLBA;
                unsigned int CLBAHigh;
                unsigned int FISAddress;
                unsigned int FISAddressHigh;
                unsigned int InterruptStatus;
                unsigned int InterruptEnable;
                unsigned int CommandandStatus;
                unsigned int Reserved1;
                unsigned int TaskFileData;
                unsigned int Signature;
                unsigned int SATAStatus;
                unsigned int SATAControl;
                unsigned int SATAError;
                unsigned int SATAActive;
                unsigned int CommandIssue;
                unsigned int SATANotification;
                unsigned int FISBasedSwitchCtrl;
                unsigned int Reserved2[11];
                unsigned int Vendor[4];
            };
            struct HBAMemory {
                unsigned int Capability;
                unsigned int GlobalHostControl;
                unsigned int InterruptStatus;
                unsigned int PortImplemented;
                unsigned int Version;
                unsigned int CCC_Control;
                unsigned int CCC_Ports;
                unsigned int EM_Location;
                unsigned int EM_Control;
                unsigned int ExtCapability;
                unsigned int BIOSHandoffCS;

                unsigned char Reserved[116];

                unsigned char VendorSpecificReg[96];

                HBAPort Ports[1];
            };

            struct HBAFIS {
                FISDMASetupReg *DMASetupFIS;
                unsigned int Padding1;

                FISPIOSetupReg *PIOSetupFIS;
                unsigned char Padding2[12];

                FISDeviceToHostReg *DeviceToHostFIS;
                unsigned int Padding3;


            };
            
            void Register(void);
            bool WriteRegister(unsigned int Register , void *Data);
            bool ReadRegister(unsigned int Register , void *Data);

            bool PreInitialization(void);
            bool GetGeometry(StorageSystem::Storage *Storage , StorageSystem::StorageGeometry *Geometry);
            unsigned long ReadSector(StorageSystem::Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer);
            unsigned long WriteSector(StorageSystem::Storage *Storage , unsigned long SectorAddress , unsigned long Count , void *Buffer);
        }
    }
}

#endif