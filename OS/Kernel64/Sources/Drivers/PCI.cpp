#include <Drivers/PCI.hpp>

bool PCI::DeviceManager::AddDevice(unsigned long HeaderPointer , int BusNumber , int SlotNumber , int FunctionNumber) {
    DeviceList[DeviceCount].BusNumber = BusNumber;
    DeviceList[DeviceCount].SlotNumber = SlotNumber;
    DeviceList[DeviceCount].FunctionNumber = FunctionNumber;
    DeviceList[DeviceCount].HeaderType = ((CommonHeader *)HeaderPointer)->HeaderType;
    DeviceList[DeviceCount].Header = HeaderPointer;
    DeviceCount++;
    return true;
}

int PCI::DeviceManager::GetDevice(unsigned long *Header , int BusNumber , int SlotNumber , int FunctionNumber) {
    int i;
    for(i = 0; i < DeviceCount; i++) {
        if((DeviceList[i].BusNumber == BusNumber)
        && (DeviceList[i].SlotNumber == SlotNumber)
        && (DeviceList[i].FunctionNumber == FunctionNumber)) {
            *Header = DeviceList[i].Header;
            return DeviceList[i].HeaderType;
        }
    }
    return -1;
}

int PCI::DeviceManager::GetDeviceByID(unsigned long *Header , unsigned char ClassID , unsigned char SubclassID) {
    int i;
    for(i = 0; i < DeviceCount; i++) {
        if((((CommonHeader *)DeviceList[i].Header)->ClassCode = ClassID)
        && (((CommonHeader *)DeviceList[i].Header)->Subclass == SubclassID)) {
            *Header = DeviceList[i].Header;
            printf("Header : 0x%X\n" , *Header);
            return DeviceList[i].HeaderType;
        }
    }
    return -1;
}

void PCI::Detect(void) {
    int i;
    CommonHeader *Header;
    HeaderType0x01 *PCItoPCIBridge;
    DeviceManager *DeviceManager = DeviceManager::GetInstance();
    DeviceManager->Initialize();
    // PCI bus 0 slot 0 is always host controller
    if(((Header = (CommonHeader *)GetDeviceData(0 , 0 , 0))->HeaderType & 0x80) == 0x00) { // multiple function
        CheckBus(0); // only one PCI host conytoller
    }
    else {
        for(i = 0; i < 8; i++) {
            PCItoPCIBridge = (HeaderType0x01 *)GetDeviceData(0 , 0 , i);
            if(PCItoPCIBridge->CommonHeader.VendorID == 0xFFFF) {
                break;
            }
            CheckBus(i);
        }
    }
}

void PCI::CheckBus(unsigned char BusNumber) {
    int i;
    int j;
    void *Header;
    DeviceManager *DeviceManager = DeviceManager::GetInstance();
    for(i = 0; i < 32; i++) {
        for(j = 0; j < 8; j++) {
            if((Header = GetDeviceData(BusNumber , i , j)) != 0x00) {
                DeviceManager->AddDevice((unsigned long)Header , BusNumber , i , j);
        #ifdef DEBUG
                Keyboard::GetASCIIData();
        #endif
            }
        }
    }
}

void *PCI::GetDeviceData(unsigned char BusNumber , unsigned char SlotNumber , unsigned char FunctionNumber) {
    int i;
    int j;
    int SecondaryBus;
    unsigned int Data[256/4];
    CommonHeader *CommonHeader = (PCI::CommonHeader *)(&(Data));
    HeaderType0x00 *GeneralDevice;
    HeaderType0x01 *PCItoPCIBridge;
    HeaderType0x02 *PCItoCardBusBridge;
    for(i = j = 0; i < 256; i += 4) {
        PCI::WriteConfigAddress(BusNumber , SlotNumber , FunctionNumber , i);
        Data[j++] = PCI::ReadConfigData();
    }
    if(CommonHeader->VendorID == 0xFFFF) {
        return 0x00;
    }
#ifdef DEBUG
    printf("Bus %d , Slot %d , Function %d\n" , BusNumber , SlotNumber , FunctionNumber);
    printf("Vendor ID   : 0x%X\n" , CommonHeader->VendorID);
    printf("Device ID   : 0x%X\n" , CommonHeader->DeviceID);
    printf("Header Type : 0x%X\n" , CommonHeader->HeaderType);
    printf("Class Code  : 0x%X\n" , CommonHeader->ClassCode);
    printf("Subclass    : 0x%X\n" , CommonHeader->Subclass);
    printf("ProgIF      : 0x%X\n" , CommonHeader->ProgIF);
#endif
    if((CommonHeader->ClassCode == 0x06) && (CommonHeader->Subclass == 0x04)) {
        PCItoPCIBridge = (HeaderType0x01 *)CommonHeader;
        SecondaryBus = PCItoPCIBridge->SubordinaryBusNumber;
        CheckBus(SecondaryBus);
    }
    switch(CommonHeader->HeaderType & 0b1111111) {
        case 0x00:
            GeneralDevice = (HeaderType0x00 *)MemoryManagement::Allocate(sizeof(HeaderType0x00));
            memcpy(GeneralDevice , Data , 256);
    #ifdef DEBUG
            printf("BaseAddress0        : 0x%X\n" , GeneralDevice->BaseAddress[0]);
            printf("BaseAddress1        : 0x%X\n" , GeneralDevice->BaseAddress[1]);
            printf("BaseAddress2        : 0x%X\n" , GeneralDevice->BaseAddress[2]);
            printf("BaseAddress3        : 0x%X\n" , GeneralDevice->BaseAddress[3]);
            printf("BaseAddress4        : 0x%X\n" , GeneralDevice->BaseAddress[4]);
            printf("ExpansionROMAddress : 0x%X\n" , GeneralDevice->ExpansionROMAddress);
            printf("Interrupt Pin  : %d\n" , GeneralDevice->InterruptPin);
            printf("Interrupt Line : %d\n" , GeneralDevice->InterruptLine);
    #endif
            return GeneralDevice;
        case 0x01:
            PCItoPCIBridge = (HeaderType0x01 *)MemoryManagement::Allocate(sizeof(HeaderType0x01));
            memcpy(PCItoPCIBridge , Data , 256);
    #ifdef DEBUG
            printf("BaseAddress0  : 0x%X\n" , PCItoPCIBridge->BaseAddress[0]);
            printf("BaseAddress1  : 0x%X\n" , PCItoPCIBridge->BaseAddress[1]);
            printf("MemoryBase    : 0x%X~0x%X\n" , PCItoPCIBridge->MemoryBase , PCItoPCIBridge->MemoryLimit);
            printf("IOAddress     : 0x%X~0x%X\n" , PCItoPCIBridge->IOBase , PCItoPCIBridge->IOLimit);
            printf("Interrupt Pin : %d\n" , PCItoPCIBridge->InterruptPin);
    #endif
            return PCItoPCIBridge;
        case 0x02:
            PCItoCardBusBridge = (HeaderType0x02 *)MemoryManagement::Allocate(sizeof(HeaderType0x02));
            memcpy(PCItoCardBusBridge , Data , 256);
    #ifdef DEBUG
            printf("BaseAddress0  : 0x%X~0x%X\n" , PCItoCardBusBridge->MemoryBaseAddress0 , PCItoCardBusBridge->MemoryLimit0);
            printf("BaseAddress1  : 0x%X~0x%X\n" , PCItoCardBusBridge->MemoryBaseAddress1 , PCItoCardBusBridge->MemoryLimit1);
            printf("IOAddress1    : 0x%X~0x%X\n" , PCItoCardBusBridge->IOBaseAddress0 , PCItoCardBusBridge->IOLimit0);
            printf("IOAddress0    : 0x%X~0x%X\n" , PCItoCardBusBridge->IOBaseAddress1 , PCItoCardBusBridge->IOLimit1);
            printf("Interrupt Pin : %d\n" , PCItoCardBusBridge->InterruptPin);
    #endif
            return PCItoCardBusBridge;
    }
    return 0x00;
}

void PCI::WriteConfigAddress(unsigned char BusNumber
                           , unsigned char SlotNumber
                           , unsigned char FunctionNumber
                           , unsigned char RegisterOffset) {
    unsigned int Address;
    Address = (RegisterOffset & 0xFC)|(FunctionNumber << 8)|(SlotNumber << 11)|(BusNumber << 16)|(1 << 31);
    IO::WriteDWord(PCI_CONFIG_ADDRESS , Address);
}

unsigned int PCI::ReadConfigData(void) {
    return IO::ReadDWord(PCI_CONFIG_DATA);
}

