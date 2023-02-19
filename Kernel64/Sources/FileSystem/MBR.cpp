#include <FileSystem/MBR.hpp>

using namespace Kernel;
using namespace Kernel::Drivers;

bool MBR::Identifier::DetectMBR(void) {
    if(StorageDriver == 0x00) {
        return false;
    }
    return true;
}

StorageSystem::Partition *MBR::Identifier::GetPartition(void) {
    // later
    return 0x00;
}