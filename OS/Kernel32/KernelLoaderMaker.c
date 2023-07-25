#include <stdio.h>
#include <stdlib.h>

int main(int argc , char **argv) {
	// argv 1 : Kernel16.bin
	// argv 2 : Kernel32.bin
	// argv 3 : APLoader.bin
	// argv 4 : KernelLoader.krn
	// argv 5 : 2048
	// argc   : 6
	int i;
	unsigned long j;
	unsigned long Alignment;
	int Padding;
	FILE **Files;
	int *FilesSize;
	FILE *Target;
	if(argc < 5) {
		printf("Usage : %s [Files1] [File2] ... [Target] [Alignment] [Padding in hex]\n" , argv[0]);
		return -1;
	}
	Files = (FILE **)malloc((argc-4)*sizeof(FILE *));
	FilesSize = (int *)malloc((argc-4)*sizeof(int));
	Alignment = atoi(argv[argc-2]);
	Padding = strtol(argv[argc-1] , 0 , 16);
	printf("Aligning to %ld\n" , Alignment);
	printf("Padding : 0x%X\n" , Padding);
	printf("Target  : %s\n" , argv[argc-3]);
	if(Padding > 255) {
		printf("Warning : Padding above 0xFF will be ignored!\n");
		Padding = 0x00;
	}
	for(i = 0; i < argc-4; i++) {
		Files[i] = fopen(argv[1+i] , "rb");
		if(Files[i] == 0) {
			printf("File not found : %s\n" , argv[1+i]);
			return -1;
		}
		printf("File : %s\n" , argv[1+i]);
	}
	Target = fopen(argv[argc-3] , "wb");
	for(i = 0; i < argc-4; i++) {
		fseek(Files[i] , 0 , SEEK_END);
		FilesSize[i] = ftell(Files[i]);
		fseek(Files[i] , 0 , SEEK_SET);
		printf("File %s Size : %d\n" , argv[1+i] , FilesSize[i]);
		for(j = 0; j < FilesSize[i]; j++) {
			if(fputc(fgetc(Files[i]) , Target) == EOF) {
				printf("Error while reading %s\n" , argv[1+i]);
				return -1;
			}
		}
		if(FilesSize[i]%Alignment != 0) {
			printf("Align : %ld\n" , Alignment-FilesSize[i]%Alignment);
			for(j = 0; j < Alignment-FilesSize[i]%Alignment; j++) {
				if(fputc((int)Padding , Target) == EOF) {
					printf("Error while reading %s\n" , argv[1+i]);
					return -1;
				}
			}
		}
		printf("Wrote %s to aligned location %ld\n" , argv[1+i] , ftell(Target));
	}
	for(i = 0; i < argc-4; i++) {
		fclose(Files[i]);
	}
	fclose(Target);
}
