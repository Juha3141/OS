#ifndef _ISO9660_H_
#define _ISO9660_H_

struct PVD {
    unsigned char VolumeDescriptionType;
    char StandardIdentifier[5];
    unsigned char Version;
    unsigned char Unused;
    char SystemIdentifier[32];
    char VolumeIdentifier[32];
    unsigned char Unused2[8];
    unsigned int VolumeSpaceSizeL;
    unsigned int VolumeSpaceSizeB;
    unsigned char Unused3[32];
	unsigned short VolumeSetSizeL;
    unsigned short VolumeSetSizeB;
    unsigned short VolumeSequenceNumberL;
    unsigned short VolumeSequenceNumberB;
    unsigned short LogicalBlockSizeL;
    unsigned short LogicalBlockSizeB;
    unsigned int PathTableSizeL;
    unsigned int PathTableSizeB;
    unsigned int PathTableLocationL;
    unsigned int PathTableLocationL_opt;
    unsigned int PathTableLocationM;
    unsigned int PathTableLocationM_opt;
    char RootDirectoryRecord[34];
    char VolumeSizeIdentifier[128];
    char PublisherIdentifier[128];
    char DataPreparerIdentifier[128];
    char ApplicationIdentifier[128];
    char CopyrightFileIdentifier[37];
    char AbstractFileIdentifier[37];
    char BibiliographicalFileIdentifier[37];
    char CreationDateTime[17];
    char ModificationDateTime[17];
    char ExpirationDateTime[17];
	char EffectiveDateTime[17];
    unsigned char FileStructureVersion;
    unsigned char Reserved1;
    char ApplicationUse[512];
    char Reserved2[653];
};


struct DirectoryRecord {
    unsigned char DirectoryLength;
    unsigned char ExtendedAttributeRecordLength;
    unsigned int LocationL;
    unsigned int LocationB;
    unsigned int DataLengthL;
    unsigned int DataLengthB;
    unsigned char Year;
    unsigned char Month;
    unsigned char Day;
    unsigned char Hour;
    unsigned char Minute;
    unsigned char Second;
    
    char OffsetGMT;
    unsigned char FileFlags;
    unsigned char FileUnitSize;
    unsigned char InterleaveGapSize;
    unsigned short VolumeSequenceNumberL;
    unsigned short VolumeSequenceNumberB;
    unsigned char FileIdentifierLength;/*
    unsigned char *FileIdentifier;
    unsigned char *Padding;
    unsigned char *SystemUse;*/
};

struct PathTableEntry {
    unsigned char DirectoryLengthIdentifier;
    unsigned char ExtAttributeRecordLength;
    unsigned int Location;
    unsigned short ParentDirectoryNumber;/*
    char *DirectoryIdentifier;
    unsigned char Padding;*/
};

#endif