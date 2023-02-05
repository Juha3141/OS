#include <stdarg.h>

unsigned int memset(void *Destination , int Data , unsigned int Size) {
	int i;
	for(i = 0; i < Size; i++) {
		((unsigned char*)Destination)[i] = Data;
	}
	return Size;
}

unsigned int memcpy(void *Destination , const void *Source , unsigned int Size) {
    int i;
    for(i = 0; i < Size; i++) {
        ((unsigned char*)Destination)[i] = ((unsigned char*)Source)[i];
    }
	return Size;
}

int strlen(const char *Destination) {
    int i;
	for(i = 0; Destination[i] != '\0'; i++) {
		;
	}
	return i;
}

char *strcpy(char *Destination , const char *Source) {
	int i;
	memset(Destination , 0 , sizeof(Destination));
	for(i = 0; (Destination[i] != '\0')||(Source[i] != '\0'); i++) {
		Destination[i] = Source[i];
	}
	return Destination;
}

char *strncpy(char *Destination , const char *Source , int Length) {
	int i;
	memset(Destination , 0 , sizeof(Destination));
	for(i = 0; i < Length; i++) {
		Destination[i] = Source[i];
	}
	return Destination;
}

char *strcat(char *Destination , const char *Source) {
	int i = 0;
	int j = 0;
	while(Destination[i] != '\0') {
		i++;
	}
	while(Source[j] != '\0') {
		Destination[i++] = Source[j++];
	}
	Destination[i] = '\0';
	return Destination;
}

char *strncat(char *Destination , const char *Source , int Length) {
	int i = 0;
	int j;
	while(Destination[i] != '\0') {
		i++;
	}
	for(j = 0; j < Length; j++) {
		Destination[i++] = Source[j++];
	}
	Destination[i] = '\0';
	return Destination;
}

int strcmp(const char *FirstString , const char *SecondString) {
	int i;
	int NotEqual = 0;
	for(i = 0; (FirstString[i] != '\0')||(SecondString[i] != '\0'); i++) {
		if(FirstString[i] != SecondString[i]) {
			NotEqual++;
		}
	}
	return NotEqual;
}

int strncmp(const char *FirstString , const char *SecondString , int Length) {
	int i;
	int NotEqual = 0;
	for(i = 0; i < Length; i++) {
		if(FirstString[i] != SecondString[i]) {
			NotEqual++;
		}
	}
	return NotEqual;
}

unsigned long atoi(const char *String) {
	unsigned long Number = 0;
	int Positive = 1;
	if(*String == '-') {
		Positive = -1;
		String++;
	}
	if(*String == '+') {
		Positive = 1;
		String++;
	}
	if(*String == '\0') {
		return 0;
	}
	while(*String != 0x00) {
		if((*String >= '0') && (*String <= '9')) {
			Number = Number*10+(*String)-'0';
			String++;
		}
	}
	Number *= Positive;
	return Number;
}

unsigned long atol(const char *String) {
	unsigned long Number = 0;
	int Positive = 1;
	if(*String == '-') {
		Positive = -1;
		String++;
	}
	if(*String == '+') {
		Positive = 1;
		String++;
	}
	if(*String == '\0') {
		return 0;
	}
	while(*String != 0x00) {
        Number *= 16;
		if(((*String >= 'A') && (*String <= 'Z'))) {
			Number += ((*String)-'A')+10;
			String++;
		}
        else if(((*String >= 'a') && (*String <= 'z'))) {
			Number += ((*String)-'a')+10;
			String++;
        }
        else {
            Number += ((*String)-'0');
            String++;
        }
	}
	Number *= Positive;
	return Number;
}

char *itoa(unsigned long Number , char *String , int Radix) {
	int i = 0;
	int Length;
	unsigned long TempBuffer;
	char Temp;
	if(Number == 0) {
		String[0] = '0';
		String[1] = '\0';
		return String;
	}
	while(1) {
		if(Number == 0) {
			break;
		}
		if(Radix <= 10) {
			String[i++] = (Number%Radix)+'0';
		}
		else {
			TempBuffer = Number%Radix;
			if(TempBuffer <= 9) {
				String[i++] = TempBuffer+'0';
			}
			else {
				String[i++] = TempBuffer-10+'A';
			}
		}
		Number /= Radix;
	}
	String[i] = 0x00;
	Length = i;
	for(i = 0; i < Length/2; i++) {
		Temp = String[i];
		String[i] = String[strlen(String)-i-1];
		String[strlen(String)-i-1] = Temp;
	}
	return String;
}