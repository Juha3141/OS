#ifndef _MBR_HPP_
#define _MBR_HPP_

#include <Kernel.hpp>
#include <Drivers/StorageDriver.hpp>
#include <MemoryManagement.hpp>

#define MBR_STARTADDRESS 446

class StorageSchemeIdentifier {
	public:
		void WriteStorageInfo(struct StorageDriver *_Driver , struct Storage *_Storage) {
			Driver = _Driver;
			Storage = _Storage;
			PartitionCount = 0;
		}
		virtual bool Detect(void) = 0;
		virtual struct Partition *GetPartition(void) = 0;
		virtual bool CreatePartition(struct Partition Partition) = 0;

		int PartitionCount = 0;
	protected:
		struct StorageDriver *Driver = 0; 
		struct Storage *Storage = 0;
		struct Partition *Partitions;

		unsigned int CurrentOffset = 0;
};
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
		unsigned char BootCode[MBR_STARTADDRESS];
		struct PartitionTableEntry Entries[4];
		unsigned short Signature; // Always 0xAA55
	};
	class Identifier : public StorageSchemeIdentifier {
		public:
			bool Detect(void);
			struct Partition *GetPartition(void);
			bool CreatePartition(struct Partition Partition);
		private:
		    struct PartitionTable *PartitionTable;
	};
}

#endif