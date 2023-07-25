#ifndef _GPT_HPP_
#define _GPT_HPP_

#include <Kernel.hpp>
#include <Drivers/StorageDriver.hpp>
#include <FileSystem/MBR.hpp>

#define GPT_PARTITION_SYSTEM 	       0x00
#define GPT_PARTITION_EFI_FIRMWARE     0x01
#define GPT_PARTITION_LEGACY_BOOTABLE  0x02
#define GPT_PARTITION_READ_ONLY 	   0x60
#define GPT_PARTITION_SHADOW_COPY      0x61
#define GPT_PARTITION_HIDDEN	 	   0x62
#define GPT_PARTITION_NO_DRIVE_LETTEER 0x63

namespace GPT {
	struct GPTHeader {
		unsigned char Signature[8];
		unsigned int Revision;
		unsigned int HeaderSize;
		unsigned int CRC32Value;
		unsigned int Reserved1;
		unsigned long GPTHeaderLBA;
		unsigned long BackupGPTHeaderLBA;
		unsigned long AvailablePartitionLBA;
		unsigned long EndingAvailPartitionLBA;
		unsigned char DiskGUID[16];
		unsigned long PartitionTableEntryLBA;
		unsigned int NumberOfPartitionEntries;
		unsigned int PartitionTableEntrySize;
		unsigned PartitionTableCRC32Value;
		unsigned char Reserved2[420];
	};
	struct PartitionTableEntry {
		unsigned char PartitionTypeGUID[16];
		unsigned char UniquePartitionGUID[16];
		unsigned long StartAddressLBA;
		unsigned long EndAddressLBA;
		unsigned long AttributeFlag;
		char PartitionName[72];
	};
	class Identifier : public StorageSchemeIdentifier {
		public:
			bool Detect(void);
			struct Partition *GetPartition(void);
			bool CreatePartition(struct Partition Partition);

			struct GPTHeader *Header;
			struct PartitionTableEntry *PartitionTableEntry;
	};
}

#endif