#include <stdarg.h>

void vsprintf(char *Destination , const char *Format , va_list ap) {
	int i;
	int j = 0;
	for(i = 0; Format[i] != 0; i++) {
		switch(Format[i]) {
			case '%':
				switch(Format[i+1]) {
					case 'c': {
						unsigned char Buffer = va_arg(ap , int);
						Destination[j++] = Buffer;
						i++;
						break;
					}
					case 's': {
						char *p = va_arg(ap , char *);
						strcpy(Destination+j , p);
						j += strlen(p);
						i++;
						break;
					}
					case 'd':
					case 'i': {
						unsigned long Value = va_arg(ap , unsigned long);
						char Buffer[128];
						memset(Buffer , 0 , 128);
						itoa(Value , Buffer , 10);
						strcpy(Destination+j , Buffer);
						j += strlen(Buffer);
						i++;
						break;
					}
					case 'X': {
						unsigned long Value = va_arg(ap , unsigned long);
						char Buffer[128];
						memset(Buffer , 0 , 128);
						itoa(Value , Buffer , 16);
						strcpy(Destination+j , Buffer);
						j += strlen(Buffer);
						i++;
						break;
					}
					case 'f': {
						break; 
					}
					case '%': {
						Destination[j++] = '%';
						i++;
						break;
					}
					default: {
						i++;
						break;
					}
				}
				break;
			default:
				Destination[j++] = Format[i];
				break;
		}
	}
	Destination[j] = '\0';
}

void sprintf(char *Destination , const char *Format , ...) {
	va_list ap;
	va_start(ap , Format);
	vsprintf(Destination , Format , ap);
	va_end(ap);
}