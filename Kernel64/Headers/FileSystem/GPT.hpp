#ifndef _GPT_HPP_
#define _GPT_HPP_

#include <Kernel.hpp>
#include <Drivers/StorageDriver.hpp>

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
		// char PartitionName[?]
	};
}

#endif