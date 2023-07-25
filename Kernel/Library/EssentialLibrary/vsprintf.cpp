#include <cstdarg>

extern void *memset(void *Destination , unsigned char Value , unsigned long Size);
extern void *memcpy(void *Destination , const void *Source , unsigned long Size);
extern unsigned long strlen(const char *String);
extern char *strcpy(char *Destination , const char *Origin);
extern char *strncpy(char *Destination , const char *Origin , unsigned long Length);
extern char *strcat(char *Destination , const char *Origin);

int SizeOfNumber(int Number);
int IndexOfNumber(int Number , int Index);

unsigned long atoi(const char *Buffer) {
	unsigned long Value = 0;
	int Positive = 1;
	if(*Buffer == '-') {
		Positive = -1;
		Buffer++;
	}
	if(*Buffer == '+') {
		Positive = 1;
		Buffer++;
	}
	if(*Buffer == '\0') {
		return 0;
	}
	while(*Buffer != 0) {
		if((*Buffer >= '0') && (*Buffer <= '9')) {
			Value = Value*10+(*Buffer)-'0';
			Buffer++;
		}
	}
	if(Positive == -1) {
		Value = -Value;
	}
	return Value;
}

unsigned long atol(const char *Buffer) {
	unsigned long Value = 0;
	int Positive = 1;
	if(*Buffer == '-') {
		Positive = -1;
		Buffer++;
	}
	if(*Buffer == '+') {
		Positive = 1;
		Buffer++;
	}
	if(*Buffer == '\0') {
		return 0;
	}
	while(*Buffer != 0) {
        Value *= 16;
		if(((*Buffer >= 'A') && (*Buffer <= 'Z'))) {
			Value += ((*Buffer)-'A')+10;
			Buffer++;
		}
        else if(((*Buffer >= 'a') && (*Buffer <= 'z'))) {
			Value += ((*Buffer)-'a')+10;
			Buffer++;
        }
        else {
            Value += ((*Buffer)-'0');
            Buffer++;
        }
	}
	if(Positive == -1) {
		Value = -Value;
	}
	return Value;
}

void ReverseString(char *Buffer) {
	int i;
	char TempBuffer;
	for(i = 0; i < strlen(Buffer)/2; i++) {
		TempBuffer = Buffer[i];
		Buffer[i] = Buffer[(strlen(Buffer)-1)-i];
		Buffer[(strlen(Buffer)-1)-i] = TempBuffer;
	}
	return;
}

char *itoa(unsigned long Value , char *Buffer , int Radix) {
	unsigned long TempBuffer;
	if(Value == 0) {
		Buffer[0] = '0';
		Buffer[1] = '\0';
		return Buffer;
	}
	while(Value) {
		if(Radix <= 10) {
			*Buffer++ = (Value%Radix)+'0';
		}
		else {
			TempBuffer = Value%Radix;
			if(TempBuffer <= 9) {
				*Buffer++ = TempBuffer+'0';
			}
			else {
				*Buffer++ = TempBuffer-10+'A';
			}
		}
		Value /= Radix;
	}
	*Buffer = '\0';
	ReverseString(Buffer);
	return Buffer;
}

void vsprintf(char *Destination , const char *Format , va_list ap) {
	int i;
	int j = 0;
	for(i = 0; i < strlen(Format); i++) {
		switch(Format[i]) {
			case '%':
				switch(Format[i+1]) {
					case 'c': {
						char Buffer = va_arg(ap , int);
						Destination[j++] = Buffer;
						i++;
						break;
					}
					case 's': {
						char *Buffer = va_arg(ap , char*);
						strcpy(Destination+j , Buffer);
						j += strlen(Buffer);
						i++;
						break;
					}
					case 'd':
					case 'i': {
						int Value = va_arg(ap , int);
						char Buffer[128] = {0 , };
						itoa(Value , Buffer , 10);
						ReverseString(Buffer);
						strcpy(Destination+j , Buffer);
						j += strlen(Buffer);
						i++;
						break;
					}
					case 'X': {
						unsigned long Value = va_arg(ap , unsigned long);
						char Buffer[128] = {0 , };
						itoa(Value , Buffer , 16);
						ReverseString(Buffer);
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

/*
int ToString_Hex(char *String , unsigned long long Value , char UpperCase) {
    int i = 0;
    int StringLength = 0;
    char Temporary;
    bool Negative = Value < 0;
    if(Value == 0) {
        String[0] = '0';
        String[1] = 0x00;
        return 1;
    }
    while(Value != 0) {
        if(((Value%16)) > 9) {
            String[i] = (Value%16)+((UpperCase == 1) ? 'A' : 'a')-0x0A;
        }
        else {
            String[i] = (Value%16)+'0';
        }
        Value /= 16;
        i++;
    }
    StringLength = i+((((StringLength+1)%2) == 0) ? 0 : -1);
    for(i = 0; i <= StringLength/2; i++) {
        Temporary = String[i];
        String[i] = String[StringLength-i];
        String[StringLength-i] = Temporary;
    }
    String[StringLength+1] = 0x00;
    return StringLength+1;
}

int ToString(char *String , unsigned long long Value) {
    int i = 0;
    int StringLength = 0;
    char Temporary;
    if(Value == 0) {
        String[0] = '0';
        String[1] = 0x00;
        return 1;
    }
    while(Value != 0) {
        String[i] = (Value%10)+'0';
        Value /= 10;
        i++;
    }
    StringLength = i+((((StringLength+1)%2) == 0) ? 0 : -1);
    for(i = 0; i <= StringLength/2; i++) {
        Temporary = String[i];
        String[i] = String[StringLength-i];
        String[StringLength-i] = Temporary;
    }
    String[StringLength+1] = 0x00;
    return StringLength+1;
}

int ToString(char *String , long long Value) {
    int i = 0;
    int StringLength = 0;
    char Temporary;
    bool Negative = Value < 0;
    if(Value == 0) {
        String[0] = '0';
        String[1] = 0x00;
        return 1;
    }
    Value = ((Negative == 1) ? -Value : Value);
    while(Value != 0) {
        String[i] = (Value%10)+'0';
        Value /= 10;
        i++;
    }
    if(Negative == 1) {
        String[i] = '-';
        i++;
    }
    StringLength = i+((((StringLength+1)%2) == 0) ? 0 : -1);
    for(i = 0; i <= StringLength/2; i++) {
        Temporary = String[i];
        String[i] = String[StringLength-i];
        String[StringLength-i] = Temporary;
    }
    String[StringLength+1] = 0x00;
    return StringLength+1;
}

int ToString(char *String , unsigned long Value) {
    return ToString(String , (unsigned long long)Value);
}

int ToString(char *String , unsigned int Value) {
    return ToString(String , (unsigned long long)Value);
}

int ToString(char *String , int Value) {
    return ToString(String , (long long)Value);
}

int ToString(char *String , long Value) {
    return ToString(String , (long long)Value);
}

int ToString_Double(char *String , long double Value , int Length) {
    int i = 0;
    int j = 0;
    int Position = 0;
    int StringLength;
    int Temporary;
    long double BackupValue = Value;
    while(((unsigned long)Value) != 0) {
        String[i] = (((unsigned long)Value)%10)+'0';
        Value /= 10;
        i++;
    }
    StringLength = i;
    for(i = 0; i <= (int)((StringLength+(((i%2) == 0) ? 0 : 1))/2); i++) {
        Temporary = String[i];
        String[i] = String[StringLength-i];
        String[StringLength-i] = Temporary;
    }
    i = StringLength+1;
    String[i++] = '.';
    for(Position = i; i < Length+Position; i++) {
        BackupValue = BackupValue-((long double)((unsigned long)BackupValue));
        BackupValue *= 10;
        String[i] = (((unsigned long)BackupValue)%10)+'0';
    }
    String[i] = 0x00;
    return i;
}

int ToString(char *String , float Value) {
    return ToString_Double(String , (long double)Value , 4);
}

int ToString(char *String , double Value) {
    return ToString_Double(String , (long double)Value , 6);
}

int ToString(char *String , long double Value) {
    return ToString_Double(String , (long double)Value , 6);
}

int ToInteger(const char *String) {
    int i;
    int Value = 0;
    int Power = 1;
    for(i = strlen(String)-1; i >= 0; i--) {
        Value += (String[i]-'0')*Power;
        Power *= 10;
    }
    return Value;
}

char GetFlag(const char *Format) {
    switch(Format[0]) {
        case '+':
        case '-':
        case ' ':
        case '0':
        case '\'':
        case '#':
            return Format[0];
        default:
            return 0x00;
    }
}

int GetWidth(const char *Format , int *CurrentPosition) {
    int i;
    int Position;
    char WidthString[16] = {0 , };
    Position = (GetFlag(Format) == 0) ? 0 : 1;
    if((Format[Position] < '0')||(Format[Position] > '9')) {
        *(CurrentPosition) = 0;
        return 0;
    }
    for(i = Position; Format[i] != 0x00; i++) {
        if(Format[i] == '.') {
            break;
        }
        if(Format[i] == ' ') {
            break;
        }
        if((Format[i] < '0')||(Format[i] > '9')) {
            break;
        }
        WidthString[i-Position] = Format[i];
    }
    WidthString[i-Position] = 0x00;
    *(CurrentPosition) = i-Position;
    return ToInteger(WidthString);
}

int GetPrecision(const char *Format , int LastPosition , int *CurrentPosition) {
    int i;
    char PrecisionString[16] = {0 , };
    if(Format[LastPosition-1] != '.') {
        return 0;
    }
    for(i = LastPosition;; i++) {
        if((Format[i] < '0')||(Format[i] > '9')) {
            break;
        }
        PrecisionString[i-LastPosition] = Format[i];
    }
    PrecisionString[i-LastPosition] = 0x00;
    *(CurrentPosition) = i;
    return ToInteger(PrecisionString);
}

unsigned short GetLength(const char *Format , int LastPosition) {
    int i;
    unsigned short Length;
    switch(Format[LastPosition]) {
        case 'l':
        case 'h':
        case 'L':
        case 'z':
        case 'j':
        case 't':
            Length = Format[LastPosition];
        default:
            Length = 0;
            break;
    }
    switch(Format[LastPosition+1]) {
        case 'l':
        case 'h':
            Length |= ((Format[LastPosition+1]) << 8);
            break;
        default:
            Length = 0;
            break;
    }
    return Length;
}

char GetType(const char *Format , unsigned short Length , int LastPosition) {
    char Type;
    if((Length & 0x0F) == 0x00) {
        Type = Format[LastPosition];
    }
    else if(((Length & 0xF0) >> 4) == 0) {
        Type = Format[LastPosition+1];
    }
    else {
        Type = Format[LastPosition+2];
    }
    return Type;
}
//https://en.wikipedia.org/wiki/Printf_format_string
int vsprintf(char *String , const char *Format , va_list ap) {
	unsigned long i;
	unsigned long j = 0;
    unsigned long k;
    char *StringPointer;
    char Flags;
    int Width;
    int Precision;
    unsigned short Length;
    char Type;
    int CurrentPosition = 0;
    
	int Integer;
    unsigned int UnsignedInteger;
    unsigned long UnsignedLong;

    double Double;
    char TemporaryBuffer1[128];
    char TemporaryBuffer2[16];
    for(i = 0; i < strlen(Format); i++) {
		switch(Format[i]) {
			case '%':
                Flags = GetFlag(Format+i+1);
                Width = GetWidth(Format+i+1 , &(CurrentPosition));
                Precision = GetPrecision(Format+i+1 , CurrentPosition ,&(CurrentPosition));
                if(Width+Length != 0) {
                    CurrentPosition++;
                }
                Length = GetLength(Format+i+1 , CurrentPosition);
                Type = GetType(Format+i+1 , Length , CurrentPosition);
                //__asm__ ("mov r8 , %0"::"r"((unsigned long)(Format+i+1)));
                //__asm__ ("mov r9 , %0"::"r"((unsigned long)i));
                //__asm__ ("mov r10 , %0"::"r"((unsigned long)Flags));
                //__asm__ ("mov r11 , %0"::"r"((unsigned long)Width));
                //__asm__ ("mov r12 , %0"::"r"((unsigned long)Precision));
                //__asm__ ("mov r13 , %0"::"r"((unsigned long)Length));
                //__asm__ ("mov r14 , %0"::"r"((unsigned long)Type));
                //__asm__ ("mov r15 , %0"::"r"((unsigned long)CurrentPosition));
                //while(1) {
                //    ;
                //}
                if(Type == '%') {
                    String[j++] = '%';
                }
                else if(Type == 's') {
                    StringPointer = va_arg(ap , char*);
                    strcpy(String+j , StringPointer);
                    j += strlen(StringPointer);
                }
                else if((Type == 'd')||(Type == 'i')) {
                    if(Precision == 'l') {
                        UnsignedLong = va_arg(ap , unsigned long);
                        ToString(TemporaryBuffer2 , UnsignedLong);
                        if(Width == 0) {
                            j += ToString(String+j , UnsignedLong);
                        }
                        else {
                            if(Width < strlen(TemporaryBuffer2)) {
                                Width = strlen(TemporaryBuffer2);
                            }
                            for(k = 0; k < Width; k++) {
                                TemporaryBuffer1[k] = Flags;
                            }
                            memcpy(TemporaryBuffer1+Width-strlen(TemporaryBuffer2) , TemporaryBuffer2 , strlen(TemporaryBuffer2));
                            TemporaryBuffer1[k] = 0x00;
                            strcpy(String+j , TemporaryBuffer1);
                            j += k;
                            while(Width != 0) {
                                Width /= 10;
                                i++;
                            }
                        }
                    }
                    else {
                        Integer = va_arg(ap , int);
                        ToString(TemporaryBuffer2 , Integer);
                        if(Width == 0) {
                            j += ToString(String+j , Integer);
                        }
                        else {
                            if(Width < strlen(TemporaryBuffer2)) {
                                Width = strlen(TemporaryBuffer2);
                            }
                            for(k = 0; k < Width; k++) {
                                TemporaryBuffer1[k] = Flags;
                            }
                            memcpy(TemporaryBuffer1+Width-strlen(TemporaryBuffer2) , TemporaryBuffer2 , strlen(TemporaryBuffer2));
                            TemporaryBuffer1[k] = 0x00;
                            strcpy(String+j , TemporaryBuffer1);
                            j += k;
                            while(Width != 0) {
                                Width /= 10;
                                i++;
                            }
                            i++;
                        }
                    }
                }
                else if(Type == 'c') {
                    Integer = va_arg(ap , int);
                    String[j++] = Integer;
                }
                else if((Type == 'f')||(Type == 'F')) {
                    Double = va_arg(ap , double);
                    if(Precision == 0) {
                        if(Width == 0) {
                            j += ToString(String+j , Double);
                            i++;
                        }
                    }
                    else {
                        // later
                    }
                }
                else if((Type == 'x')||(Type == 'X')) {
                    UnsignedLong = va_arg(ap , unsigned long);
                    ToString_Hex(TemporaryBuffer2 , UnsignedLong , (Type == 'x') ? 0 : 1);
                    if(Width == 0) {
                        j += ToString_Hex(String+j , UnsignedLong , (Type == 'x') ? 0 : 1);
                    }
                    else {
                        if(Width < strlen(TemporaryBuffer2)) {
                            Width = strlen(TemporaryBuffer2);
                        }
                        for(k = 0; k < Width; k++) {
                            TemporaryBuffer1[k] = Flags;
                        }
                        memcpy(TemporaryBuffer1+Width-strlen(TemporaryBuffer2) , TemporaryBuffer2 , strlen(TemporaryBuffer2));
                        TemporaryBuffer1[k] = 0x00;
                        strcpy(String+j , TemporaryBuffer1);
                        j += k;
                        if(Width != 0) {
                            i++;
                        }
                        while(Width != 0) {
                            Width /= 10;
                            i++;
                        }
                    }
                }
                i++;
                break;
			default:
				String[j++] = Format[i];
				break;
		}
	}
	String[j] = 0x00;
    return j;
}

int sprintf(char *String , const char *Format , ...) {
    int Length;
    va_list ap;
    va_start(ap , Format);
    Length = vsprintf(String , Format , ap);
    va_end(ap);
    return Length;
}*/