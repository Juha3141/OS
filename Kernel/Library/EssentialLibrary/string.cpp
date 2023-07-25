void *memset(void *Destination , unsigned char Value , unsigned long Size) {
    unsigned long i;
    for(i = 0; i < Size; i++) {
        ((unsigned char*)Destination)[i] = Value;
    }
    return Destination;
}

void *memcpy(void *Destination , const void *Source , unsigned long Size) {
    unsigned long i;
    for(i = 0; i < Size; i++) {
        ((unsigned char*)Destination)[i] = ((unsigned char*)Source)[i];
    }
    return Destination;
}

int memcmp(const void *Buffer1 , const void *Buffer2 , unsigned long Size) {
    unsigned long i;
    for(i = 0; i < Size; i++) {
        if(((unsigned char *)Buffer1)[i] > ((unsigned char*)Buffer2)[i]) {
            return 1;
        }
        else if(((unsigned char *)Buffer1)[i] < ((unsigned char*)Buffer2)[i]) {
            return -1;
        }
    }
    return 0;
}

unsigned long strlen(const char *String) {
    unsigned long Length = 0;
    while(String[Length] != 0x00) {
        Length++;
    }
    return Length;
}

char *strcpy(char *Destination , const char *Origin) {
    unsigned long i;
    unsigned long OriginLength = strlen(Origin);
    for(i = 0; i < OriginLength; i++) {
        Destination[i] = Origin[i];
    }
    Destination[i] = 0x00;
    return Destination;
}

char *strncpy(char *Destination , const char *Origin , unsigned long Length) {
    unsigned long i;
    unsigned long OriginLength = strlen(Origin);
    if(Length > OriginLength) {
       Length = OriginLength; 
    }
    for(i = 0; i < Length; i++) {
        Destination[i] = Origin[i];
    }
    Destination[i] = 0x00;
    return Destination;
}

char *strcat(char *Destination , const char *Origin) {
    unsigned long i;
    unsigned long DestinationLength = strlen(Destination);
    unsigned long OriginLength = strlen(Origin);
    for(i = DestinationLength; i < DestinationLength+OriginLength; i++) {
        Destination[i] = Origin[i-DestinationLength];
    }
    Destination[i] = 0x00;
    return Destination;
}

char *strncat(char *Destination , const char *Origin , unsigned long Length) {
    unsigned long i;
    unsigned long DestinationLength = strlen(Destination);
    for(i = DestinationLength; i < DestinationLength+Length; i++) {
        Destination[i] = Origin[i-DestinationLength];
    }
    Destination[i] = 0x00;
    return Destination;
}

int strcmp(const char *Destination , const char *Origin) {
    unsigned long i;
    unsigned long DestinationLength = strlen(Destination);
    unsigned long OriginLength = strlen(Origin);
    if(DestinationLength > OriginLength) {
        return 1;
    }
    else if(DestinationLength < OriginLength) {
        return -1;
    }
    for(i = 0; i < DestinationLength; i++) {
        if(Destination[i] > Origin[i]) {
            return 1;
        }
        else if(Destination[i] < Origin[i]) {
            return -1;
        }
    }
    return 0;
}

int strncmp(const char *Destination , const char *Origin , unsigned long Length) {
    unsigned long i;
    unsigned long DestinationLength = strlen(Destination);
    unsigned long OriginLength = strlen(Origin);
    if(Length > ((DestinationLength < OriginLength) ? DestinationLength : OriginLength)) {
        Length = ((DestinationLength < OriginLength) ? DestinationLength : OriginLength);
    }
    for(i = 0; i < Length; i++) {
        if(Destination[i] > Origin[i]) {
            return 1;
        }
        else if(Destination[i] < Origin[i]) {
            return -1;
        }
    }
    return 0;
}

