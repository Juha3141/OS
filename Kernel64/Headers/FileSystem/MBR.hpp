#ifndef _MBR_HPP_
#define _MBR_HPP_

#include <Kernel.hpp>
#include <Drivers/StorageDriver.hpp>

#define MBR_STARTADDRESS 446

namespace Kernel {
	namespace MBR {
		struct PartitionTable {
			unsigned char BootableFlag;
			unsigned char CHS[3];
			unsigned char PartitionType;
		};
		class Identifier {
			public:
				Identifier(Drivers::StorageSystem::Driver *StorageDriver_ , Drivers::StorageSystem::Storage *Storage_)
																			 : StorageDriver(StorageDriver_)
																			 , Storage(Storage_)
																			 , PartitionCount(0) {}
				bool DetectMBR(void);
				Drivers::StorageSystem::Partition *GetPartition(void);

				int PartitionCount;
			private:
				Drivers::StorageSystem::Driver *StorageDriver = 0; 
				Drivers::StorageSystem::Storage *Storage = 0; 
				Drivers::StorageSystem::Partition *Partitions;

				unsigned int CurrentOffset = 0;
		};
	}
}

#endif