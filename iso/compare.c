#include <stdio.h>
#include <stdlib.h>

int GetFileSize(FILE *File);

int main(int argc , char **argv) {
	int i;
	int Size1;
	int Size2;
	unsigned char *Buffer1;
	unsigned char *Buffer2;
	FILE *File1;
	FILE *File2;
	if(argc != 3) {
		printf("arg\n");
		return -1;
	}
	File1 = fopen(argv[1] , "rb");
	File2 = fopen(argv[2] , "rb");
	if(File1 == NULL) {
		printf("%s not found\n" , argv[1]);
		return -1;
	}
	if(File2 == NULL) {
		printf("%s not found\n" , argv[2]);
		return -1;
	}
	Size1 = GetFileSize(File1);
	Size2 = GetFileSize(File2);
	if(Size1 != Size2) {
		printf("Different size\n");
	}
	printf("Size1 : %d\n" , Size1);
	printf("Size2 : %d\n" , Size2);
	Buffer1 = (unsigned char *)malloc(Size1);
	Buffer2 = (unsigned char *)malloc(Size2);
	fread(Buffer1 , Size1 , 1 , File1);
	fread(Buffer2 , Size2 , 1 , File2);
	for(i = 0; i < ((Size1 < Size2) ? Size1 : Size2); i++) {
		if(Buffer1[i] != Buffer2[i]) {
			printf("Different Value At : 0x%08X (0x%02X != 0x%02X)\n" , i , Buffer1[i] , Buffer2[i]);
		}
	}	
	free(Buffer1);
	free(Buffer2);
	fclose(File1);
	fclose(File2);
	return 0;
}

int GetFileSize(FILE *File) {
	int Size;
	int LastOffset = ftell(File);
	fseek(File , 0 , SEEK_END);
	Size = ftell(File);
	fseek(File , LastOffset , SEEK_SET);
	return Size;
}
