#ifndef _MBR_HPP_
#define _MBR_HPP_

#include <Kernel.hpp>
#include <Drivers/StorageDriver.hpp>

#define MBR_STARTADDRESS 446

namespace Kernel {
	namespace MBR {
		struct PartitionTableEntry {
			unsigned char BootableFlag;
			unsigned char StartingCHS[3];
			unsigned char PartitionType;
			unsigned char EndingCHS[3];
			unsigned int StartingLBA;
			unsigned int SizeInSector;
		};
		struct PartitionTable {
			unsigned char BootCode[446];
			struct PartitionTableEntry Entries[4];
			unsigned short Signature; // Always 0xAA55
		};
		class Identifier {
			public:
				Identifier(Drivers::StorageSystem::Driver *StorageDriver_
				         , Drivers::StorageSystem::Storage *Storage_)
						 : StorageDriver(StorageDriver_)
						 , Storage(Storage_)
						 , PartitionCount(0) {
					PartitionTable = (struct PartitionTable *)Kernel::MemoryManagement::Allocate(512);
				}
				bool Detect(void);
				Drivers::StorageSystem::Partition *GetPartition(void);

				int PartitionCount;
			private:
				Drivers::StorageSystem::Driver *StorageDriver = 0; 
				Drivers::StorageSystem::Storage *Storage = 0;
				Drivers::StorageSystem::Partition *Partitions;
			    struct PartitionTable *PartitionTable;

				unsigned int CurrentOffset = 0;
		};
	}
}

#endif